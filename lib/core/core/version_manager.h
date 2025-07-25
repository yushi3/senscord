/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CORE_VERSION_MANAGER_H_
#define LIB_CORE_CORE_VERSION_MANAGER_H_

#include "senscord/config.h"

#ifdef SENSCORD_STREAM_VERSION

#include <stdint.h>
#include <string>
#include <map>

#include "core/internal_types.h"
#include "core/config_manager.h"
#include "senscord/senscord_types.h"
#include "senscord/property_types.h"
#include "senscord/status.h"
#include "senscord/noncopyable.h"
#include "util/mutex.h"
#include "util/autolock.h"

namespace senscord {

#ifdef SENSCORD_SERVER
// pre-definition
class VersionFetcher;
#endif  // SENSCORD_SERVER

/**
 * @brief Version manager.
 */
class VersionManager : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   * @param[in] (config_manager) Config manager.
   */
  explicit VersionManager(const ConfigManager* config_manager);

  /**
   * @brief Destructor.
   */
  virtual ~VersionManager();

  /**
   * @brief Get senscord version.
   * @param[out] (version) SensCord version information.
   * @param[in] (is_clientless) Is this clientless process.
   * @return Status object.
   */
  Status GetVersion(SensCordVersion* version, bool is_clientless);

#ifdef SENSCORD_SERVER
  /**
   * @brief Notify server version.
   * @param[in] (fetcher) Fetcher subject to notification.
   * @param[in] (version) Server versions.
   * @param[in] (status) Server status.
   */
  void NotifyServerVersion(
      const VersionFetcher* fetcher, const SensCordVersion* version,
      const Status& status);
#endif  // SENSCORD_SERVER

 private:
  /**
   * @brief Set senscord core version.
   * @return Status object.
   */
  Status SetSensCordVersion(Version* version);

  /**
   * @brief Set senscord project version.
   * @return Status object.
   */
  Status SetProjectVersion(Version* version);

  /**
   * @brief Set senscord stream versions.
   * @return Status object.
   */
  Status SetStreamVersions(std::map<std::string, StreamVersion>* versions);

  /**
   * @brief Set stream version specified by stream key.
   * @param[in] (stream_setting) Target stream setting.
   * @param[out] (version) Stream version.
   * @return Status object.
   */
  Status SetStreamVersion(
      const StreamSetting& stream_setting, StreamVersion* version);

  /**
   * @brief Set stream linkage version.
   * @param[in] (component_config) Source component config.
   * @param[out] (version) Stream version.
   * @return Status object.
   */
  Status SetStreamLinkageVersions(
      const ComponentConfig& component_config, StreamVersion* version);

#ifdef SENSCORD_SERVER
  /**
   * @brief Set destination of stream version.
   * @param[in] (stream_setting) Target stream setting.
   * @param[out] (version) Stream version.
   * @return Status object.
   */
  Status SetStreamDestination(
      const StreamSetting& stream_setting, StreamVersion* version);

  /**
   * @brief Get destination id.
   * @param[in] (instance_name) Target instance name.
   * @param[out] (id) Destination id.
   * @return Status object.
   */
  Status GetDestinationId(const std::string& instance_name, int32_t* id);

  /**
   * @brief Set server versions.
   * @return Status object.
   */
  Status SetServerVersions();

  /**
   * @brief Wait for fetch server versions.
   * @return Status object.
   */
  Status WaitFetchServerVersion();

  /**
   * @brief Get connection information by client instance name.
   * @param[in] (instance_name) Target client instance name.
   * @param[out] (type) Connection type.
   * @param[out] (address) Destination address.
   * @return Status object.
   */
  Status GetConnectionInfoByClientInstance(
      const std::string& instance_name,
      std::string* type, std::string* address);
#endif  // SENSCORD_SERVER

 private:
  const ConfigManager* config_manager_;
  bool is_clientless_;
  SensCordVersion* version_;
  util::Mutex* mutex_;

#ifdef SENSCORD_SERVER
  // stream destination manage map
  std::map<std::string, int32_t> destination_map_;
  util::Mutex* destination_map_mutex_;
  // waiting fetch resource
  struct FetcherValue {
    int32_t destination_id;
    bool is_finished;
    Status status;
  };
  typedef std::map<VersionFetcher*, FetcherValue> Fetchers;
  Fetchers waiting_fetcher_map_;
  util::Mutex* waiting_mutex_;
  senscord::osal::OSCond* waiting_cond_;
#endif  // SENSCORD_SERVER
};

}  // namespace senscord

#else  // SENSCORD_STREAM_VERSION

#include "senscord/status.h"
#include "senscord/noncopyable.h"
#include "core/config_manager.h"
#include "core/version.h"

namespace senscord {

class VersionManager : private util::Noncopyable {
 public:
  explicit VersionManager(const ConfigManager* /*config_manager*/) {}

  /**
   * @brief Get senscord version.
   * @param[out] (version) SensCord version information.
   * @return Status object.
   */
  Status GetVersion(SensCordVersion* version, bool /*is_clientless*/) {
    if (version != NULL) {
      version->senscord_version.name = CoreVersion::Name();
      version->senscord_version.major = CoreVersion::Major();
      version->senscord_version.minor = CoreVersion::Minor();
      version->senscord_version.patch = CoreVersion::Patch();
      version->senscord_version.description = CoreVersion::Description();
      version->project_version = Version();
    }
    return Status::OK();
  }
};

}  // namespace senscord

#endif  // SENSCORD_STREAM_VERSION
#endif  // LIB_CORE_CORE_VERSION_MANAGER_H_
