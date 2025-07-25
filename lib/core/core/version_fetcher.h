/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CORE_VERSION_FETCHER_H_
#define LIB_CORE_CORE_VERSION_FETCHER_H_

#include <stdint.h>
#include <string>
#include <map>

#include "core/internal_types.h"
#include "core/version_manager.h"
#include "senscord/senscord_types.h"
#include "senscord/property_types.h"
#include "senscord/status.h"
#include "senscord/connection.h"
#include "senscord/noncopyable.h"
#include "util/mutex.h"
#include "util/autolock.h"

namespace senscord {

// pre-definition
class VersionManager;

/**
 * @brief Version manager.
 */
class VersionFetcher : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   */
  VersionFetcher(
      const std::string& type,
      const std::string& address,
      VersionManager* parent);

  /**
   * @brief Destructor.
   */
  virtual ~VersionFetcher();

  /**
   * @brief Get senscord version.
   * @return Status object.
   */
  Status RequestVersion();

  /**
   * @brief The method of connection thread.
   */
  void ConnectionThreadCore();

  /**
   * @brief Wait connection thread join.
   */
  void WaitPostProcess();

 private:
  /**
   * @brief Send get version request to server.
   * @return Status object.
   */
  Status SendGetVersionCommand();

  /**
   * @brief Dealing the received message.
   * @param[in] (msg) The new incoming message.
   * @return Successfully processed Getversion reply.
   */
  bool DealMessage(Message* msg);

  /**
   * @brief Release the reply message.
   * @param[in] (msg) The reply message.
   */
  void ReleaseCommandReply(Message* msg);

  /**
   * @brief Notify the cancellation in response to the parent request.
   * @param[in] (status) Error status.
   */
  void NotifyCancel(const Status& status);

  /**
   * @brief Receiving response from server.
   */
  void ReceivingProcess();

  volatile bool is_connected_;
  Connection* connection_;
  std::string type_;
  std::string address_;
  VersionManager* parent_manager_;
  osal::OSThread* recv_thread_;
};

}  // namespace senscord

#endif  // LIB_CORE_CORE_VERSION_FETCHER_H_
