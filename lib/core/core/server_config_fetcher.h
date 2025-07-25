/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CORE_SERVER_CONFIG_FETCHER_H_
#define LIB_CORE_CORE_SERVER_CONFIG_FETCHER_H_

#include <string>

#include "core/server_config_manager.h"
#include "senscord/connection.h"
#include "senscord/noncopyable.h"
#include "senscord/osal.h"
#include "senscord/status.h"

namespace senscord {

// pre-definition
class ServerConfigManager;

/**
 * @brief Server config fetcher.
 */
class ServerConfigFetcher : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   */
  ServerConfigFetcher(const std::string& type, const std::string& address,
                      ServerConfigManager* parent);

  /**
   * @brief Destructor.
   */
  virtual ~ServerConfigFetcher();

  /**
   * @brief Get senscord config.
   * @return Status object.
   */
  Status RequestConfig();

  /**
   * @brief The method of connection thread.
   */
  void ConnectionThreadCore();

  /**
   * @brief Wait connection thread join.
   */
  void WaitPostProcess();

  /**
   * @brief Get server type.
   */
  const std::string GetServerType() const;

  /**
   * @brief Get server address.
   */
  const std::string GetServerAddress() const;

 private:
  /**
   * @brief Send get config request to server.
   * @return Status object.
   */
  Status SendGetConfigCommand();

  /**
   * @brief Dealing the received message.
   * @param[in] (msg) The new incoming message.
   * @return Successfully processed GetConfig reply.
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
  ServerConfigManager* parent_manager_;
  osal::OSThread* recv_thread_;
};

}  // namespace senscord

#endif  // LIB_CORE_CORE_SERVER_CONFIG_FETCHER_H_
