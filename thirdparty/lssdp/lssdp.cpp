/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 zlargon
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <ctype.h>   // isprint, isspace
#include <errno.h>   // errno
#include <stdarg.h>  // va_start, va_end, va_list
#include <stdio.h>   // snprintf, vsnprintf
#include <string.h>  // memset, memcpy, strlen, strcpy, strcmp, strncasecmp, strerror

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdlib.h>
#include <Winsock2.h>    // before Windows.h, else Winsock 1 conflict
#include <Ws2tcpip.h>    // needed for ip_mreq definition for multicast
#include <Windows.h>
#include <iphlpapi.h>
#else
#include <arpa/inet.h>   // inet_aton, inet_ntop, inet_addr, also include <netinet/in.h>
#include <fcntl.h>       // fcntl, F_GETFD, F_SETFD, FD_CLOEXEC
#include <net/if.h>      // struct ifconf, struct ifreq
#include <netinet/in.h>  // struct sockaddr_in, struct ip_mreq, INADDR_ANY, IPPROTO_IP, also include <sys/socket.h>
#include <sys/ioctl.h>   // ioctl, FIONBIO
#include <sys/socket.h>  // struct sockaddr, AF_INET, SOL_SOCKET, socklen_t, setsockopt, socket, bind, sendto, recvfrom
#include <unistd.h>      // close
#endif

#include <algorithm>
#include "logger/logger.h"
#include "senscord/osal.h"
#include "lssdp.h"

namespace senscord {

#ifndef _SIZEOF_ADDR_IFREQ
#define _SIZEOF_ADDR_IFREQ sizeof
#endif

/** Definition **/
#define LSSDP_BUFFER_LEN 2048
#define lssdp_debug(fmt, ...) \
  lssdp_log(LSSDP_LOG_DEBUG, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define lssdp_info(fmt, ...) \
  lssdp_log(LSSDP_LOG_INFO, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define lssdp_warn(fmt, ...) \
  lssdp_log(LSSDP_LOG_WARN, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define lssdp_error(fmt, ...) \
  lssdp_log(LSSDP_LOG_ERROR, __LINE__, __func__, fmt, ##__VA_ARGS__)
#ifdef _WIN32
#define strncasecmp(x, y, z) _strnicmp(x, y, z)
#define strerror_r(errno, buf, len) strerror_s(buf, len, errno)
#define close(x) closesocket(x)
#else
#endif

/** Struct: lssdp_packet **/
typedef struct lssdp_packet {
  char method[LSSDP_FIELD_LEN];       // M-SEARCH, NOTIFY, RESPONSE
  char st[LSSDP_FIELD_LEN];           // Search Target
  char usn[LSSDP_FIELD_LEN];          // Unique Service Name
  char location[LSSDP_LOCATION_LEN];  // Location

  /* Additional SSDP Header Fields */
  char connection[LSSDP_FIELD_LEN];
  char address[LSSDP_FIELD_LEN];
  char address_secondly[LSSDP_FIELD_LEN];
  char sm_id[LSSDP_FIELD_LEN];
  char device_type[LSSDP_FIELD_LEN];
  int64_t update_time;
} lssdp_packet;

/** Internal Function **/
static int send_multicast_data(const char *data, const struct lssdp_nwif nwif,
                               uint16_t ssdp_port);
static int lssdp_send_response(lssdp_ctx *lssdp, struct sockaddr_in address);
static int lssdp_packet_parser(const char *data, size_t data_len,
                               lssdp_packet *packet);
static int parse_field_line(const char *data, size_t start, size_t end,
                            lssdp_packet *packet);
static int get_colon_index(const char *str, size_t start, size_t end);
static int trim_spaces(const char *str, size_t *start, size_t *end);
static int lssdp_log(int level, int line, const char *func, const char *format,
                     ...);
static int neighbor_list_add(lssdp_ctx *lssdp, lssdp_packet *packet);
static int lssdp_neighbor_remove_all(lssdp_ctx *lssdp);
static void neighbor_list_free(lssdp_nbr *list);
static struct lssdp_nwif *find_interface_in_LAN(lssdp_ctx *lssdp,
                                                uint32_t address);

/** Global Variable **/
struct ST_GLOBAL {
  const char *MSEARCH;
  const char *NOTIFY;
  const char *RESPONSE;

  const char *HEADER_MSEARCH;
  const char *HEADER_NOTIFY;
  const char *HEADER_RESPONSE;

  const char *ADDR_LOCALHOST;
  const char *ADDR_MULTICAST;

  void (*log_callback)(const char *file, const char *tag, int level, int line,
                       const char *func, const char *message);
};

struct ST_GLOBAL Global = {"M-SEARCH",
                           "NOTIFY",
                           "RESPONSE",
                           "M-SEARCH * HTTP/1.1\r\n",
                           "NOTIFY * HTTP/1.1\r\n",
                           "HTTP/1.1 200 OK\r\n",
                           "127.0.0.1",
                           "239.255.255.250",
                           NULL};

static char errmsg[256];

// 01. lssdp_network_interface_update
int lssdp_network_interface_update(lssdp_ctx *lssdp) {
  if (lssdp == NULL) {
    lssdp_error("lssdp should not be NULL\n");
    return -1;
  }

  const size_t SIZE_OF_INTERFACE_LIST =
      sizeof(struct lssdp_nwif) * LSSDP_INTERFACE_LIST_SIZE;

  // 1. copy original interface
  struct lssdp_nwif original_nwif[LSSDP_INTERFACE_LIST_SIZE] = {};
  osal::OSMemcpy(original_nwif, sizeof(original_nwif), lssdp->nwif,
                 SIZE_OF_INTERFACE_LIST);

  // 2. reset lssdp->interface
  lssdp->nwif_num = 0;
  osal::OSMemset(lssdp->nwif, 0, SIZE_OF_INTERFACE_LIST);

  int result = 0;

  /* Reference to this article:
   * http://stackoverflow.com/a/8007079
   */

  // 3. create UDP socket
#ifdef _WIN32
  int fd = static_cast<int>(socket(AF_INET, SOCK_DGRAM, 0));
#else
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
#endif
  if (fd < 0) {
    lssdp_error("create socket failed, errno = %s (%d)\n",
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    // close socket
    if (fd >= 0 && close(fd) != 0) {
      lssdp_error("close fd %d failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    }
    // compare with original interface
    if (osal::OSMemcmp(original_nwif, lssdp->nwif, SIZE_OF_INTERFACE_LIST) ==
        0) {
      // interface is not changed
      return result;
    }

    /* Network Interface is changed */

    // 1. force clean up neighbor_list
    lssdp_neighbor_remove_all(lssdp);

    // 2. invoke network interface changed callback
    if (lssdp->network_interface_changed_callback != NULL) {
      lssdp->network_interface_changed_callback(lssdp);
    }

    return result;
  }

  // 4. get ifconfig
#ifdef _WIN32
#else
  char buffer[LSSDP_BUFFER_LEN] = {};
  struct ifconf ifc;
  ifc.ifc_len = sizeof(buffer);
  ifc.ifc_buf = (caddr_t)buffer;

  result = ioctl(fd, SIOCGIFCONF, &ifc);
  if (result < 0) {
    lssdp_error("ioctl SIOCGIFCONF failed, errno = %s (%d)\n",
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    // close socket
    if (fd >= 0 && close(fd) != 0) {
      lssdp_error("close fd %d failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    }

    // compare with original interface
    if (osal::OSMemcmp(original_nwif, lssdp->nwif, SIZE_OF_INTERFACE_LIST) ==
        0) {
      // interface is not changed
      return result;
    }

    /* Network Interface is changed */

    // 1. force clean up neighbor_list
    lssdp_neighbor_remove_all(lssdp);

    // 2. invoke network interface changed callback
    if (lssdp->network_interface_changed_callback != NULL) {
      lssdp->network_interface_changed_callback(lssdp);
    }

    return result;
  }
#endif

  // 5. setup lssdp->interface
#ifdef _WIN32
  PMIB_IPADDRTABLE pIpAddrTable = NULL;
  DWORD dwSize = 0;
  DWORD dwResult = 0;

  if (GetIpAddrTable(pIpAddrTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
    pIpAddrTable = static_cast<MIB_IPADDRTABLE *>(osal::OSMalloc(dwSize));
    dwResult = GetIpAddrTable(pIpAddrTable, &dwSize, 0);
  }
  if ((pIpAddrTable != NULL) && (dwResult == NO_ERROR)) {
    for (uint32_t i = 0; i < pIpAddrTable->dwNumEntries; i++) {
      if (lssdp->nwif_num >= LSSDP_INTERFACE_LIST_SIZE) {
        lssdp_warn("interface number is over than MAX SIZE (%d) %s\n",
                   LSSDP_INTERFACE_LIST_SIZE, pIpAddrTable->table[i].dwIndex);
        continue;
      }

      // 5-1. get interface ip string
      char ip[LSSDP_IP_LEN] = {};
      if (inet_ntop(AF_INET, (struct in_addr *)&pIpAddrTable->table[i].dwAddr,
                    ip, sizeof(ip)) == NULL) {
        lssdp_error("inet_ntop failed, errno = %s (%d)\n",
                    strerror_r(errno, errmsg, sizeof(errmsg)), errno);
        continue;
      }

      // 5-2. get network mask
      char netmask[LSSDP_IP_LEN] = {};
      if (inet_ntop(AF_INET, (struct in_addr *)&pIpAddrTable->table[i].dwMask,
                    netmask, sizeof(netmask)) == NULL) {
        lssdp_error("inet_ntop failed, errno = %s (%d)\n",
                    strerror_r(errno, errmsg, sizeof(errmsg)), errno);
        continue;
      }

      // 5-4. set interface
      size_t n = lssdp->nwif_num;
      snprintf(lssdp->nwif[n].name, LSSDP_INTERFACE_NAME_LEN, "%ld",
               pIpAddrTable->table[i].dwIndex);             // name
      snprintf(lssdp->nwif[n].ip, LSSDP_IP_LEN, "%s", ip);  // ip string
      lssdp->nwif[n].addr =
          pIpAddrTable->table[i].dwAddr;  // address in network byte order
      lssdp->nwif[n].netmask =
          pIpAddrTable->table[i].dwMask;  // netmask in network byte order
      // increase interface number
      lssdp->nwif_num++;
    }
    osal::OSFree(pIpAddrTable);
  }
#else
  int i;
  struct ifreq *ifr;
  for (i = 0; i < ifc.ifc_len; i += _SIZEOF_ADDR_IFREQ(*ifr)) {
    ifr = (struct ifreq *)(buffer + i);
    if (ifr->ifr_addr.sa_family != AF_INET) {
      // only support IPv4
      continue;
    }

    // 5-1. get interface ip string
    char ip[LSSDP_IP_LEN] = {};
    struct sockaddr_in *addr = (struct sockaddr_in *)&ifr->ifr_addr;
    if (inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip)) == NULL) {
      lssdp_error("inet_ntop failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
      continue;
    }

    // 5-2. get network mask
    struct ifreq netmask = {};
    osal::OSMemcpy(netmask.ifr_name, LSSDP_INTERFACE_NAME_LEN, ifr->ifr_name,
                   LSSDP_INTERFACE_NAME_LEN);
    if (ioctl(fd, SIOCGIFNETMASK, &netmask) != 0) {
      lssdp_error("ioctl SIOCGIFNETMASK failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
      continue;
    }

    // 5-3. check network interface number
    if (lssdp->nwif_num >= LSSDP_INTERFACE_LIST_SIZE) {
      lssdp_warn("interface number is over than MAX SIZE (%d) %s %s\n",
                 LSSDP_INTERFACE_LIST_SIZE, ifr->ifr_name, ip);
      continue;
    }

    // 5-4. set interface
    size_t n = lssdp->nwif_num;
    snprintf(lssdp->nwif[n].name, LSSDP_INTERFACE_NAME_LEN, "%s",
             ifr->ifr_name);                              // name
    snprintf(lssdp->nwif[n].ip, LSSDP_IP_LEN, "%s", ip);  // ip string
    lssdp->nwif[n].addr =
        addr->sin_addr.s_addr;  // address in network byte order

    // set network mask
    addr = (struct sockaddr_in *)&netmask.ifr_addr;
    lssdp->nwif[n].netmask =
        addr->sin_addr.s_addr;  // mask in network byte order

    // increase interface number
    lssdp->nwif_num++;
  }
#endif
  result = 0;

  // close socket
  if (fd >= 0 && close(fd) != 0) {
    lssdp_error("close fd %d failed, errno = %s (%d)\n",
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
  }

  // compare with original interface
  if (osal::OSMemcmp(original_nwif, lssdp->nwif, SIZE_OF_INTERFACE_LIST) == 0) {
    // interface is not changed
    return result;
  }

  /* Network Interface is changed */

  // 1. force clean up neighbor_list
  lssdp_neighbor_remove_all(lssdp);

  // 2. invoke network interface changed callback
  if (lssdp->network_interface_changed_callback != NULL) {
    lssdp->network_interface_changed_callback(lssdp);
  }

  return result;
}

// 02. lssdp_socket_create
int lssdp_socket_create(lssdp_ctx *lssdp) {
  if (lssdp == NULL) {
    lssdp_error("lssdp should not be NULL\n");
    return -1;
  }

  if (lssdp->port == 0) {
    lssdp_error("SSDP port (%d) has not been setup.\n", lssdp->port);
    return -1;
  }

  // close original SSDP socket
  lssdp_socket_close(lssdp);

  int result = -1;
  // create UDP socket
  size_t loop =
    (std::min)(lssdp->nwif_num, static_cast<size_t>(LSSDP_INTERFACE_NAME_LEN));
  for (size_t i = 0; i < loop; i++) {
#ifdef _WIN32
    lssdp->sock[i] = static_cast<int>(socket(AF_INET, SOCK_DGRAM, 0));
#else
    lssdp->sock[i] = socket(AF_INET, SOCK_DGRAM, 0);
#endif
    if (lssdp->sock[i] < 0) {
      lssdp_error("create socket failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
      lssdp_socket_close(lssdp);
      return -1;
    }

#ifdef _WIN32
    int opt = 1;
    // set reuse address
    if (setsockopt(lssdp->sock[i], SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<char *>(&opt), sizeof(opt)) != 0) {
      lssdp_error("setsockopt SO_REUSEADDR failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
      lssdp_socket_close(lssdp);
      return result;
    }
#else
    // set non-blocking
    int opt = 1;
    if (ioctl(lssdp->sock[i], FIONBIO, &opt) != 0) {
      lssdp_error("ioctl FIONBIO failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
      lssdp_socket_close(lssdp);
      return result;
    }

    // set reuse address
    if (setsockopt(lssdp->sock[i], SOL_SOCKET, SO_REUSEADDR, &opt,
                    sizeof(opt)) != 0) {
      lssdp_error("setsockopt SO_REUSEADDR failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
      lssdp_socket_close(lssdp);
      return result;
    }

    // set FD_CLOEXEC (http://kaivy2001.pixnet.net/blog/post/32726732)
    int sock_opt = fcntl(lssdp->sock[i], F_GETFD);
    if (sock_opt == -1) {
      lssdp_error("fcntl F_GETFD failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    } else {
      // F_SETFD
      if (fcntl(lssdp->sock[i], F_SETFD, sock_opt | FD_CLOEXEC) == -1) {
        lssdp_error("fcntl F_SETFD FD_CLOEXEC failed, errno = %s (%d)\n",
                    strerror_r(errno, errmsg, sizeof(errmsg)), errno);
      }
    }
#endif

    // bind socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(lssdp->port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(lssdp->sock[i], (struct sockaddr *)&addr, sizeof(addr)) != 0) {
      lssdp_error("bind failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
      lssdp_socket_close(lssdp);
      return result;
    }

    struct ip_mreq imr;
    uint32_t multiaddr = 0;
    uint32_t ifaddr = 0;
    if (osal::OSInetAton(Global.ADDR_MULTICAST, &multiaddr) != 0) {
      SENSCORD_LOG_ERROR("OSInetAton %s", Global.ADDR_MULTICAST);
      return result;
    }
    if (osal::OSInetAton(
        static_cast<const char *>(lssdp->nwif[i].ip), &ifaddr) != 0) {
      SENSCORD_LOG_ERROR("OSInetAton %s", lssdp->nwif[i].ip);
      return result;
    }
    imr.imr_multiaddr.s_addr = multiaddr;
    imr.imr_interface.s_addr = ifaddr;
    if (setsockopt(lssdp->sock[i], IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   reinterpret_cast<char *>(&imr),
                   sizeof(struct ip_mreq)) != 0) {
      lssdp_error("setsockopt IP_ADD_MEMBERSHIP failed: %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
      lssdp_socket_close(lssdp);
      return result;
    }

    lssdp_info("create SSDP socket %d\n", lssdp->sock[i]);
  }

  return 0;
}

// 03. lssdp_socket_close
int lssdp_socket_close(lssdp_ctx *lssdp) {
  if (lssdp == NULL) {
    lssdp_error("lssdp should not be NULL\n");
    return -1;
  }

  for (size_t i = 0; i < LSSDP_INTERFACE_LIST_SIZE; i++) {
    // check lssdp->sock
    if (lssdp->sock[i] <= 0) {
      lssdp_warn("SSDP socket is %d, ignore socket_close request.\n",
                 lssdp->sock[i]);
      lssdp->sock[i] = -1;
      continue;
    }

    // close socket
    if (close(lssdp->sock[i]) != 0) {
      lssdp_error("close socket %d failed, errno = %s (%d)\n", lssdp->sock[i],
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    }

    // close socket success
    lssdp_info("close SSDP socket %d\n", lssdp->sock[i]);

    lssdp->sock[i] = -1;
  }

  lssdp_neighbor_remove_all(lssdp);  // force clean up neighbor_list

  return 0;
}

// 04. lssdp_socket_read
int lssdp_socket_read(lssdp_ctx *lssdp, int index) {
  if (lssdp == NULL) {
    lssdp_error("lssdp should not be NULL\n");
    return -1;
  }

  // check socket index
  if ((index < 0) || (index >= LSSDP_INTERFACE_LIST_SIZE)) {
    lssdp_error("SSDP socket index out of range.\n", index);
    return -1;
  }

  // check socket and port
  if (lssdp->sock[index] <= 0) {
    lssdp_error("SSDP socket (%d) has not been setup.\n", lssdp->sock[index]);
    return -1;
  }

  if (lssdp->port == 0) {
    lssdp_error("SSDP port (%d) has not been setup.\n", lssdp->port);
    return -1;
  }

  char buffer[LSSDP_BUFFER_LEN] = {};
  struct sockaddr_in address = {};
  socklen_t address_len = sizeof(struct sockaddr_in);

  int32_t recv_len = recvfrom(lssdp->sock[index], buffer, sizeof(buffer), 0,
                              (struct sockaddr *)&address, &address_len);
  if (recv_len == -1) {
    lssdp_error("recvfrom fd %d failed, errno = %s (%d)\n", lssdp->sock[index],
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    return -1;
  }

  // ignore the SSDP packet received from self
  size_t loop =
    (std::min)(lssdp->nwif_num, static_cast<size_t>(LSSDP_INTERFACE_NAME_LEN));
  for (size_t i = 0; i < loop; i++) {
    if (lssdp->nwif[i].addr == address.sin_addr.s_addr) {
      // invoke packet received callback
      if (lssdp->packet_received_callback != NULL) {
        lssdp->packet_received_callback(lssdp, buffer, recv_len);
      }

      return 0;
    }
  }

  // parse SSDP packet to struct
  lssdp_packet packet = {};
  if (lssdp_packet_parser(buffer, recv_len, &packet) != 0) {
    // invoke packet received callback
    if (lssdp->packet_received_callback != NULL) {
      lssdp->packet_received_callback(lssdp, buffer, recv_len);
    }

    return 0;
  }

  // check search target
  if (strncmp(packet.st, lssdp->header.search_target, LSSDP_FIELD_LEN) != 0) {
    // search target is not match
    if (lssdp->debug) {
      lssdp_info("RECV <- %-8s   not match with %-14s %s\n", packet.method,
                 lssdp->header.search_target, packet.location);
    }
    // invoke packet received callback
    if (lssdp->packet_received_callback != NULL) {
      lssdp->packet_received_callback(lssdp, buffer, recv_len);
    }

    return 0;
  }

  // M-SEARCH: send RESPONSE back
  if (strncmp(packet.method, Global.MSEARCH, LSSDP_FIELD_LEN) == 0) {
    lssdp_send_response(lssdp, address);
    // invoke packet received callback
    if (lssdp->packet_received_callback != NULL) {
      lssdp->packet_received_callback(lssdp, buffer, recv_len);
    }

    return 0;
  }

  // RESPONSE, NOTIFY: add to neighbor_list
  neighbor_list_add(lssdp, &packet);

  if (lssdp->debug) {
    lssdp_info("RECV <- %-8s   %-28s  %s\n", packet.method, packet.location,
               packet.sm_id);
  }

  // invoke packet received callback
  if (lssdp->packet_received_callback != NULL) {
    lssdp->packet_received_callback(lssdp, buffer, recv_len);
  }

  return 0;
}

// 05. lssdp_send_msearch
int lssdp_send_msearch(lssdp_ctx *lssdp) {
  if (lssdp == NULL) {
    lssdp_error("lssdp should not be NULL\n");
    return -1;
  }

  if (lssdp->port == 0) {
    lssdp_error("SSDP port (%d) has not been setup.\n", lssdp->port);
    return -1;
  }

  // check network interface number
  if (lssdp->nwif_num == 0) {
    lssdp_warn("Network Interface is empty, no destination to send %s\n",
               Global.MSEARCH);
    return -1;
  }

  // 1. set M-SEARCH packet
  char msearch[LSSDP_BUFFER_LEN] = {};
  snprintf(msearch, sizeof(msearch),
           "%s"
           "HOST:%s:%d\r\n"
           "MAN:\"ssdp:discover\"\r\n"
           "MX:1\r\n"
           "ST:%s\r\n"
           "\r\n",
           Global.HEADER_MSEARCH,               // HEADER
           Global.ADDR_MULTICAST, lssdp->port,  // HOST
           lssdp->header.search_target);

  // 2. send M-SEARCH to each interface
  size_t loop =
    (std::min)(lssdp->nwif_num, static_cast<size_t>(LSSDP_INTERFACE_NAME_LEN));
  for (size_t i = 0; i < loop; i++) {
    struct lssdp_nwif *nwif = &lssdp->nwif[i];

    // avoid sending multicast to localhost
    uint32_t local = 0;
    if (osal::OSInetAton(Global.ADDR_LOCALHOST, &local) != 0) {
      SENSCORD_LOG_ERROR("OSInetAton %s", Global.ADDR_LOCALHOST);
      return -1;
    }
    if (nwif->addr == local) {
      continue;
    }

    // send M-SEARCH
    int ret = send_multicast_data(msearch, *nwif, lssdp->port);
    if (ret == 0 && lssdp->debug) {
      lssdp_info("SEND => %-8s   %s => MULTICAST\n", Global.MSEARCH, nwif->ip);
    }
  }

  return 0;
}

// 06. lssdp_send_notify
int lssdp_send_notify(lssdp_ctx *lssdp) {
  if (lssdp == NULL) {
    lssdp_error("lssdp should not be NULL\n");
    return -1;
  }

  if (lssdp->port == 0) {
    lssdp_error("SSDP port (%d) has not been setup.\n", lssdp->port);
    return -1;
  }

  // check network interface number
  if (lssdp->nwif_num == 0) {
    lssdp_warn("Network Interface is empty, no destination to send %s\n",
               Global.NOTIFY);
    return -1;
  }

  size_t loop =
    (std::min)(lssdp->nwif_num, static_cast<size_t>(LSSDP_INTERFACE_NAME_LEN));
  size_t i;
  for (i = 0; i < loop; i++) {
    struct lssdp_nwif *nwif = &lssdp->nwif[i];

    // avoid sending multicast to localhost
    uint32_t local = 0;
    if (osal::OSInetAton(Global.ADDR_LOCALHOST, &local) != 0) {
      SENSCORD_LOG_ERROR("OSInetAton %s", Global.ADDR_LOCALHOST);
      return -1;
    }
    if (nwif->addr == local) {
      continue;
    }

    // set notify packet
    char notify[LSSDP_BUFFER_LEN] = {};
    char *domain = lssdp->header.location.domain;
    snprintf(notify, sizeof(notify),
             "%s"
             "HOST:%s:%d\r\n"
             "CACHE-CONTROL:max-age=120\r\n"
             "LOCATION:%s%s\r\n"
             "ST:%s\r\n"
             "CONNECTION:%s\r\n"
             "ADDRESS:%s%s\r\n"
             "ADDRESS_SECONDLY:%s%s\r\n"
             "\r\n",
             Global.HEADER_NOTIFY,                    // HEADER
             Global.ADDR_MULTICAST, lssdp->port,      // HOST
             strlen(domain) > 0 ? domain : nwif->ip,  // LOCATION
             lssdp->header.location.suffix,
             lssdp->header.search_target,             // ST (Search Target)
             lssdp->header.connection,                // CONNECTION
             strlen(domain) > 0 ? domain : nwif->ip,  // ADDRESS
             lssdp->header.location.suffix,
             strlen(domain) > 0 ? domain : nwif->ip,  // ADDRESS_SECONDLY
             lssdp->header.location.suffix_secondly);

    // send NOTIFY
    int ret = send_multicast_data(notify, *nwif, lssdp->port);
    if (ret == 0 && lssdp->debug) {
      lssdp_info("SEND => %-8s   %s => MULTICAST\n", Global.NOTIFY, nwif->ip);
    }
  }

  return 0;
}

// 07. lssdp_neighbor_check_timeout
int lssdp_neighbor_check_timeout(lssdp_ctx *lssdp) {
  if (lssdp == NULL) {
    lssdp_error("lssdp should not be NULL\n");
    return -1;
  }

  // check neighbor_timeout
  if (lssdp->neighbor_timeout <= 0) {
    lssdp_warn(
        "lssdp->neighbor_timeout (%ld) is invalid, ignore check_timeout "
        "request.\n",
        lssdp->neighbor_timeout);
    return 0;
  }

  uint64_t current_time = lssdp_get_current_time();
  if (current_time == 0) {
    lssdp_error("got invalid timestamp %lld\n", current_time);
    return -1;
  }

  bool is_changed = false;
  lssdp_nbr *prev = NULL;
  lssdp_nbr *nbr = lssdp->neighbor_list;
  while (nbr != NULL) {
    uint64_t pass_time = current_time - nbr->update_time;
    if (pass_time < lssdp->neighbor_timeout) {
      prev = nbr;
      nbr = nbr->next;
      continue;
    }

    is_changed = true;
    lssdp_warn("remove timeout SSDP neighbor: %s (%s) (%ldms)\n", nbr->sm_id,
               nbr->location, pass_time);

    if (prev == NULL) {
      // it's first neighbor in list
      lssdp->neighbor_list = nbr->next;
      osal::OSFree(nbr);
      nbr = lssdp->neighbor_list;
    } else {
      prev->next = nbr->next;
      osal::OSFree(nbr);
      nbr = prev->next;
    }
  }

  // invoke neighbor list changed callback
  if (is_changed == true && lssdp->neighbor_list_changed_callback != NULL) {
    lssdp->neighbor_list_changed_callback(lssdp);
  }
  return 0;
}

// 08. lssdp_set_log_callback
void lssdp_set_log_callback(void (*callback)(const char *file, const char *tag,
                                             int level, int line,
                                             const char *func,
                                             const char *message)) {
  Global.log_callback = callback;
}

uint64_t lssdp_get_current_time() {
  uint64_t nano_seconds = 0;
  if (osal::OSGetTime(&nano_seconds) != 0) {
    lssdp_error("OSGetTime failed\n");
    return 0;
  }
  return nano_seconds / 1000 / 1000;
}

/*
 * Init.
 * @return = 0      success
 *         < 0      failed
 */
int lssdp_init() {
#ifdef _WIN32
  //
  // Initialize Windows Socket API with given VERSION.
  //
  WSADATA wsaData;
  if (WSAStartup(0x0101, &wsaData)) {
    perror("WSAStartup");
    return 1;
  }
#endif
  return 0;
}

/*
 * Exit.
 */
void lssdp_exit() {
#ifdef _WIN32
  //
  // Program never actually gets here due to infinite loop that has to be
  // canceled, but since people on the internet wind up using examples
  // they find at random in their own code it's good to show what shutting
  // down cleanly would look like.
  //
  WSACleanup();
#endif
}

/** Internal Function **/

static int send_multicast_data(const char *data, const struct lssdp_nwif nwif,
                               uint16_t ssdp_port) {
  if (data == NULL) {
    lssdp_error("data should not be NULL\n");
    return -1;
  }

  size_t data_len = strlen(data);
  if (data_len == 0) {
    lssdp_error("data length should not be empty\n");
    return -1;
  }

  if (strlen(nwif.name) == 0) {
    lssdp_error("interface.name should not be empty\n");
    return -1;
  }

  int result = -1;

  // 1. create UDP socket
#ifdef _WIN32
  int fd = static_cast<int>(socket(AF_INET, SOCK_DGRAM, 0));
#else
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
#endif
  if (fd < 0) {
    lssdp_error("create socket failed, errno = %s (%d)\n",
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    if (fd >= 0 && close(fd) != 0) {
      lssdp_error("close fd %d failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    }
    return result;
  }

#ifdef _WIN32
  // set network interface
  uint32_t ifaddr = 0;
  if (osal::OSInetAton(static_cast<const char *>(nwif.ip), &ifaddr) != 0) {
    SENSCORD_LOG_ERROR("OSInetAton %s", nwif.ip);
    return result;
  }
  if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF,
                 (char *)&ifaddr, sizeof(ifaddr)) != 0) {
    lssdp_error("setsockopt IP_MULTICAST_IF failed, errno = %s (%d)\n",
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    return result;
  }
#else
  // 2. bind socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = nwif.addr;

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    lssdp_error("bind failed, errno = %s (%d)\n",
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    if (fd >= 0 && close(fd) != 0) {
      lssdp_error("close fd %d failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    }
    return result;
  }

  // 3. disable IP_MULTICAST_LOOP
  char opt = 0;
  if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &opt, sizeof(opt)) < 0) {
    lssdp_error("setsockopt IP_MULTICAST_LOOP failed, errno = %s (%d)\n",
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    if (fd >= 0 && close(fd) != 0) {
      lssdp_error("close fd %d failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    }
    return result;
  }
#endif

  // 4. set destination address
  struct sockaddr_in dest_addr;
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(ssdp_port);
  if (inet_pton(dest_addr.sin_family, Global.ADDR_MULTICAST,
                &dest_addr.sin_addr) == 0) {
    lssdp_error("inet_aton failed, errno = %s (%d)\n",
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    if (fd >= 0 && close(fd) != 0) {
      lssdp_error("close fd %d failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    }
    return result;
  }

  // 5. send data
#ifdef _WIN32
  if (sendto(fd, data, static_cast<int>(data_len), 0,
             (struct sockaddr *)&dest_addr, sizeof(dest_addr)) == -1) {
#else
  if (sendto(fd, data, data_len, 0, (struct sockaddr *)&dest_addr,
             sizeof(dest_addr)) == -1) {
#endif
    lssdp_error("sendto %s (%s) failed, errno = %s (%d)\n", nwif.name, nwif.ip,
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    if (fd >= 0 && close(fd) != 0) {
      lssdp_error("close fd %d failed, errno = %s (%d)\n",
                  strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    }
    return result;
  }

  result = 0;

  if (fd >= 0 && close(fd) != 0) {
    lssdp_error("close fd %d failed, errno = %s (%d)\n",
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
  }
  return result;
}

static int lssdp_send_response(lssdp_ctx *lssdp, struct sockaddr_in address) {
  // get M-SEARCH IP
  char msearch_ip[LSSDP_IP_LEN] = {};
  if (inet_ntop(AF_INET, &address.sin_addr, msearch_ip, sizeof(msearch_ip)) ==
      NULL) {
    lssdp_error("inet_ntop failed, errno = %s (%d)\n",
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    return -1;
  }

  // 1. find the interface which is in LAN
  struct lssdp_nwif *nwif =
      find_interface_in_LAN(lssdp, address.sin_addr.s_addr);
  if (nwif == NULL) {
    if (lssdp->debug) {
      lssdp_info("RECV <- %-8s   Interface is not found        %s\n",
                 Global.MSEARCH, msearch_ip);
    }

    if (lssdp->nwif_num == 0) {
      lssdp_warn("Network Interface is empty, no destination to send %s\n",
                 Global.RESPONSE);
    }
    return -1;
  }

  // 2. set response packet
  char response[LSSDP_BUFFER_LEN] = {};
  char *domain = lssdp->header.location.domain;
  int response_len =
      snprintf(response, sizeof(response),
               "%s"
               "CACHE-CONTROL:max-age=120\r\n"
               "LOCATION:%s%s\r\n"
               "ST:%s\r\n"
               "CONNECTION:%s\r\n"
               "ADDRESS:%s%s\r\n"
               "ADDRESS_SECONDLY:%s%s\r\n"
               "\r\n",
               Global.HEADER_RESPONSE,                  // HEADER
               strlen(domain) > 0 ? domain : nwif->ip,  // LOCATION
               lssdp->header.location.suffix,
               lssdp->header.search_target,             // ST (Search Target)
               lssdp->header.connection,                // CONNECTION
               strlen(domain) > 0 ? domain : nwif->ip,  // ADDRESS
               lssdp->header.location.suffix,
               strlen(domain) > 0 ? domain : nwif->ip,  // ADDRESS_SECONDLY
               lssdp->header.location.suffix_secondly);

  if (response_len < 0) {
    lssdp_error("response_len failed\n");
    return -1;
  }

  // 3. set port to address
  address.sin_port = htons(lssdp->port);

  if (lssdp->debug) {
    lssdp_info("RECV <- %-8s   %s <- %s\n", Global.MSEARCH, nwif->ip,
               msearch_ip);
  }

  // 4. send data
  if (sendto(lssdp->sock[0], response, response_len, 0,
             (struct sockaddr *)&address, sizeof(struct sockaddr_in)) == -1) {
    lssdp_error("send RESPONSE to %s failed, errno = %s (%d)\n", msearch_ip,
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    return -1;
  }

  if (lssdp->debug) {
    lssdp_info("SEND => %-8s   %s => %s\n", Global.RESPONSE, nwif->ip,
               msearch_ip);
  }

  return 0;
}

static int lssdp_packet_parser(const char *data, size_t data_len,
                               lssdp_packet *packet) {
  if (data == NULL) {
    lssdp_error("data should not be NULL\n");
    return -1;
  }

  if (data_len != strnlen(data, LSSDP_BUFFER_LEN)) {
    lssdp_error("data_len (%zu) is not match to the data length (%zu)\n",
                data_len, strnlen(data, LSSDP_BUFFER_LEN));
    return -1;
  }

  if (packet == NULL) {
    lssdp_error("packet should not be NULL\n");
    return -1;
  }

  // 1. compare SSDP Method Header: M-SEARCH, NOTIFY, RESPONSE
  size_t i;
  if ((i = strlen(Global.HEADER_MSEARCH)) < data_len &&
      osal::OSMemcmp(data, Global.HEADER_MSEARCH, i) == 0) {
    osal::OSMemcpy(packet->method, LSSDP_FIELD_LEN - 1, Global.MSEARCH,
                   data_len);
  } else if ((i = strlen(Global.HEADER_NOTIFY)) < data_len &&
             osal::OSMemcmp(data, Global.HEADER_NOTIFY, i) == 0) {
    osal::OSMemcpy(packet->method, LSSDP_FIELD_LEN - 1, Global.NOTIFY,
                   data_len);
  } else if ((i = strlen(Global.HEADER_RESPONSE)) < data_len &&
             osal::OSMemcmp(data, Global.HEADER_RESPONSE, i) == 0) {
    osal::OSMemcpy(packet->method, LSSDP_FIELD_LEN - 1, Global.RESPONSE,
                   data_len);
  } else {
    lssdp_warn("received unknown SSDP packet\n");
    lssdp_debug("%s\n", data);
    return -1;
  }

  // 2. parse each field line
  size_t start = i;
  for (i = start; i < data_len; i++) {
    if (data[i] == '\n' && i - 1 > start && data[i - 1] == '\r') {
      parse_field_line(data, start, i - 2, packet);
      start = i + 1;
    }
  }

  // 3. set update_time
  uint64_t current_time = lssdp_get_current_time();
  if (current_time == 0) {
    lssdp_error("got invalid timestamp %lld\n", current_time);
    return -1;
  }
  packet->update_time = current_time;
  return 0;
}

static int parse_field_line(const char *data, size_t start, size_t end,
                            lssdp_packet *packet) {
  // 1. find the colon
  if (data[start] == ':') {
    lssdp_warn("the first character of line should not be colon\n");
    lssdp_debug("%s\n", data);
    return -1;
  }

  int colon = get_colon_index(data, start + 1, end);
  if (colon == -1) {
    lssdp_warn("there is no colon in line\n");
    lssdp_debug("%s\n", data);
    return -1;
  }

  if ((size_t)colon == end) {
    // value is empty
    return -1;
  }

  // 2. get field, field_len
  size_t i = start;
  size_t j = colon - 1;
  if (trim_spaces(data, &i, &j) == -1) {
    return -1;
  }
  const char *field = &data[i];
  size_t field_len = j - i + 1;

  // 3. get value, value_len
  i = colon + 1;
  j = end;
  if (trim_spaces(data, &i, &j) == -1) {
    return -1;
  }
  const char *value = &data[i];
  size_t value_len = j - i + 1;

  // 4. set each field's value to packet
  if (field_len == strlen("st") && strncasecmp(field, "st", field_len) == 0) {
    osal::OSMemcpy(
        packet->st, LSSDP_FIELD_LEN - 1, value,
        value_len < LSSDP_FIELD_LEN ? value_len : LSSDP_FIELD_LEN - 1);
    return 0;
  }

  if (field_len == strlen("nt") && strncasecmp(field, "nt", field_len) == 0) {
    osal::OSMemcpy(
        packet->st, LSSDP_FIELD_LEN - 1, value,
        value_len < LSSDP_FIELD_LEN ? value_len : LSSDP_FIELD_LEN - 1);
    return 0;
  }

  if (field_len == strlen("usn") && strncasecmp(field, "usn", field_len) == 0) {
    osal::OSMemcpy(
        packet->usn, LSSDP_FIELD_LEN - 1, value,
        value_len < LSSDP_FIELD_LEN ? value_len : LSSDP_FIELD_LEN - 1);
    return 0;
  }

  if (field_len == strlen("location") &&
      strncasecmp(field, "location", field_len) == 0) {
    osal::OSMemcpy(
        packet->location, LSSDP_LOCATION_LEN - 1, value,
        value_len < LSSDP_LOCATION_LEN ? value_len : LSSDP_LOCATION_LEN - 1);
    return 0;
  }

  if (field_len == strlen("sm_id") &&
      strncasecmp(field, "sm_id", field_len) == 0) {
    osal::OSMemcpy(
        packet->sm_id, LSSDP_FIELD_LEN - 1, value,
        value_len < LSSDP_FIELD_LEN ? value_len : LSSDP_FIELD_LEN - 1);
    return 0;
  }

  if (field_len == strlen("dev_type") &&
      strncasecmp(field, "dev_type", field_len) == 0) {
    osal::OSMemcpy(
        packet->device_type, LSSDP_FIELD_LEN - 1, value,
        value_len < LSSDP_FIELD_LEN ? value_len : LSSDP_FIELD_LEN - 1);
    return 0;
  }

  if (field_len == strlen("connection") &&
      strncasecmp(field, "connection", field_len) == 0) {
    osal::OSMemcpy(
        packet->connection, LSSDP_FIELD_LEN - 1, value,
        value_len < LSSDP_FIELD_LEN ? value_len : LSSDP_FIELD_LEN - 1);
    return 0;
  }

  if (field_len == strlen("address") &&
      strncasecmp(field, "address", field_len) == 0) {
    osal::OSMemcpy(
        packet->address, LSSDP_FIELD_LEN - 1, value,
        value_len < LSSDP_FIELD_LEN ? value_len : LSSDP_FIELD_LEN - 1);
    return 0;
  }

  if (field_len == strlen("address_secondly") &&
      strncasecmp(field, "address_secondly", field_len) == 0) {
    osal::OSMemcpy(
        packet->address_secondly, LSSDP_FIELD_LEN - 1, value,
        value_len < LSSDP_FIELD_LEN ? value_len : LSSDP_FIELD_LEN - 1);
    return 0;
  }

  // the field is not in the struct packet
  return 0;
}

static int get_colon_index(const char *str, size_t start, size_t end) {
  size_t i;
  for (i = start; i <= end; i++) {
    if (str[i] == ':') {
      return static_cast<int>(i);
    }
  }
  return -1;
}

static int trim_spaces(const char *str, size_t *start, size_t *end) {
  size_t i = *start;
  size_t j = *end;

  while (i <= *end && (!isprint(str[i]) || isspace(str[i]))) i++;
  while (j >= *start && (!isprint(str[j]) || isspace(str[j]))) j--;

  if (i > j) {
    return -1;
  }

  *start = i;
  *end = j;
  return 0;
}

static int lssdp_log(int level, int line, const char *func, const char *format,
                     ...) {
  if (Global.log_callback == NULL) {
    return -1;
  }

  char message[LSSDP_BUFFER_LEN] = {};

  // create message by va_list
  va_list args;
  va_start(args, format);
  osal::OSVsnprintf(message, LSSDP_BUFFER_LEN, format, args);
  va_end(args);

  // invoke log callback function
  Global.log_callback(__FILE__, "SSDP", level, line, func, message);
  return 0;
}

static int neighbor_list_add(lssdp_ctx *lssdp, lssdp_packet *packet) {
  lssdp_nbr *last_nbr = lssdp->neighbor_list;

  bool is_changed = false;
  lssdp_nbr *nbr;
  for (nbr = lssdp->neighbor_list; nbr != NULL;
       last_nbr = nbr, nbr = nbr->next) {
    if (strncmp(nbr->location, packet->location, LSSDP_LOCATION_LEN) != 0) {
      // location is not match
      continue;
    }

    /* location is not found in SSDP list: update neighbor */

    // usn
    if (strncmp(nbr->usn, packet->usn, LSSDP_FIELD_LEN) != 0) {
      lssdp_debug("neighbor usn is changed. (%s -> %s)\n", nbr->usn,
                  packet->usn);
      osal::OSMemcpy(nbr->usn, LSSDP_FIELD_LEN, packet->usn, LSSDP_FIELD_LEN);
      is_changed = true;
    }

    // sm_id
    if (strncmp(nbr->sm_id, packet->sm_id, LSSDP_FIELD_LEN) != 0) {
      lssdp_debug("neighbor sm_id is changed. (%s -> %s)\n", nbr->sm_id,
                  packet->sm_id);
      osal::OSMemcpy(nbr->sm_id, LSSDP_FIELD_LEN, packet->sm_id,
                     LSSDP_FIELD_LEN);
      is_changed = true;
    }

    // device type
    if (strncmp(nbr->device_type, packet->device_type, LSSDP_FIELD_LEN) != 0) {
      lssdp_debug("neighbor device_type is changed. (%s -> %s)\n",
                  nbr->device_type, packet->device_type);
      osal::OSMemcpy(nbr->device_type, LSSDP_FIELD_LEN, packet->device_type,
                     LSSDP_FIELD_LEN);
      is_changed = true;
    }

    // connection
    if (strncmp(nbr->connection, packet->connection, LSSDP_FIELD_LEN) != 0) {
      lssdp_debug("neighbor connection is changed. (%s -> %s)\n",
                  nbr->connection, packet->connection);
      osal::OSMemcpy(nbr->connection, LSSDP_FIELD_LEN, packet->connection,
                     LSSDP_FIELD_LEN);
      is_changed = true;
    }

    // address
    if (strncmp(nbr->address, packet->address, LSSDP_FIELD_LEN) != 0) {
      lssdp_debug("neighbor address is changed. (%s -> %s)\n", nbr->address,
                  packet->address);
      osal::OSMemcpy(nbr->address, LSSDP_FIELD_LEN, packet->address,
                     LSSDP_FIELD_LEN);
      is_changed = true;
    }

    // address_secondly
    if (strncmp(nbr->address_secondly, packet->address_secondly,
                LSSDP_FIELD_LEN) != 0) {
      lssdp_debug("neighbor address_secondly is changed. (%s -> %s)\n",
                  nbr->address_secondly, packet->address_secondly);
      osal::OSMemcpy(nbr->address_secondly, LSSDP_FIELD_LEN,
                     packet->address_secondly, LSSDP_FIELD_LEN);
      is_changed = true;
    }

    // update_time
    nbr->update_time = packet->update_time;
    // invoke neighbor list changed callback
    if (lssdp->neighbor_list_changed_callback != NULL && is_changed == true) {
      lssdp->neighbor_list_changed_callback(lssdp);
    }

    return 0;
  }

  /* location is not found in SSDP list: add to list */

  // 1. memory allocate lssdp_nbr
  nbr = static_cast<lssdp_nbr *>(osal::OSMalloc(sizeof(lssdp_nbr)));
  if (nbr == NULL) {
    lssdp_error("malloc failed, errno = %s (%d)\n",
                strerror_r(errno, errmsg, sizeof(errmsg)), errno);
    return -1;
  }

  // 2. setup neighbor
  osal::OSMemcpy(nbr->usn, LSSDP_FIELD_LEN, packet->usn, LSSDP_FIELD_LEN);
  osal::OSMemcpy(nbr->sm_id, LSSDP_FIELD_LEN, packet->sm_id, LSSDP_FIELD_LEN);
  osal::OSMemcpy(nbr->device_type, LSSDP_FIELD_LEN, packet->device_type,
                 LSSDP_FIELD_LEN);
  osal::OSMemcpy(nbr->connection, LSSDP_FIELD_LEN, packet->connection,
                 LSSDP_FIELD_LEN);
  osal::OSMemcpy(nbr->address, LSSDP_FIELD_LEN, packet->address,
                 LSSDP_FIELD_LEN);
  osal::OSMemcpy(nbr->address_secondly, LSSDP_FIELD_LEN,
                 packet->address_secondly, LSSDP_FIELD_LEN);
  osal::OSMemcpy(nbr->location, LSSDP_LOCATION_LEN, packet->location,
                 LSSDP_LOCATION_LEN);
  nbr->update_time = packet->update_time;
  nbr->next = NULL;

  // 3. add neighbor to the end of list
  if (last_nbr == NULL) {
    // it's the first neighbor
    lssdp->neighbor_list = nbr;
  } else {
    last_nbr->next = nbr;
  }

  // invoke neighbor list changed callback
  if (lssdp->neighbor_list_changed_callback != NULL) {
    lssdp->neighbor_list_changed_callback(lssdp);
  }

  return 0;
}

static int lssdp_neighbor_remove_all(lssdp_ctx *lssdp) {
  if (lssdp->neighbor_list == NULL) {
    return 0;
  }

  // free neighbor_list
  neighbor_list_free(lssdp->neighbor_list);
  lssdp->neighbor_list = NULL;

  lssdp_info("neighbor list has been force clean up.\n");

  // invoke neighbor list changed callback
  if (lssdp->neighbor_list_changed_callback != NULL) {
    lssdp->neighbor_list_changed_callback(lssdp);
  }
  return 0;
}

static void neighbor_list_free(lssdp_nbr *list) {
  if (list != NULL) {
    neighbor_list_free(list->next);
    osal::OSFree(list);
  }
}

static struct lssdp_nwif *find_interface_in_LAN(lssdp_ctx *lssdp,
                                                uint32_t address) {
  struct lssdp_nwif *ifc;
  size_t loop =
    (std::min)(lssdp->nwif_num, static_cast<size_t>(LSSDP_INTERFACE_NAME_LEN));
  for (size_t i = 0; i < loop; i++) {
    ifc = &lssdp->nwif[i];

    // mask address to check whether the interface is under the same Local
    // Network Area or not
    if ((ifc->addr & ifc->netmask) == (address & ifc->netmask)) {
      return ifc;
    }
  }
  return NULL;
}

}  // namespace senscord
