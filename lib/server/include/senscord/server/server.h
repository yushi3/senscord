/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_SERVER_SERVER_H_
#define SENSCORD_SERVER_SERVER_H_

#include <string>
#include <vector>
#include <map>

#include "senscord/config.h"
#include "senscord/senscord.h"
#include "senscord/connection.h"

namespace senscord {
namespace server {

class ClientListenerBase;
class ClientAdapterManager;
class ConfigManager;

/**
 * @brief Configurations for the server functions.
 */
struct ServerConfig {
  /** Primary bind address. (for Connection::Bind()) */
  std::string bind_config;

  /** Secondary bind address. */
  std::string bind_config2;

  /** True means enabling the client function. */
  bool is_enabled_client;

  /** Settings for each stream. (Stream key + setting)
   * To override the default setting, register with an empty stream key.
   */
  std::map<std::string, OpenStreamSetting> streams;
};

/**
 * @brief The host server class for SDK.
 */
class Server : private util::Noncopyable {
 public:
  /**
   * @brief Open the host server.
   * @param[in] (listener) The connection interface instance.
   * @param[in] (config_path) The path of server configuration file.
   * @return Status object.
   */
  Status Open(Connection* listener, const std::string& config_path) {
    return Open(listener, config_path, NULL);
  }

  /**
   * @brief Open the host server.
   * @param[in] (listener) The connection interface instance.
   * @param[in] (config) The configuration for the server functions.
   * @return Status object.
   */
  Status Open(Connection* listener, const ServerConfig& config) {
    return Open(listener, config, NULL);
  }

  /**
   * @brief Open the host server.
   * @param[in] (listener) The primary connection interface instance.
   * @param[in] (config_path) The path of server configuration file.
   * @param[in] (listener2) The secondary connection interface instance.
   * @return Status object.
   */
  Status Open(Connection* listener, const std::string& config_path,
              Connection* listener2);

  /**
   * @brief Open the host server.
   * @param[in] (listener) The primary connection interface instance.
   * @param[in] (config) The configuration for the server functions.
   * @param[in] (listener2) The secondary connection interface instance.
   * @return Status object.
   */
  Status Open(Connection* listener, const ServerConfig& config,
              Connection* listener2);

  /**
   * @brief Close the host server.
   * @return Status object.
   */
  Status Close();

  /**
   * @brief Constructor.
   */
  Server();

  /**
   * @brief Destructor.
   */
  virtual ~Server();

 private:
  /**
   * @brief Client listener list.
   */
  std::vector<ClientListenerBase*> listeners_;

  /**
   * @brief Client manager.
   */
  ClientAdapterManager* client_manager_;

  /**
   * @brief The instance of SDK Core.
   */
  Core* core_;

  /**
   * @brief Config manager.
   */
  ConfigManager* config_manager_;
};

}   // namespace server
}   // namespace senscord

#endif  // SENSCORD_SERVER_SERVER_H_
