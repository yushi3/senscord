/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <ifaddrs.h>
#include <errno.h>
#include <inttypes.h>

#include <algorithm>
#include <vector>
#include <limits>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"
#include "linux/osal_linuxerror.h"
#include "linux/socket_info_manager.h"

namespace {

/**
 * @brief Convert nanoseconds to timeval.
 */
timeval ToTimeval(uint64_t nano_seconds) {
  // round-up (+999 nanoseconds).
  lldiv_t val = lldiv(nano_seconds + 999, 1000LL * 1000 * 1000);

  timeval tval = {};
  tval.tv_sec = val.quot;
  tval.tv_usec = static_cast<suseconds_t>(val.rem / 1000);

  return tval;
}

const uint64_t kConnectTimeoutDefault = static_cast<uint64_t>(-1);

}  // namespace

namespace senscord {
namespace osal {

/**
 * @brief Type used by SelectSocket function.
 */
enum SelectType {
  kSelectRead,    /**< Check for readable. */
  kSelectWrite,   /**< Check for writable. */
  kSelectExcept,  /**< Check for errors. */
};

/**
 * @brief Convert OSSocket to socket fd.
 */
static int32_t GetSocketFd(OSSocket* socket) {
  intptr_t temp = reinterpret_cast<intptr_t>(socket);
  int32_t socket_fd = static_cast<int32_t>(temp);
  return socket_fd;
}

/**
 * @brief Convert socket fd to OSSocket.
 */
static OSSocket* GetOSSocket(int32_t socket_fd) {
  intptr_t temp = static_cast<intptr_t>(socket_fd);
  OSSocket* socket = reinterpret_cast<OSSocket*>(temp);
  return socket;
}

static OSErrorCause GetSocketType(int32_t socket_fd, int32_t* socket_type) {
  if (socket_type == NULL) {
    return kErrorInvalidArgument;
  }
  int32_t type = 0;
  socklen_t size = sizeof(type);
  int32_t ret = getsockopt(socket_fd, SOL_SOCKET, SO_TYPE, &type, &size);
  if (ret != 0) {
    return GetErrorCauseFromErrno(errno);
  }
  *socket_type = type;
  return kErrorNone;
}

/**
 * @brief Create a socket.
 * @param[in]  socket_type Type of socket.
 * @param[out] socket      Pointer to the variable that receives the socket
 *                         object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSCreateSocket(OSSocketType socket_type, OSSocket** socket) {
  static const OSFunctionId kFuncId = kIdOSCreateSocket;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int32_t domain = 0;
  int32_t type = 0;
  int32_t protocol = 0;
  switch (socket_type) {
    case kSocketTypeInetUdp:
      domain = AF_INET;
      type = SOCK_DGRAM;
      break;
    case kSocketTypeInetTcp:
      domain = AF_INET;
      type = SOCK_STREAM;
      break;
    default:
      return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int32_t socket_fd = ::socket(domain, type, protocol);
  if (socket_fd < 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  OSSocket* new_socket = GetOSSocket(socket_fd);

  SocketInfo info = {};
  info.binded = false;
  if (type == SOCK_DGRAM) {
    info.writable = true;
  } else {
    info.writable = false;
  }
  OSErrorCause cause =
      SocketInfoManager::GetInstance()->Insert(new_socket, info);
  if (cause != kErrorNone) {
    close(socket_fd);
    SENSCORD_OSAL_LOG_ERROR("Insert(SocketInfo) failed. cause=%d", cause);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }

  *socket = new_socket;

  return 0;
}

/**
 * @brief Disables send, receive, or both on a socket.
 * @param[in] socket  Socket object.
 * @param[in] option  Shutdown option.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSShutdownSocket(OSSocket* socket, OSShutdownOption option) {
  static const OSFunctionId kFuncId = kIdOSShutdownSocket;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);

  int32_t how = 0;
  switch (option) {
    case kShutdownReceive: how = SHUT_RD;   break;
    case kShutdownSend:    how = SHUT_WR;   break;
    case kShutdownBoth:    how = SHUT_RDWR; break;
    default:
      return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int32_t ret = shutdown(socket_fd, how);
  if (ret != 0) {
    int32_t error = errno;
    // Returns success if ENOTCONN and SOCK_DGRAM.
    if (error == ENOTCONN) {
      int32_t socket_type = 0;
      GetSocketType(socket_fd, &socket_type);
      if (socket_type == SOCK_DGRAM) {
        SENSCORD_OSAL_LOG_DEBUG("success (ENOTCONN, DGRAM)");
        return 0;
      }
    }
    OSErrorCause cause = GetErrorCauseFromErrno(error);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

/**
 * @brief Destroy a socket.
 * @param[in] socket  Socket object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDestroySocket(OSSocket* socket) {
  static const OSFunctionId kFuncId = kIdOSDestroySocket;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  SocketInfo info = {};
  OSErrorCause cause = SocketInfoManager::GetInstance()->Delete(socket, &info);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  int32_t socket_fd = GetSocketFd(socket);

  int32_t ret = close(socket_fd);
  if (ret != 0) {
    int32_t error = errno;
    SENSCORD_OSAL_LOG_ERROR("close failed. errno=" PRId32, error);
    if (error != EBADF) {
      // Re-register the deleted information.
      SocketInfoManager::GetInstance()->Insert(socket, info);
    }
    cause = GetErrorCauseFromErrno(error);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

/**
 * @brief Bind a name to a socket.
 * @param[in] socket  Socket object.
 * @param[in] address IP address to assign to the bound socket.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSBindSocket(OSSocket* socket, const OSSocketAddressInet& address) {
  static const OSFunctionId kFuncId = kIdOSBindSocket;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  SocketInfo info = {};
  OSErrorCause cause = SocketInfoManager::GetInstance()->Get(socket, &info);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  int32_t socket_fd = GetSocketFd(socket);

  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = address.port;
  addr.sin_addr.s_addr = address.address;

  int32_t ret = bind(socket_fd,
                     reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
  if (ret != 0) {
    cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  info.binded = true;
  cause = SocketInfoManager::GetInstance()->Set(socket, info);
  if (cause != kErrorNone) {
    SENSCORD_OSAL_LOG_ERROR("Set(SocketInfo) failed. cause=%d", cause);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }

  return 0;
}

/**
 * @brief Listen for connections on a socket.
 * @param[in] socket  Socket object.
 * @param[in] backlog The maximum length of the queue of pending connections.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSListenSocket(OSSocket* socket, int32_t backlog) {
  static const OSFunctionId kFuncId = kIdOSListenSocket;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  SocketInfo info = {};
  OSErrorCause cause = SocketInfoManager::GetInstance()->Get(socket, &info);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  if (!info.binded) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);

  int32_t ret = listen(socket_fd, backlog);
  if (ret != 0) {
    cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

/**
 * @brief Accept a connection on a socket.
 * @param[in]  socket         Socket object.
 * @param[out] accept_socket  Pointer to the variable that receives the
 *                            accepted socket object.
 * @param[out] accept_address Pointer to the variable that receives the
 *                            address of the connection destination. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSAcceptSocket(OSSocket* socket,
                       OSSocket** accept_socket,
                       OSSocketAddressInet* accept_address) {
  static const OSFunctionId kFuncId = kIdOSAcceptSocket;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (accept_socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);

  sockaddr_in addr = {};
  socklen_t addr_size = sizeof(addr);

  int32_t accept_fd = accept(socket_fd,
                             reinterpret_cast<sockaddr*>(&addr), &addr_size);
  if (accept_fd < 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  OSSocket* new_socket = GetOSSocket(accept_fd);

  SocketInfo info = {};
  info.binded = false;
  info.writable = true;
  OSErrorCause cause =
      SocketInfoManager::GetInstance()->Insert(new_socket, info);
  if (cause != kErrorNone) {
    close(accept_fd);
    SENSCORD_OSAL_LOG_ERROR("Insert(SocketInfo) failed. cause=%d", cause);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }

  *accept_socket = new_socket;

  if (accept_address != NULL) {
    accept_address->port = addr.sin_port;
    accept_address->address = addr.sin_addr.s_addr;
  }

  return 0;
}

/**
 * @brief Initiate a connection on a socket.
 * @param[in] socket  Socket object.
 * @param[in] address IP address of the connection destination.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSConnectSocket(OSSocket* socket,
                        const OSSocketAddressInet& address) {
  static const OSFunctionId kFuncId = kIdOSConnectSocket;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  SocketInfo info = {};
  OSErrorCause cause = SocketInfoManager::GetInstance()->Get(socket, &info);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  int32_t socket_fd = GetSocketFd(socket);

  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = address.port;
  addr.sin_addr.s_addr = address.address;

  int32_t ret = connect(socket_fd,
                        reinterpret_cast<const sockaddr*>(&addr),
                        sizeof(addr));
  if (ret != 0) {
    cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  info.writable = true;
  cause = SocketInfoManager::GetInstance()->Set(socket, info);
  if (cause != kErrorNone) {
    SENSCORD_OSAL_LOG_ERROR("Set(SocketInfo) failed. cause=%d", cause);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }

  return 0;
}

/**
 * @brief Initiate a connection on a socket.
 * @param[in] socket  Socket object.
 * @param[in] address IP address of the connection destination.
 * @param[in] relative_timeout Timeout relative time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSConnectSocket(OSSocket* socket,
                        const OSSocketAddressInet& address,
                        uint64_t relative_timeout) {
  static const OSFunctionId kFuncId = kIdOSConnectSocket;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  SocketInfo info = {};
  OSErrorCause cause = SocketInfoManager::GetInstance()->Get(socket, &info);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  int32_t socket_fd = GetSocketFd(socket);

  if (info.writable) {
    int32_t socket_type = 0;
    GetSocketType(socket_fd, &socket_type);
    if (socket_type == SOCK_STREAM) {
      return OSMakeErrorCode(kFuncId, kErrorIsConnected);
    }
  }

  if (relative_timeout != kConnectTimeoutDefault) {
    // Set non-blocking socket
    int32_t flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
  }

  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = address.port;
  addr.sin_addr.s_addr = address.address;

  int32_t ret = connect(
      socket_fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));

  if (ret != 0) {
    SENSCORD_OSAL_LOG_DEBUG("connect() errno=%" PRId32, errno);
    if ((relative_timeout != kConnectTimeoutDefault) &&
        (errno == EINPROGRESS)) {
      // non-blocking
      fd_set wfds, xfds;
      FD_ZERO(&wfds);
      FD_ZERO(&xfds);
      FD_SET(socket_fd, &wfds);
      FD_SET(socket_fd, &xfds);
      timeval timeout = ToTimeval(relative_timeout);

      ret = select(socket_fd + 1, NULL, &wfds, &xfds, &timeout);
      if (ret == 0) {
        SENSCORD_OSAL_LOG_DEBUG("connect(select) timeout");
        cause = kErrorTimedOut;
      } else if (ret < 0) {
        cause = GetErrorCauseFromErrno(errno);
      } else {
        int32_t optval = 0;
        socklen_t optlen = sizeof(optval);
        ret = getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &optval, &optlen);
        if (ret < 0) {
          cause = GetErrorCauseFromErrno(errno);
        } else if (optval != 0) {
          SENSCORD_OSAL_LOG_WARNING("connect() SO_ERROR=%" PRId32, optval);
          if (optval == ECONNREFUSED) {
            // FIXME(nakashima): ECONNREFUSED error in non-blocking connect()
            SENSCORD_OSAL_LOG_ERROR(
                "ECONNREFUSED error in non-blocking `connect()` "
                "will break the socket");
          }
          cause = GetErrorCauseFromErrno(optval);
        }
      }
    } else {
      cause = GetErrorCauseFromErrno(errno);
    }
  }

  if (relative_timeout != kConnectTimeoutDefault) {
    // Reset blocking socket
    int32_t flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags & ~O_NONBLOCK);
  }

  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }

  info.writable = true;
  cause = SocketInfoManager::GetInstance()->Set(socket, info);
  if (cause != kErrorNone) {
    SENSCORD_OSAL_LOG_ERROR("Set(SocketInfo) failed. cause=%d", cause);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }

  return 0;
}

/**
 * @brief Send a message on a socket.
 * @param[in]  socket      Socket object.
 * @param[in]  buffer      Pointer to the buffer.
 * @param[in]  buffer_size Buffer length in bytes.
 * @param[out] sent_size   Pointer to the variable that receives the size
 *                         sent. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSendSocket(OSSocket* socket,
                     const void* buffer,
                     size_t buffer_size,
                     size_t* sent_size) {
  static const OSFunctionId kFuncId = kIdOSSendSocket;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (buffer == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);

  int32_t flags = 0;
  flags |= MSG_NOSIGNAL;  // Do not generate SIGPIPE.
  ssize_t ret_size = send(socket_fd, buffer, buffer_size, flags);
  if (ret_size < 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  if (sent_size != NULL) {
    *sent_size = static_cast<size_t>(ret_size);
  }

  return 0;
}

/**
 * @brief Send a message on a socket.
 * @param[in]  socket       Socket object.
 * @param[in]  buffer       Pointer to the buffer.
 * @param[in]  buffer_size  Buffer length in bytes.
 * @param[in]  dest_address IP address of the destination. (optional)
 * @param[out] sent_size    Pointer to the variable that receives the size
 *                          sent. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSendToSocket(OSSocket* socket,
                       const void* buffer,
                       size_t buffer_size,
                       const OSSocketAddressInet* dest_address,
                       size_t* sent_size) {
  static const OSFunctionId kFuncId = kIdOSSendToSocket;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (buffer == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);

  const sockaddr* addr = NULL;
  socklen_t addr_size = 0;
  sockaddr_in addr_in = {};

  if (dest_address != NULL) {
    addr = reinterpret_cast<const sockaddr*>(&addr_in);
    addr_size = sizeof(addr_in);
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = dest_address->port;
    addr_in.sin_addr.s_addr = dest_address->address;
  }

  int32_t flags = 0;
  flags |= MSG_NOSIGNAL;  // Do not generate SIGPIPE.
  ssize_t ret_size = sendto(socket_fd, buffer, buffer_size, flags,
                            addr, addr_size);
  if (ret_size < 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  if (sent_size != NULL) {
    *sent_size = static_cast<size_t>(ret_size);
  }

  return 0;
}

/**
 * @brief Concatenate multiple messages and send with socket.
 *
 * For unconnected DGRAM socket, specify dest_address.
 *
 * @param[in]  socket       Socket object.
 * @param[in]  messages     List of messages to concatenate.
 * @param[in]  dest_address IP address of the destination. (optional)
 * @param[out] sent_size    Pointer to the variable that receives the size
 *                          sent. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSendMsgSocket(OSSocket* socket,
                        const std::vector<OSSocketMessage>& messages,
                        const OSSocketAddressInet* dest_address,
                        size_t* sent_size) {
  static const OSFunctionId kFuncId = kIdOSSendMsgSocket;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (messages.empty()) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);

  // Preparation of msghdr.
  std::vector<iovec> vec;
  std::vector<OSSocketMessage>::const_iterator itr = messages.begin();
  std::vector<OSSocketMessage>::const_iterator itr_end = messages.end();
  for (; itr != itr_end; ++itr) {
    iovec iov = {};
    iov.iov_base = itr->buffer;
    iov.iov_len = itr->buffer_size;
    vec.push_back(iov);
  }

  msghdr msg = {};
  msg.msg_iov = &vec[0];
  msg.msg_iovlen = messages.size();

  sockaddr_in addr_in = {};
  if (dest_address != NULL) {
    msg.msg_name = reinterpret_cast<sockaddr*>(&addr_in);
    msg.msg_namelen = sizeof(addr_in);
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = dest_address->port;
    addr_in.sin_addr.s_addr = dest_address->address;
  }

  // sendmsg
  int32_t flags = 0;
  flags |= MSG_NOSIGNAL;  // Do not generate SIGPIPE.
  ssize_t ret_size = sendmsg(socket_fd, &msg, flags);
  if (ret_size < 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  if (sent_size != NULL) {
    *sent_size = static_cast<size_t>(ret_size);
  }

  return 0;
}

/**
 * @brief Receive a message from a socket.
 * @param[in]  socket        Socket object.
 * @param[out] buffer        Pointer to the buffer to receive the incoming
 *                           data.
 * @param[in]  buffer_size   Buffer length in bytes.
 * @param[out] received_size Pointer to the variable that receives the size
 *                           received. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSRecvSocket(OSSocket* socket,
                     void* buffer,
                     size_t buffer_size,
                     size_t* received_size) {
  static const OSFunctionId kFuncId = kIdOSRecvSocket;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (buffer == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);

  ssize_t ret_size = recv(socket_fd, buffer, buffer_size, 0);
  if (ret_size < 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  if (received_size != NULL) {
    *received_size = static_cast<size_t>(ret_size);
  }

  return 0;
}

/**
 * @brief Receive a message from a socket.
 * @param[in]  socket         Socket object.
 * @param[out] buffer         Pointer to the buffer to receive the incoming
 *                            data.
 * @param[in]  buffer_size    Buffer length in bytes.
 * @param[out] source_address Pointer to the variable that receives the
 *                            IP address of the source. (optional)
 * @param[out] received_size  Pointer to the variable that receives the size
 *                            received. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSRecvFromSocket(OSSocket* socket,
                         void* buffer,
                         size_t buffer_size,
                         OSSocketAddressInet* source_address,
                         size_t* received_size) {
  static const OSFunctionId kFuncId = kIdOSRecvFromSocket;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (buffer == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);

  sockaddr* addr = NULL;
  socklen_t* addr_size_ptr = NULL;
  sockaddr_in addr_in = {};
  socklen_t addr_size = 0;

  if (source_address != NULL) {
    addr = reinterpret_cast<sockaddr*>(&addr_in);
    addr_size = sizeof(addr_in);
    addr_size_ptr = &addr_size;
  }

  ssize_t ret_size = recvfrom(socket_fd, buffer, buffer_size, 0,
                              addr, addr_size_ptr);
  if (ret_size < 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  if (source_address != NULL) {
    source_address->port = addr_in.sin_port;
    source_address->address = addr_in.sin_addr.s_addr;
  }

  if (received_size != NULL) {
    *received_size = static_cast<size_t>(ret_size);
  }

  return 0;
}

/**
 * @brief Make fd_set from socket list(vector::<OSSocket*>).
 */
static fd_set* MakeFdSet(SelectType type,
                         const std::vector<OSSocket*>* sockets,
                         fd_set* fdset, int32_t* maxfd) {
  if ((sockets == NULL) || (fdset == NULL) || (maxfd == NULL)) {
    return NULL;
  }
  int32_t temp_max = *maxfd;
  uint32_t count = 0;
  FD_ZERO(fdset);
  for (std::vector<OSSocket*>::const_iterator itr = sockets->begin();
      itr != sockets->end(); ++itr) {
    OSSocket* socket = *itr;
    int32_t socket_fd = GetSocketFd(socket);
    if (type == kSelectWrite) {
      SocketInfo info = {};
      OSErrorCause cause =
          SocketInfoManager::GetInstance()->Get(socket, &info);
      if (cause == kErrorNone) {
        if (!info.writable) {
          continue;
        }
      }
    }
    if (socket_fd < FD_SETSIZE) {
      FD_SET(socket_fd, fdset);
      temp_max = std::max(temp_max, socket_fd);
      ++count;
    }
  }
  if (count == 0) {
    return NULL;
  }
  *maxfd = temp_max;
  return fdset;
}

/**
 * @brief Set the result of fd_set to the socket list(vector::<OSSocket*>).
 */
static void SetSocketList(const fd_set* fdset,
                          std::vector<OSSocket*>* sockets) {
  if ((sockets == NULL) || (fdset == NULL)) {
    return;
  }
  for (std::vector<OSSocket*>::iterator itr = sockets->begin();
      itr != sockets->end();) {
    OSSocket* socket = *itr;
    int32_t socket_fd = GetSocketFd(socket);
    if (!FD_ISSET(socket_fd, fdset)) {
      itr = sockets->erase(itr);
    } else {
      ++itr;
    }
  }
}

/**
 * @brief Determine the state of one or more sockets and perform
 *        synchronous I/O.
 */
static OSErrorCause SelectSocket(
    std::vector<OSSocket*>* read_sockets,
    std::vector<OSSocket*>* write_sockets,
    std::vector<OSSocket*>* except_sockets,
    const uint64_t* nano_seconds) {
  if ((read_sockets != NULL && read_sockets->size() > FD_SETSIZE) ||
      (write_sockets != NULL && write_sockets->size() > FD_SETSIZE) ||
      (except_sockets != NULL && except_sockets->size() > FD_SETSIZE)) {
    return kErrorInvalidArgument;
  }
  int32_t maxfd = 0;
  fd_set rfds, wfds, xfds;
  fd_set* rfds_ptr = MakeFdSet(kSelectRead, read_sockets, &rfds, &maxfd);
  fd_set* wfds_ptr = MakeFdSet(kSelectWrite, write_sockets, &wfds, &maxfd);
  fd_set* xfds_ptr = MakeFdSet(kSelectExcept, except_sockets, &xfds, &maxfd);

  if (maxfd == 0) {
    return kErrorInvalidArgument;
  }

  timeval* timeout_ptr = NULL;
  timeval timeout = {};
  if (nano_seconds != NULL) {
    timeout = ToTimeval(*nano_seconds);
    timeout_ptr = &timeout;
  }

  int32_t ret = select(maxfd + 1, rfds_ptr, wfds_ptr, xfds_ptr, timeout_ptr);
  if (ret < 0) {
    return GetErrorCauseFromErrno(errno);
  } else if (ret == 0) {
    SENSCORD_OSAL_LOG_DEBUG("timedout");
    return kErrorTimedOut;
  }

  SetSocketList(rfds_ptr, read_sockets);
  SetSocketList(wfds_ptr, write_sockets);
  SetSocketList(xfds_ptr, except_sockets);

  return kErrorNone;
}

/**
 * @brief Determine the state of one or more sockets and perform
 *        synchronous I/O.
 * @param[in,out] read_sockets   Pointer to list of sockets to be checked for
 *                               readability. (optional)
 * @param[in,out] write_sockets  Pointer to list of sockets to be checked for
 *                               writability. (optional)
 * @param[in,out] except_sockets Pointer to list of sockets to be checked for
 *                               errors. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSelectSocket(
    std::vector<OSSocket*>* read_sockets,
    std::vector<OSSocket*>* write_sockets,
    std::vector<OSSocket*>* except_sockets) {
  static const OSFunctionId kFuncId = kIdOSSelectSocket;
  OSErrorCause cause =
      SelectSocket(read_sockets, write_sockets, except_sockets, NULL);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Determine the state of one or more sockets and perform
 *        synchronous I/O.
 * @param[in,out] read_sockets   Pointer to list of sockets to be checked for
 *                               readability. (optional)
 * @param[in,out] write_sockets  Pointer to list of sockets to be checked for
 *                               writability. (optional)
 * @param[in,out] except_sockets Pointer to list of sockets to be checked for
 *                               errors. (optional)
 * @param[in]     nano_seconds   Timeout relative time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSRelativeTimedSelectSocket(
    std::vector<OSSocket*>* read_sockets,
    std::vector<OSSocket*>* write_sockets,
    std::vector<OSSocket*>* except_sockets,
    uint64_t nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSRelativeTimedSelectSocket;
  OSErrorCause cause =
      SelectSocket(read_sockets, write_sockets, except_sockets, &nano_seconds);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Determine the state of one or more sockets and perform
 *        synchronous I/O.
 * @param[in,out] read_sockets   Pointer to list of sockets to be checked for
 *                               readability. (optional)
 * @param[in,out] write_sockets  Pointer to list of sockets to be checked for
 *                               writability. (optional)
 * @param[in,out] except_sockets Pointer to list of sockets to be checked for
 *                               errors. (optional)
 * @param[in]     nano_seconds   Timeout absolute time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSTimedSelectSocket(
    std::vector<OSSocket*>* read_sockets,
    std::vector<OSSocket*>* write_sockets,
    std::vector<OSSocket*>* except_sockets,
    uint64_t nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSTimedSelectSocket;
  uint64_t curr_nanosec = 0;
  int32_t ret = OSGetTime(&curr_nanosec);
  if (ret != 0) {
    SENSCORD_OSAL_LOG_ERROR("OSGetTime failed. ret=0x%" PRIx32, ret);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }
  uint64_t rel_timeout = 0;
  if (nano_seconds > curr_nanosec) {
    rel_timeout = nano_seconds - curr_nanosec;
  }
  OSErrorCause cause =
      SelectSocket(read_sockets, write_sockets, except_sockets, &rel_timeout);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Convert uint32_t from host to network byte order. (which is
 *        big-endian)
 * @param[in] hostlong  32-bit number in host byte order.
 * @return 32-bit number in network byte order.
 */
uint32_t OSHtonl(uint32_t hostlong) {
  uint32_t netlong = htonl(hostlong);
  return netlong;
}

/**
 * @brief Convert uint16_t from host to network byte order. (which is
 *        big-endian).
 * @param[in] hostshort  16-bit number in host byte order.
 * @return 16-bit number in network byte order.
 */
uint16_t OSHtons(uint16_t hostshort) {
  uint16_t netshort = htons(hostshort);
  return netshort;
}

/**
 * @brief Convert uint32_t from network to host byte order.
 * @param[in] netlong  32-bit number in network byte order.
 * @return 32-bit number in host byte order.
 */
uint32_t OSNtohl(uint32_t netlong) {
  uint32_t hostlong = ntohl(netlong);
  return hostlong;
}

/**
 * @brief Convert uint16_t from network to host byte order.
 * @param[in] netshort  16-bit number in network byte order.
 * @return 16-bit number in host byte order.
 */
uint16_t OSNtohs(uint16_t netshort) {
  uint16_t hostshort = ntohs(netshort);
  return hostshort;
}

/**
 * @brief Convert a string IPv4 address to binary data in network byte order.
 * @param[in]  source_address      String IPv4 address.
 * @param[out] destination_address IPv4 address in network byte order.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSInetAton(const char* source_address,
                   uint32_t* destination_address) {
  static const OSFunctionId kFuncId = kIdOSInetAton;
  if (source_address == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (destination_address == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  in_addr addr = {};
  int32_t ret = inet_aton(source_address, &addr);
  if (ret == 0) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  *destination_address = addr.s_addr;
  return 0;
}

/**
 * @brief Convert the binary data given in network byte order, to a string
 *        IPv4 address.
 * @param[in]  source_address      IPv4 address in network byte order.
 * @param[out] destination_address Pointer to the buffer to receive the string
 *                                 IPv4 address.
 * @param[in]  destination_size    Buffer length in bytes.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSInetNtoa(uint32_t source_address,
                   char* destination_address,
                   size_t destination_size) {
  static const OSFunctionId kFuncId = kIdOSInetNtoa;
  if (destination_address == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  in_addr addr = {};
  addr.s_addr = source_address;
  const char* ptr = inet_ntop(AF_INET, &addr, destination_address,
                              static_cast<socklen_t>(destination_size));
  if (ptr == NULL) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Set the send buffer size.
 * @param[in] socket      Socket object.
 * @param[in] buffer_size Buffer size in bytes.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSetSocketSendBufferSize(OSSocket* socket, uint32_t buffer_size) {
  static const OSFunctionId kFuncId = kIdOSSetSocketSendBufferSize;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);
  int32_t ret = setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF,
                           &buffer_size, sizeof(buffer_size));
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Set the receive buffer size.
 * @param[in] socket      Socket object.
 * @param[in] buffer_size Buffer size in bytes.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSetSocketRecvBufferSize(OSSocket* socket, uint32_t buffer_size) {
  static const OSFunctionId kFuncId = kIdOSSetSocketRecvBufferSize;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);
  int32_t ret = setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF,
                           &buffer_size, sizeof(buffer_size));
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Set rules for reuse of bind address.
 *
 * It must be called before OSBindSocket.
 *
 * @param[in] socket  Socket object.
 * @param[in] flag    true: Reuse is enabled.
 *                    false: Reuse is disabled.
 */
int32_t OSSetSocketReuseAddr(OSSocket* socket, bool flag) {
  static const OSFunctionId kFuncId = kIdOSSetSocketReuseAddr;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);
  int32_t enable = 0;
  if (flag) {
    enable = 1;
  }
  int32_t ret = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
                           &enable, sizeof(enable));
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Set the socket option for TCP_NODELAY.
 *
 * @param[in] socket  Socket object.
 * @param[in] enabled true: Enabling TCP_NODELAY.
 *                    false: Disabling TCP_NODELAY.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSetSocketTcpNoDelay(OSSocket* socket, bool enabled) {
  static const OSFunctionId kFuncId = kIdOSSetSocketTcpNoDelay;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);
  int32_t flag = 0;
  if (enabled) {
    flag = 1;
  }
  int32_t ret = setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY,
                           &flag, sizeof(flag));
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Get a list of IPv4 addresses of the terminal.
 *
 * @param[out] addr_list  Pointer to a variable that
 *                        stores a list of IPv4 addresses.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetInetAddressList(std::vector<OSSocketAddressInet>* addr_list) {
  static const OSFunctionId kFuncId = kIdOSGetInetAddressList;
  if (addr_list == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  ifaddrs* list = NULL;
  int32_t ret = getifaddrs(&list);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  for (ifaddrs* ifa = list; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL) {
      continue;
    }
    if (ifa->ifa_addr->sa_family == AF_INET) {
      sockaddr_in* sin = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
      OSSocketAddressInet addr = {};
      addr.address = sin->sin_addr.s_addr;
      addr_list->push_back(addr);
    }
  }
  freeifaddrs(list);
  return 0;
}

}  // namespace osal
}  // namespace senscord
