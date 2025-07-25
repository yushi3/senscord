/*
 * SPDX-FileCopyrightText: 2020-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iterator>
#include <set>
#include <algorithm>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>  // select
#endif

#include "logger/logger.h"
#include "util/senscord_utils.h"
#include "lssdp/lssdp.h"
#include "searcher/ssdp_module.h"

namespace senscord {

static const char* kSearchTarget = "ST_SensCode";
static const char* kConnection = "tcp";
static const char* kAttributeTimeout = "timeout";
static const int kDefaultTimeoutMilliseconds = 1000;

/**
 * @brief Callback func for lssdp.
 * @param[in] (lssdp) lssdp context.
 */
static int show_neighbor_list(lssdp_ctx* lssdp) {
  senscord::SsdpModule* parent =
      reinterpret_cast<senscord::SsdpModule*>(lssdp->parent);
  lssdp_nbr* nbr;
  for (nbr = lssdp->neighbor_list; nbr != NULL; nbr = nbr->next) {
    senscord::DeviceAddress deviceAddress;
    deviceAddress.connection =
        std::string(nbr->connection, strnlen(nbr->connection, LSSDP_FIELD_LEN));
    deviceAddress.address =
        std::string(nbr->address, strnlen(nbr->address, LSSDP_FIELD_LEN));
    deviceAddress.addressSecondary = std::string(
        nbr->address_secondly, strnlen(nbr->address_secondly, LSSDP_FIELD_LEN));
    parent->AddDeviceAddress(deviceAddress);
  }
  return 0;
}

/**
 * @brief Callback func for lssdp.
 * @param[in] (lssdp) lssdp context.
 */
static int show_interface_list_and_rebind_socket(lssdp_ctx* lssdp) {
  // re-bind SSDP socket
  if (lssdp_socket_create(lssdp) != 0) {
    return -1;
  }
  return 0;
}

/**
 * @brief The thread processing for server.
 * @param[in] (arg) SsdpModule pointer.
 * @return Don't care.
 */
static senscord::osal::OSThreadResult ServerThreadProcess(void* arg) {
  if (arg) {
    SsdpModule* ssdp = reinterpret_cast<SsdpModule*>(arg);
    ssdp->ServerThread();
  }
  return static_cast<senscord::osal::OSThreadResult>(0);
}

/**
 * @brief Constructor.
 */
SsdpModule::SsdpModule() : lssdp_(), server_thread_(), address_list_() {
  address_list_.clear();
}

/**
 * @brief Destructor.
 */
SsdpModule::~SsdpModule() {
  std::vector<DeviceAddress>().swap(address_list_);
  if (lssdp_) {
    delete lssdp_;
    lssdp_ = NULL;
  }
}

/**
 * @brief Search servers.
 * @param[in] (arguments) arguments map.
 * @return Status object.
 */
Status SsdpModule::Init(const std::map<std::string, std::string>& arguments) {
  int timeoutMilliseconds = kDefaultTimeoutMilliseconds;
  std::map<std::string, std::string>::const_iterator itr =
      arguments.find(kAttributeTimeout);
  if (itr != arguments.end()) {
    if (!util::StrToInt(itr->second, &timeoutMilliseconds)) {
      SENSCORD_LOG_WARNING(
          "can not be converted to a number, use default value : %s=%s",
          kAttributeTimeout, itr->second.c_str());
      timeoutMilliseconds = kDefaultTimeoutMilliseconds;
    }
  }

  lssdp_ = GetCtx(timeoutMilliseconds, "", "");
  return Status::OK();
}

/**
 * @brief Search servers.
 * @return vector<DeviceAddress>.
 */
const std::vector<senscord::DeviceAddress> SsdpModule::Search() {
  Wait(lssdp_);
  return this->address_list_;
}

/**
 * @brief Get lssdp context for server.
 * @param[in] (port) port.
 * @param[in] (portSecondly) port.
 * @return Status object.
 */
Status SsdpModule::ServerInit(std::string port, std::string portSecondly) {
  lssdp_ = GetCtx(-1, port, portSecondly);
  lssdp_->stop = false;
  return Status::OK();
}

/**
 * @brief Start server.
 * @return Status object.
 */
Status SsdpModule::ServerStart() {
  int32_t ret =
      osal::OSCreateThread(&server_thread_, ServerThreadProcess, this, NULL);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL("messenger", Status::kCauseAborted,
                                "failed to OSCreateThread: %" PRIx32, ret);
  }
  return Status::OK();
}

/**
 * @brief Stop Server.
 * @return Status object.
 */
Status SsdpModule::ServerStop() {
  if (server_thread_) {
    lssdp_->stop = true;
    int32_t ret = senscord::osal::OSJoinThread(server_thread_, NULL);
    if (ret != 0) {
      return SENSCORD_STATUS_FAIL("messenger", Status::kCauseAborted,
                                  "failed to OSJoinThread: %" PRIx32, ret);
    }
    server_thread_ = NULL;
  }
  return Status::OK();
}

/**
 * @brief Add device address.
 * @param[in] (address) device address.
 */
void SsdpModule::AddDeviceAddress(DeviceAddress address) {
  if (address_list_.size() > 0) {
    std::vector<senscord::DeviceAddress>::iterator itr = address_list_.begin();
    std::vector<senscord::DeviceAddress>::iterator end = address_list_.end();
    bool found = false;
    for (; itr != end; ++itr) {
      if (itr->address == address.address) {
        found = true;
      }
    }
    if (!found) {
      address_list_.push_back(address);
    }
  } else {
    address_list_.push_back(address);
  }
}

/**
 * @brief Server thread.
 */
void SsdpModule::ServerThread() { Wait(lssdp_); }

/**
 * @brief Chech TCP Connection.
 * @return true tcp, false other.
 */
bool SsdpModule::IsTcpConnection(std::string connection) {
  return (connection == kConnection);
}

/**
 * @brief Get lssdp context.
 * @param[in] (timeoutMilliseconds) wait time for client.
 * @param[in] (port) port for server.
 * @param[in] (portSecondly) secondly port for server.
 * @return lssdp_ctx.
 */
lssdp_ctx* SsdpModule::GetCtx(int timeoutMilliseconds, std::string port,
                             std::string portSecondly) {
  lssdp_ctx* ret = new lssdp_ctx();

  ret->parent = this;
  ret->search_timeout = timeoutMilliseconds;
  ret->port = 1900;
  ret->neighbor_timeout = 500;

  snprintf(ret->header.search_target, LSSDP_FIELD_LEN - 1, "%s",
           kSearchTarget);
  snprintf(ret->header.connection, LSSDP_FIELD_LEN - 1, "%s",
           kConnection);
  snprintf(ret->header.location.suffix, LSSDP_FIELD_LEN - 1, "%s",
           port.c_str());
  snprintf(ret->header.location.suffix_secondly, LSSDP_FIELD_LEN - 1, "%s",
           portSecondly.c_str());

  // callback
  ret->neighbor_list_changed_callback = show_neighbor_list;
  ret->network_interface_changed_callback =
      show_interface_list_and_rebind_socket;

  return ret;
}

/**
 * @brief Wait SSDP.
 * @param[in] (lssdp) lssdp context.
 */
void SsdpModule::Wait(lssdp_ctx* lssdp) {
  int result = lssdp_init();
  if (result < 0) {
    SENSCORD_LOG_ERROR("lssdp_init");
  }

  /* get network interface at first time, network_interface_changed_callback
   * will be invoke SSDP socket will be created in callback function
   */
  lssdp_network_interface_update(lssdp);

  uint64_t last_time = lssdp_get_current_time();
  int intervalMilliseconds = 500;
  int count = lssdp->search_timeout < 0
                  ? -1 : lssdp->search_timeout / intervalMilliseconds;

  for (;;) {
    size_t loop = (std::min)(lssdp->nwif_num,
                             static_cast<size_t>(LSSDP_INTERFACE_NAME_LEN));
    for (size_t i = 0; i < loop; i++) {
      if (lssdp->sock[i] < 0) {
        SENSCORD_LOG_WARNING("lssdp->sock :%d %d", i, lssdp->sock[i]);
      }
    }

    if (lssdp->stop) {
      break;
    }

    fd_set fs;
    FD_ZERO(&fs);
    int maxfd = 0;
    for (size_t i = 0; i < loop; i++) {
      FD_SET(lssdp->sock[i], &fs);
      maxfd = (std::max)(lssdp->sock[i], maxfd);
    }
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = static_cast<long>(intervalMilliseconds * 1000);

    select(maxfd + 1, &fs, NULL, NULL, &tv);

    for (size_t i = 0; i < loop; i++) {
      if (FD_ISSET(lssdp->sock[i], &fs)) {
        lssdp_socket_read(lssdp, static_cast<int>(i));
      }
    }

    uint64_t current_time = lssdp_get_current_time();
    if ((current_time - last_time)
        >= static_cast<uint64_t>(intervalMilliseconds)) {
      lssdp_network_interface_update(lssdp);  // 1. update network interface
      if (strnlen(lssdp->header.location.suffix, LSSDP_FIELD_LEN) == 0) {
        lssdp_send_msearch(lssdp);  // 2. send M-SEARCH
      }
      if (strnlen(lssdp->header.location.suffix, LSSDP_FIELD_LEN) > 0) {
        lssdp_send_notify(lssdp);  // 3. send NOTIFY
      }
      lssdp_neighbor_check_timeout(lssdp);  // 4. check neighbor timeout

      last_time = current_time;  // update last_time
      // timeout
      if (count == -1) {
      } else if (count == 0) {
        break;
      } else {
        count--;
      }
    }
  }
  lssdp_socket_close(lssdp);
  lssdp_exit();
}

}  // namespace senscord
