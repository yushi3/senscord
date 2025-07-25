/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_server.h"
#include <string>
#include <vector>
#include "senscord/osal.h"
#include "senscord/connection.h"
#include "senscord/senscord.h"
#include "server_log.h"
#include "client_listener.h"
#include "client_adapter_manager.h"
#include "secondary_client_listener.h"
#include "internal_types.h"
#include "config_manager.h"
#include "core_clientless.h"

namespace senscord {
namespace server {

/**
 * @brief Open the host server.
 * @param[in] (config_path) The path of server configuration file.
 * @return Status object.
 */
Status MultiServer::Open(const std::string& config_path) {
  Status status;

  // read config
  if (!config_path.empty()) {
    status = config_manager_->ReadConfig(config_path);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }

#ifdef SENSCORD_LOG_ENABLED
  util::LoggerFactory::GetInstance()->CreateLogger(server::kLoggerTagServer);
#endif

  // setup core
  if (core_ == NULL) {
    bool is_enabled_client = false;
    config_manager_->GetClientEnabled(&is_enabled_client);

    if (is_enabled_client) {
      core_ = new Core();
    } else {
      core_ = new CoreClientless();
    }

    status = core_->Init();
    if (!status.ok()) {
      delete core_;
      core_ = NULL;
      return SENSCORD_STATUS_TRACE(status);
    }

    // print version
    SensCordVersion version = {};
    core_->GetVersion(&version);
    SENSCORD_SERVER_LOG_INFO(
        "[server] Core version: %s %" PRIu32 ".%" PRIu32 ".%" PRIu32 " %s",
        version.senscord_version.name.c_str(),
        version.senscord_version.major,
        version.senscord_version.minor,
        version.senscord_version.patch,
        version.senscord_version.description.c_str());
  }

  // print config value
  config_manager_->PrintConfig();
  {
    std::vector<StreamTypeInfo> supported_streams;
    core_->GetStreamList(&supported_streams);
    config_manager_->VerifySupportedStream(supported_streams);
  }

  // create listeners
  std::vector<ListenerSetting> listeners;
  status = config_manager_->GetListenerList(&listeners);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  status = client_manager_->Start();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  for (std::vector<ListenerSetting>::const_iterator
      itr = listeners.begin(), itr_end = listeners.end();
      itr != itr_end; ++itr) {
    // create primary listener.
    ClientListener* primary_listener = new ClientListener(
        client_manager_, itr->connection, itr->address_primary,
        core_, *config_manager_);
    status = primary_listener->Start();
    if (!status.ok()) {
      delete primary_listener;
      Close();
      return SENSCORD_STATUS_TRACE(status);
    }
    listeners_.push_back(primary_listener);

    SENSCORD_SERVER_LOG_INFO("[server] listen [%s] %s (primary)",
        itr->connection.c_str(), itr->address_primary.c_str());

    // create secondary listener.
    if (!itr->address_secondary.empty()) {
      ClientListenerBase* secondary_listener = new SecondaryClientListener(
          client_manager_, itr->connection, itr->address_secondary);
      status = secondary_listener->Start();
      if (!status.ok()) {
        delete secondary_listener;
        Close();
        return SENSCORD_STATUS_TRACE(status);
      }
      listeners_.push_back(secondary_listener);

      SENSCORD_SERVER_LOG_INFO("[server] listen [%s] %s (secondary)",
          itr->connection.c_str(), itr->address_secondary.c_str());
    }
#ifdef SENSCORD_SERVER_SEARCH_SSDP
    if (SsdpModule::IsTcpConnection(itr->connection)) {
      SsdpModule* ssdp_server = new SsdpModule();
      std::string port_primary;
      std::string port_secondly;
      port_primary =
          itr->address_primary.substr(itr->address_primary.find(":"));
      if (!itr->address_secondary.empty()) {
        port_secondly =
            itr->address_secondary.substr(itr->address_secondary.find(":"));
      }
      ssdp_server->ServerInit(port_primary, port_secondly);
      status = ssdp_server->ServerStart();
      if (!status.ok()) {
        delete ssdp_server;
        return SENSCORD_STATUS_TRACE(status);
      }
      ssdp_servers_.push_back(ssdp_server);
    }
#endif  // SENSCORD_SERVER_SEARCH_SSDP
  }

  if (listeners_.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        senscord::Status::kCauseAborted,
        "Listener does not exist.");
  }

  SENSCORD_SERVER_LOG_INFO("[server] Start server");
  return Status::OK();
}

/**
 * @brief Close the host server.
 * @return Status object.
 */
Status MultiServer::Close() {
  client_manager_->Stop();

  // stop and delete listener
  for (std::vector<ClientListenerBase*>::iterator itr = listeners_.begin(),
      itr_end = listeners_.end(); itr != itr_end; ++itr) {
    ClientListenerBase* listener = *itr;
    listener->Stop();
    delete listener;
  }
  listeners_.clear();

#ifdef SENSCORD_SERVER_SEARCH_SSDP
  for (std::vector<SsdpModule*>::iterator itr = ssdp_servers_.begin(),
      itr_end = ssdp_servers_.end(); itr != itr_end; ++itr) {
    SsdpModule* ssdp_server = *itr;
    ssdp_server->ServerStop();
    delete ssdp_server;
  }
  ssdp_servers_.clear();
#endif  // SENSCORD_SERVER_SEARCH_SSDP

  if (core_) {
    core_->Exit();
    delete core_;
    core_ = NULL;
  }
  SENSCORD_SERVER_LOG_INFO("[server] Stop server");
  return Status::OK();
}

/**
 * @brief Constructor.
 */
MultiServer::MultiServer()
    : client_manager_(), core_(), config_manager_() {
  client_manager_ = new ClientAdapterManager;
  config_manager_ = new ConfigManager;
}

/**
 * @brief Destructor.
 */
MultiServer::~MultiServer() {
  Close();

  delete config_manager_;
  delete client_manager_;
}

}   // namespace server
}   // namespace senscord
