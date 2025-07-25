/*
 * SPDX-FileCopyrightText: 2020-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CORE_SERVER_CONFIG_MANAGER_H_
#define LIB_CORE_CORE_SERVER_CONFIG_MANAGER_H_

#include "senscord/config.h"

#ifdef SENSCORD_SERVER_SETTING

#include <map>
#include <string>
#include <utility>  // std::pair
#include <vector>

#include "core/internal_types.h"
#include "core/server_config_fetcher.h"
#include "senscord/noncopyable.h"
#include "senscord/osal.h"
#include "senscord/senscord_types.h"
#include "senscord/status.h"
#include "util/mutex.h"

namespace senscord {

// pre-definition
class ServerConfigFetcher;

/**
 * @brief Server config manager.
 */
class ServerConfigManager : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   */
  ServerConfigManager();

  /**
   * @brief Destructor.
   */
  virtual ~ServerConfigManager();

  /**
   * @brief Get server config.
   * @param[in/out] (core_config) Core configuration.
   * @param[in] (identification) SensCord identification.
   * @return Status object.
   */
  Status GetServerConfig(CoreConfig* core_config,
                         const std::string& identification);

  /**
   * @brief Notify server config.
   * @param[in] (fetcher) Fetcher subject to notification.
   * @param[in] (config) server config, NULL is cancel.
   * @param[in] (status) Server status.
   */
  void NotifyServerConfig(const ServerConfigFetcher* fetcher,
                          const ServerConfig* config, const Status& status);

 private:
  /**
   * @brief Get server configs.
   * @param[in] (server_list) Target servers.
   * @return Status object.
   */
  Status GetServerConfigs(const std::vector<ServerSetting>& server_list);

  /**
   * @brief Wait for fetch server configs.
   * @return Status object.
   */
  Status WaitFetchServerConfig();

  /**
   * @brief Get server information.
   * @param[in] (server_setting) Server setting.
   * @param[out] (type) Connection type.
   * @param[out] (address) Destination address.
   * @return Status object.
   */
  Status GetServerInfo(const ServerSetting& server_setting, std::string* type,
                       std::string* address);

  /**
   * @brief Update core config.
   * @param[in/out] (core_config) Core configuration.
   * @param[in] (identification) SensCord identification.
   * @return Status object.
   */
  Status UpdateCoreConfig(CoreConfig* core_config,
                          const std::string& identification);

  /**
   * @brief Add allocator keys.
   * @param[out] (allocator_key_list) Allocator key list of client instance.
   * @param[in] (instance_name) Radical instance name of stream.
   * @param[in] (instance_list) Instance list of server.
   * @return Status object.
   */
  Status AddAllocatorKey(
      std::map<std::string, std::string>* allocator_key_list,
      const std::string& instance_name,
      const std::vector<ServerComponentInstanceConfig>& instance_list);

  /**
   * @brief Fetcher value.
   */
  struct FetcherValue {
    bool is_finished;
    Status status;
    ServerSetting server_setting;
  };
  typedef std::map<ServerConfigFetcher*, FetcherValue> Fetchers;

  /**
   * @brief Server config info.
   */
  struct ServerConfigInfo {
    std::string type;
    std::string address;
    ServerSetting server_setting;
  };

  Fetchers waiting_fetcher_map_;
  std::vector<std::pair<ServerConfig, ServerConfigInfo> > server_config_list_;
  util::Mutex* request_mutex_;
  util::Mutex* request_waiting_mutex_;
  senscord::osal::OSCond* request_waiting_cond_;
};

}  // namespace senscord

#else

#include "senscord/noncopyable.h"

namespace senscord {

class ServerConfigManager : private util::Noncopyable {
};

}  // namespace senscord

#endif  // SENSCORD_SERVER_SETTING

#endif  // LIB_CORE_CORE_SERVER_CONFIG_MANAGER_H_
