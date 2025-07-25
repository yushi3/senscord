/*
 * SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core/server_config_manager.h"

#include <stdint.h>

#include <algorithm>
#include <set>

#include "allocator/memory_manager.h"
#include "senscord/develop/common_types.h"
#include "util/autolock.h"

namespace {
const char kArgumentNameConnection[] = "connection";
const char kArgumentNameAddress[] = "address";
// kArgumentNamePortNum is defined by develop/common_types.h
const char kArgumentNameThreading[] = "threading";

const char kArgumentValuePortNumMax[] = "256";
const char kArgumentValueParallel[] = "parallel";

const char kComponentNameComponentClient[] = "component_client";
}  // unnamed namespace

namespace senscord {

/**
 * @brief Constructor.
 */
ServerConfigManager::ServerConfigManager()
    : waiting_fetcher_map_(),
      server_config_list_(),
      request_mutex_(new util::Mutex),
      request_waiting_mutex_(new util::Mutex),
      request_waiting_cond_(NULL) {
  senscord::osal::OSCreateCond(&request_waiting_cond_);
}

/**
 * @brief Destructor.
 */
ServerConfigManager::~ServerConfigManager() {
  {
    util::AutoLock lock(request_waiting_mutex_);
    while (!waiting_fetcher_map_.empty()) {
      ServerConfigFetcher* fetcher = waiting_fetcher_map_.begin()->first;
      waiting_fetcher_map_.erase(waiting_fetcher_map_.begin());
      delete fetcher;
    }
    waiting_fetcher_map_.clear();
  }  // lock(request_waiting_mutex_)

  delete request_mutex_;
  request_mutex_ = NULL;
  delete request_waiting_mutex_;
  request_waiting_mutex_ = NULL;
  senscord::osal::OSDestroyCond(request_waiting_cond_);
  request_waiting_cond_ = NULL;
}

/**
 * @brief Get server config.
 * @param[in/out] (core_config) Core configuration.
 * @param[in] (identification) SensCord identification.
 * @return Status object.
 */
Status ServerConfigManager::GetServerConfig(CoreConfig* core_config,
                                            const std::string& identification) {
  if (core_config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
                                "core_config is null");
  }

  GetServerConfigs(core_config->server_list);

  return UpdateCoreConfig(core_config, identification);
}

/**
 * @brief Notify server config.
 * @param[in] (fetcher) Fetcher subject to notification.
 * @param[in] (config) server config, NULL is cancel.
 * @param[in] (status) Server status.
 */
void ServerConfigManager::NotifyServerConfig(const ServerConfigFetcher* fetcher,
                                             const ServerConfig* config,
                                             const Status& status) {
  if (fetcher == NULL) {
    SENSCORD_LOG_ERROR("invalid parameter");
    return;
  }

  util::AutoLock lock(request_waiting_mutex_);
  for (Fetchers::iterator itr = waiting_fetcher_map_.begin(),
                          end = waiting_fetcher_map_.end();
       itr != end; ++itr) {
    if (itr->first == fetcher) {
      if (config != NULL) {
        ServerConfigInfo server_config_info = {itr->first->GetServerType(),
                                               itr->first->GetServerAddress(),
                                               itr->second.server_setting};
        server_config_list_.push_back(
            std::make_pair(*config, server_config_info));
      }

      itr->second.is_finished = true;
      itr->second.status = status;
      senscord::osal::OSSignalCond(request_waiting_cond_);
      return;
    }
  }
  SENSCORD_LOG_ERROR("fetcher is not found : %p", fetcher);
}

/**
 * @brief Get server configs.
 * @param[in] (server_list) Target servers.
 * @return Status object.
 */
Status ServerConfigManager::GetServerConfigs(
    const std::vector<ServerSetting>& server_list) {
  util::AutoLock lock(request_mutex_);

  Status status;
  {
    util::AutoLock waiting_lock(request_waiting_mutex_);

    server_config_list_.clear();

    if (!waiting_fetcher_map_.empty()) {
      SENSCORD_LOG_WARNING("waiting_fetcher_map_ is not empty");
    }

    for (std::vector<ServerSetting>::const_iterator itr = server_list.begin(),
                                                    end = server_list.end();
         itr != end; ++itr) {
      std::string type;
      std::string address;
      if (!GetServerInfo(*itr, &type, &address).ok()) {
        SENSCORD_LOG_WARNING("exclude invalid element of server_list");
        continue;
      }

      ServerConfigFetcher* fetcher =
          new ServerConfigFetcher(type, address, this);
      status = fetcher->RequestConfig();
      if (!status.ok()) {
        SENSCORD_STATUS_TRACE(status);
        delete fetcher;
        continue;
      }

      FetcherValue value = {false, Status::OK(), *itr};
      waiting_fetcher_map_.insert(std::make_pair(fetcher, value));
    }
  }  // lock(request_waiting_mutex_)

  status = WaitFetchServerConfig();
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Wait for fetch server configs.
 * @return Status object.
 */
Status ServerConfigManager::WaitFetchServerConfig() {
  Status return_status;  // first of error status is a priority
  osal::OSLockMutex(request_waiting_mutex_->GetObject());
  while (true) {
    Fetchers::iterator itr = waiting_fetcher_map_.begin();
    Fetchers::iterator end = waiting_fetcher_map_.end();
    while (itr != end) {
      if (itr->second.is_finished) {
        if ((return_status.ok()) && (!itr->second.status.ok())) {
          return_status = itr->second.status;
        }
        ServerConfigFetcher* fetcher = itr->first;
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
    osal::OSWaitCond(request_waiting_cond_,
                     request_waiting_mutex_->GetObject());
  }
  osal::OSUnlockMutex(request_waiting_mutex_->GetObject());
  return SENSCORD_STATUS_TRACE(return_status);
}

/**
 * @brief Get server information.
 * @param[in] (server_setting) Server setting.
 * @param[out] (type) Connection type.
 * @param[out] (address) Destination address.
 * @return Status object.
 */
Status ServerConfigManager::GetServerInfo(const ServerSetting& server_setting,
                                          std::string* type,
                                          std::string* address) {
  typedef std::map<std::string, std::string>::const_iterator Iter;

  Iter type_itr = server_setting.arguments.find(kArgumentNameConnection);
  Iter addr_itr = server_setting.arguments.find(kArgumentNameAddress);
  Iter end = server_setting.arguments.end();

  if ((type_itr == end) || (addr_itr == end)) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseInvalidArgument,
                                "invalid parameter");
  }

  *type = type_itr->second;
  *address = addr_itr->second;

  return Status::OK();
}

/**
 * @brief Update core config.
 * @param[in/out] (core_config) Core configuration.
 * @param[in] (identification) SensCord identification.
 * @return Status object.
 */
Status ServerConfigManager::UpdateCoreConfig(
    CoreConfig* core_config, const std::string& identification) {
  std::vector<std::string> stream_key_list;
  std::map<std::string, std::set<int32_t> > port_id_map;
  for (std::vector<StreamSetting>::const_iterator
           itr = core_config->stream_list.begin(),
           end = core_config->stream_list.end();
       itr != end; ++itr) {
    stream_key_list.push_back(itr->stream_key);

    std::set<int32_t>& port_id_list = port_id_map[itr->address.instance_name];
    port_id_list.insert(itr->address.port_id);
  }

  util::AutoLock lock(request_waiting_mutex_);
  for (std::vector<std::pair<ServerConfig, ServerConfigInfo> >::const_iterator
           itr = server_config_list_.begin(),
           end = server_config_list_.end();
       itr != end; ++itr) {
    const ServerConfig& server_config = itr->first;
    const ServerConfigInfo& server_config_info = itr->second;

    const std::string client_instance_name =
        identification.empty()
            ? server_config_info.address
            : identification + kSensCordIdentificationDelimiter +
                  server_config_info.address;

    std::vector<ComponentInstanceConfig>::iterator instance_list_itr =
        core_config->instance_list.begin();
    std::vector<ComponentInstanceConfig>::iterator instance_list_end =
        core_config->instance_list.end();
    for (; instance_list_itr != instance_list_end; ++instance_list_itr) {
      if (instance_list_itr->instance_name == client_instance_name) {
        break;
      }
    }
    if (instance_list_itr == instance_list_end) {
      ComponentInstanceConfig instance_config = {};
      instance_config.instance_name = client_instance_name;
      instance_config.component_name = kComponentNameComponentClient;
      instance_config.arguments =
          server_config_info.server_setting.arguments;

      std::map<std::string, std::string>::const_iterator itr_arg_end =
          instance_config.arguments.end();

      std::map<std::string, std::string>::const_iterator itr_arg_port_num =
          instance_config.arguments.find(kArgumentNamePortNum);
      if (itr_arg_port_num == itr_arg_end) {
        instance_config.arguments[kArgumentNamePortNum] =
            kArgumentValuePortNumMax;
      }

      std::map<std::string, std::string>::const_iterator itr_arg_threading =
          instance_config.arguments.find(kArgumentNameThreading);
      if (itr_arg_threading == itr_arg_end) {
        instance_config.arguments[kArgumentNameThreading] =
            kArgumentValueParallel;
      }

      core_config->instance_list.push_back(instance_config);
      instance_list_itr = --(core_config->instance_list.end());
    }

    for (std::vector<ServerStreamSetting>::const_iterator
             server_stream_setting_itr = server_config.stream_list.begin(),
             server_stream_setting_end = server_config.stream_list.end();
         server_stream_setting_itr != server_stream_setting_end;
         ++server_stream_setting_itr) {
      const ServerStreamSetting& server_stream_setting =
          *server_stream_setting_itr;

      AddAllocatorKey(&(instance_list_itr->allocator_key_list),
                      server_stream_setting.radical_address.instance_name,
                      server_config.instance_list);
      AddAllocatorKey(&(instance_list_itr->allocator_key_list),
                      server_stream_setting.address.instance_name,
                      server_config.instance_list);

      if (std::find(stream_key_list.begin(), stream_key_list.end(),
                    server_stream_setting.stream_key) !=
          stream_key_list.end()) {
        continue;
      }
      StreamSetting stream_setting;
      stream_setting.stream_key = server_stream_setting.stream_key;
      stream_setting.identification = server_stream_setting.identification;

      stream_setting.address.instance_name = client_instance_name;
      stream_setting.address.port_type = kPortTypeClient;
      if (port_id_map[client_instance_name].empty()) {
        stream_setting.address.port_id = 0;
        port_id_map[client_instance_name].insert(0);
      } else {
        int32_t port_id = *(--(port_id_map[client_instance_name].end())) + 1;
        stream_setting.address.port_id = port_id;
        port_id_map[client_instance_name].insert(port_id);
      }

      stream_setting.radical_address.instance_name =
          server_stream_setting.radical_address.instance_name;
      stream_setting.radical_address.port_type =
          server_stream_setting.radical_address.port_type;
      stream_setting.radical_address.port_id =
          server_stream_setting.radical_address.port_id;

      stream_setting.frame_buffering = server_stream_setting.frame_buffering;

      stream_setting.client_instance_name = client_instance_name;
      stream_setting.client_specified = server_stream_setting.client_specified;

      core_config->stream_list.push_back(stream_setting);
      stream_key_list.push_back(stream_setting.stream_key);
    }
  }

  return Status::OK();
}

/**
 * @brief Add allocator keys.
 * @param[out] (allocator_key_list) Allocator key list of client instance.
 * @param[in] (instance_name) Radical instance name of stream.
 * @param[in] (instance_list) Instance list of server.
 * @return Status object.
 */
Status ServerConfigManager::AddAllocatorKey(
    std::map<std::string, std::string>* allocator_key_list,
    const std::string& instance_name,
    const std::vector<ServerComponentInstanceConfig>& instance_list) {
  for (std::vector<ServerComponentInstanceConfig>::const_iterator
           itr = instance_list.begin(),
           end = instance_list.end();
       itr != end; ++itr) {
    if (itr->instance_name == instance_name) {
      if (itr->allocator_key_list.empty()) {
        (*allocator_key_list)[kAllocatorNameDefault] = kAllocatorDefaultKey;
      } else {
        for (std::map<std::string, std::string>::const_iterator
                 allocator_key_list_itr = itr->allocator_key_list.begin(),
                 allocator_key_list_end = itr->allocator_key_list.end();
             allocator_key_list_itr != allocator_key_list_end;
             ++allocator_key_list_itr) {
          // name is as allocator key, because names are not used by client.
          (*allocator_key_list)[allocator_key_list_itr->second] =
              allocator_key_list_itr->second;
        }
      }
      return Status::OK();
    }
  }
  return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
                              "unknown instance name: %s",
                              instance_name.c_str());
}

}  // namespace senscord
