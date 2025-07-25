/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APPLICATION_MULTI_SERVER_MULTI_SERVER_H_
#define APPLICATION_MULTI_SERVER_MULTI_SERVER_H_

#include <string>
#include <vector>
#include <map>
#include "senscord/senscord.h"
#include "senscord/connection.h"
#ifdef SENSCORD_SERVER_SEARCH_SSDP
#include "searcher/ssdp_module.h"
#endif  // SENSCORD_SERVER_SEARCH_SSDP

namespace senscord {
namespace server {

class ClientListenerBase;
class ClientAdapterManager;
class ConfigManager;

/**
 * @brief Listener settings.
 */
struct ListenerSetting {
  /** Connection key. */
  std::string connection;
  /** Primary listen address. */
  std::string address_primary;
  /** Secondary listen address. */
  std::string address_secondary;
};

/** Connection buffering settings (key=Connection key). */
typedef std::map<std::string, FrameBuffering> ConnectionBuffering;

/**
 * @brief Stream settings.
 */
struct StreamSetting {
  /** Stream key. */
  std::string stream_key;
  /** Buffering settings. */
  ConnectionBuffering buffering;
};

/**
 * @brief Configurations for the server functions.
 */
struct ServerConfig {
  /** True means enabling the client function. */
  bool is_enabled_client;

  /** List of listener settings. */
  std::vector<ListenerSetting> listeners;

  /**
   * Settings for each stream. (Stream key + setting)
   * To override the default setting, register with an empty stream key.
   */
  std::vector<StreamSetting> streams;
};

/**
 * @brief The host server class for SDK.
 */
class MultiServer : private util::Noncopyable {
 public:
  /**
   * @brief Open the host server.
   * @param[in] (config_path) The path of server configuration file.
   * @return Status object.
   */
  Status Open(const std::string& config_path);

  /**
   * @brief Close the host server.
   * @return Status object.
   */
  Status Close();

  /**
   * @brief Constructor.
   */
  MultiServer();

  /**
   * @brief Destructor.
   */
  virtual ~MultiServer();

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

#ifdef SENSCORD_SERVER_SEARCH_SSDP
  /**
   * @brief SSDP Server.
   */
  std::vector<SsdpModule*> ssdp_servers_;
#endif  // SENSCORD_SERVER_SEARCH_SSDP
};

}   // namespace server
}   // namespace senscord

#endif  // APPLICATION_MULTI_SERVER_MULTI_SERVER_H_
