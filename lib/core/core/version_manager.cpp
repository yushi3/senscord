/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core/version_manager.h"

#include <stdint.h>
#include <vector>
#include <string>
#include <utility>      // std::make_pair

#include "core/version.h"
#include "core/internal_types.h"
#include "component/component_manager.h"
#include "senscord/connection_manager.h"
#ifdef SENSCORD_SERVER
#include "core/version_fetcher.h"
#include "senscord/develop/client_instance_utils.h"
#endif  // SENSCORD_SERVER

namespace senscord {

/**
 * @brief Constructor.
 * @param[in] (config_manager) Config manager.
 */
VersionManager::VersionManager(const ConfigManager* config_manager) :
    config_manager_(config_manager), is_clientless_(), version_() {
  mutex_ = new util::Mutex();
#ifdef SENSCORD_SERVER
  destination_map_mutex_ = new util::Mutex();
  waiting_mutex_ = new util::Mutex();
  senscord::osal::OSCreateCond(&waiting_cond_);
#endif  // SENSCORD_SERVER
}

/**
 * @brief Destructor.
 */
VersionManager::~VersionManager() {
#ifdef SENSCORD_SERVER
  senscord::osal::OSDestroyCond(waiting_cond_);
  waiting_cond_ = NULL;
  {
    util::AutoLock lock(waiting_mutex_);
    while (!waiting_fetcher_map_.empty()) {
      VersionFetcher* fetcher = waiting_fetcher_map_.begin()->first;
      waiting_fetcher_map_.erase(waiting_fetcher_map_.begin());
      delete fetcher;
    }
    waiting_fetcher_map_.clear();
  }
  delete waiting_mutex_;
  waiting_mutex_ = NULL;
  delete destination_map_mutex_;
  destination_map_mutex_ = NULL;
#endif  // SENSCORD_SERVER
  delete mutex_;
  mutex_ = NULL;
}

/**
 * @brief Get senscord version.
 * @param[out] (version) SensCord version information.
 * @param[in] (is_clientless) Is this clientless process.
 * @return Status object.
 */
Status VersionManager::GetVersion(
    SensCordVersion* version, bool is_clientless) {
  if (version == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  util::AutoLock lock(mutex_);
  is_clientless_ = is_clientless;
  version_ = version;
  Status return_status;
  // continue processing even if an error occurs on the way
  Status status = SetSensCordVersion(&version_->senscord_version);
  SENSCORD_STATUS_TRACE(status);
  if (!status.ok()) {
    SENSCORD_LOG_ERROR("%s", status.ToString().c_str());
    return_status = status;
  }
  status = SetProjectVersion(&version_->project_version);
  SENSCORD_STATUS_TRACE(status);
  if (!status.ok()) {
    SENSCORD_LOG_ERROR("%s", status.ToString().c_str());
    if (return_status.ok()) {
      return_status = status;
    }
  }
  status = SetStreamVersions(&version_->stream_versions);
  SENSCORD_STATUS_TRACE(status);
  if (!status.ok()) {
    SENSCORD_LOG_ERROR("%s", status.ToString().c_str());
    if (return_status.ok()) {
      return_status = status;
    }
  }
#ifdef SENSCORD_SERVER
  if (!is_clientless) {
    status = SetServerVersions();
    SENSCORD_STATUS_TRACE(status);
    if (!status.ok()) {
      SENSCORD_LOG_ERROR("%s", status.ToString().c_str());
      if (return_status.ok()) {
        return_status = status;
      }
    }
  }
#endif  // SENSCORD_SERVER
  version_ = NULL;
  return return_status;
}

/**
 * @brief Set senscord core version.
 * @return Status object.
 */
Status VersionManager::SetSensCordVersion(Version* version) {
  if (version == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  version->name = CoreVersion::Name();
  version->major = CoreVersion::Major();
  version->minor = CoreVersion::Minor();
  version->patch = CoreVersion::Patch();
  version->description = CoreVersion::Description();
  return Status::OK();
}

/**
 * @brief Set senscord project version.
 * @return Status object.
 */
Status VersionManager::SetProjectVersion(Version* version) {
  if (version == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  const CoreConfig* config = config_manager_->GetConfig();
  *version = config->project_version;
  return Status::OK();
}

/**
 * @brief Set senscord stream versions.
 * @return Status object.
 */
Status VersionManager::SetStreamVersions(
    std::map<std::string, StreamVersion>* versions) {
  if (versions == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  const CoreConfig* config = config_manager_->GetConfig();

  Status return_status;
  typedef std::vector<StreamSetting>::const_iterator stream_list_itr;
  stream_list_itr itr = config->stream_list.begin();
  stream_list_itr end = config->stream_list.end();
  for (; itr != end; ++itr) {
    // if error case, not insert with return success.
    StreamVersion version;
    Status status = SetStreamVersion(*itr, &version);
    if (status.ok()) {
      versions->insert(std::make_pair(itr->stream_key, version));
    } else {
      SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
    }
  }
  return Status::OK();
}

/**
 * @brief Set stream version specified by stream key.
 * @param[in] (stream_setting) Target stream setting.
 * @param[out] (version) Stream version.
 * @return Status object.
 */
Status VersionManager::SetStreamVersion(
    const StreamSetting& stream_setting, StreamVersion* version) {
  if (version == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  // get relation component config
  const std::string& instance_name = is_clientless_ ?
      stream_setting.radical_address.instance_name :
      stream_setting.address.instance_name;
  const ComponentInstanceConfig* instance_config =
      config_manager_->GetComponentConfigByInstanceName(instance_name);
  if (instance_config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound,
        "instance config not found : instance_name=%s",
        instance_name.c_str());
  }
  // get component config
  ComponentManager* component_manager = ComponentManager::GetInstance();
  ComponentConfig* component_config = NULL;
  Status status = component_manager->GetComponentConfig(
      instance_config->component_name, &component_config);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  version->stream_version.name = component_config->name;
  version->stream_version.major = component_config->major_version;
  version->stream_version.minor = component_config->minor_version;
  version->stream_version.patch = component_config->patch_version;
  version->stream_version.description = component_config->description;
  // set linkage version
  status = SetStreamLinkageVersions(*component_config, version);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
#ifdef SENSCORD_SERVER
  status = SetStreamDestination(stream_setting, version);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
#else
  version->destination_id = kDestinationStreamNone;
#endif  // SENSCORD_SERVER
  return Status::OK();
}

/**
 * @brief Set stream linkage version.
 * @param[in] (component_config) Source component config.
 * @param[out] (version) Stream version.
 * @return Status object.
 */
Status VersionManager::SetStreamLinkageVersions(
    const ComponentConfig& component_config, StreamVersion* version) {
  if (version == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  std::vector<Version>::const_iterator itr =
      component_config.linkage_versions.begin();
  std::vector<Version>::const_iterator end =
      component_config.linkage_versions.end();
  for (; itr != end; ++itr) {
    Version library;
    library.name = itr->name;
    library.major = itr->major;
    library.minor = itr->minor;
    library.patch = itr->patch;
    library.description = itr->description;
    version->linkage_versions.push_back(library);
  }
  return Status::OK();
}

#ifdef SENSCORD_SERVER
/**
 * @brief Set destination of stream version.
 * @param[in] (stream_setting) Target stream setting.
 * @param[out] (version) Stream version.
 * @return Status object.
 */
Status VersionManager::SetStreamDestination(
    const StreamSetting& stream_setting, StreamVersion* version) {
  if (version == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  // check if you are using a client
  bool isUseClient = false;
  if (stream_setting.address.instance_name ==
      stream_setting.client_instance_name) {
    isUseClient = true;
  }
  Status status;
  if ((!is_clientless_) && isUseClient) {
    status = GetDestinationId(
        stream_setting.client_instance_name, &version->destination_id);
  } else {
    version->destination_id = kDestinationStreamNone;
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get destination id.
 * @param[in] (instance_name) Target instance name.
 * @param[out] (id) destination id.
 * @return Status object.
 */
Status VersionManager::GetDestinationId(
    const std::string& instance_name, int32_t* id) {
  if (id == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  util::AutoLock lock(destination_map_mutex_);
  std::map<std::string, int32_t>::iterator registered_itr =
      destination_map_.find(instance_name);
  if (registered_itr != destination_map_.end()) {
    *id = registered_itr->second;
    return Status::OK();  // found
  }
  // not found, newly registered in map
  int32_t destination_id = -1;
  std::map<std::string, int32_t>::iterator itr = destination_map_.begin();
  std::map<std::string, int32_t>::iterator end = destination_map_.end();
  for (; itr != end; ++itr) {
    if (destination_id < itr->second) {
      destination_id = itr->second;
    }
  }
  destination_map_.insert(std::make_pair(instance_name, ++destination_id));
  *id = destination_id;
  return Status::OK();
}

/**
 * @brief Set server versions.
 * @return Status object.
 */
Status VersionManager::SetServerVersions() {
  Status return_status;   // first of error status is a priority
  Status status;
  std::map<std::string, int32_t>::iterator itr = destination_map_.begin();
  std::map<std::string, int32_t>::iterator end = destination_map_.end();
  for (; itr != end; ++itr) {
    std::string type;
    std::string address;
    status = GetConnectionInfoByClientInstance(itr->first, &type, &address);
    if ((return_status.ok()) && (!status.ok())) {
      SENSCORD_STATUS_TRACE(status);
      return_status = status;
    }
    if (!status.ok()) {
      continue;   // next server
    }
    util::AutoLock lock(waiting_mutex_);
    VersionFetcher* fetcher = new VersionFetcher(type, address, this);
    status = fetcher->RequestVersion();
    if ((return_status.ok()) && (!status.ok())) {
      SENSCORD_STATUS_TRACE(status);
      return_status = status;
    }
    if (status.ok()) {
      FetcherValue value = {};
      value.destination_id = itr->second;
      waiting_fetcher_map_.insert(std::make_pair(fetcher, value));
    } else {
      delete fetcher;
    }
  }
  // wait response
  status = WaitFetchServerVersion();
  if ((return_status.ok()) && (!status.ok())) {
    SENSCORD_STATUS_TRACE(status);
    return_status = status;
  }
  return SENSCORD_STATUS_TRACE(return_status);
}

/**
 * @brief Notify server version.
 * @param[in] (fetcher) Fetcher subject to notification.
 * @param[in] (version) server versions, NULL is cancel.
 * @param[in] (status) Server status.
 */
void VersionManager::NotifyServerVersion(
    const VersionFetcher* fetcher,
    const SensCordVersion* version, const Status& status) {
  if (fetcher == NULL) {
    SENSCORD_LOG_ERROR("invalid parameter");
    return;
  }
  util::AutoLock lock(waiting_mutex_);
  Fetchers::iterator itr = waiting_fetcher_map_.begin();
  Fetchers::iterator end = waiting_fetcher_map_.end();
  for (; itr != end; ++itr) {
    if (itr->first == fetcher) {
      if (version != NULL) {
        version_->server_versions.insert(
            std::make_pair(itr->second.destination_id, *version));
      }
      itr->second.is_finished = true;
      itr->second.status = status;
      senscord::osal::OSSignalCond(waiting_cond_);
      return;
    }
  }
  SENSCORD_LOG_ERROR("fetcher is not found : %p", fetcher);
}

/**
 * @brief Wait for fetch server versions.
 * @return Status object.
 */
Status VersionManager::WaitFetchServerVersion() {
  Status return_status;   // first of error status is a priority
  osal::OSLockMutex(waiting_mutex_->GetObject());
  while (true) {
    Fetchers::iterator itr = waiting_fetcher_map_.begin();
    Fetchers::iterator end = waiting_fetcher_map_.end();
    while (itr != end) {
      if (itr->second.is_finished) {
        if ((return_status.ok()) && (!itr->second.status.ok())) {
          return_status = itr->second.status;
        }
        VersionFetcher* fetcher = itr->first;
        fetcher->WaitPostProcess();
        waiting_fetcher_map_.erase(itr++);
        delete fetcher;
      } else {
        ++itr;
      }
    }
    if (waiting_fetcher_map_.empty()) {
      break;  // all responses were received
    }
    osal::OSWaitCond(waiting_cond_ , waiting_mutex_->GetObject());
  }
  osal::OSUnlockMutex(waiting_mutex_->GetObject());
  return SENSCORD_STATUS_TRACE(return_status);
}

/**
 * @brief Get connection information by client instance name.
 * @param[in] (instance_name) Target client instance name.
 * @param[out] (type) Connection type.
 * @param[out] (address) Destination address.
 * @return Status object.
 */
Status VersionManager::GetConnectionInfoByClientInstance(
    const std::string& instance_name,
    std::string* type, std::string* address) {
  if ((type == NULL) || (address == NULL)) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  const ComponentInstanceConfig* instance_config =
      config_manager_->GetComponentConfigByInstanceName(instance_name);
  if (instance_config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound,
        "instance config not found : instance_name=%s",
        instance_name.c_str());
  }
  ClientInstanceUtility::GetConnectionType(
      instance_config->arguments, type);
  Status status = ClientInstanceUtility::GetConnectionAddress(
      instance_config->arguments, address, NULL);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_SERVER

}  // namespace senscord
