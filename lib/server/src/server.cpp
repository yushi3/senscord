/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/server/server.h"
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
 * @param[in] (listener) The primary connection interface instance.
 * @param[in] (config_path) The path of server configuration file.
 * @param[in] (listener2) The secondary connection interface instance.
 * @return Status object.
 */
Status Server::Open(Connection* listener, const std::string& config_path,
                    Connection* listener2) {
  if (!listeners_.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseInvalidOperation, "already opened");
  }
  if (listener == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseInvalidArgument, "listener is null");
  }

  Status status;

  // read config
  if (!config_path.empty()) {
    status = config_manager_->ReadConfig(config_path);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }

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

  status = client_manager_->Start();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  {
    // create primary listener.
    std::string bind_address;
    status = config_manager_->GetBindAddress(&bind_address);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    ClientListener* primary_listener = new ClientListener(
        client_manager_, listener, bind_address, core_, *config_manager_);
    status = primary_listener->Start();
    if (!status.ok()) {
      delete primary_listener;
      return SENSCORD_STATUS_TRACE(status);
    }
    listeners_.push_back(primary_listener);
  }

  {
    // create secondary listener.
    std::string bind_address;
    status = config_manager_->GetSecondaryBindAddress(&bind_address);
    if (status.ok()) {
      if (listener2 == NULL) {
        Close();
        return SENSCORD_STATUS_FAIL(kStatusBlockServer,
            Status::kCauseInvalidArgument, "listener2 is null");
      }

      ClientListenerBase* secondary_listener = new SecondaryClientListener(
          client_manager_, listener2, bind_address);
      status = secondary_listener->Start();
      if (!status.ok()) {
        delete secondary_listener;
        Close();
        return SENSCORD_STATUS_TRACE(status);
      }
      listeners_.push_back(secondary_listener);
    }
  }

  SENSCORD_SERVER_LOG_INFO("[server] Start server");
  return Status::OK();
}

/**
 * @brief Open the host server.
 * @param[in] (listener) The primary connection interface instance.
 * @param[in] (config) The configuration for the server functions.
 * @param[in] (listener2) The secondary connection interface instance.
 * @return Status object.
 */
Status Server::Open(Connection* listener, const ServerConfig& config,
                    Connection* listener2) {
  if (!listeners_.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseInvalidOperation, "already opened");
  }
  if (listener == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseInvalidArgument, "listener is null");
  }

  Status status = config_manager_->SetConfig(config);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Open the server without reading config.
  status = Open(listener, "", listener2);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Close the host server.
 * @return Status object.
 */
Status Server::Close() {
  client_manager_->Stop();

  // stop and delete listeners
  for (std::vector<ClientListenerBase*>::iterator itr = listeners_.begin(),
      itr_end = listeners_.end(); itr != itr_end; ++itr) {
    ClientListenerBase* listener = *itr;
    listener->Stop();
    delete listener;
  }
  listeners_.clear();

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
Server::Server() : client_manager_(), core_(), config_manager_() {
  client_manager_ = new ClientAdapterManager;
  config_manager_ = new ConfigManager;
}

/**
 * @brief Destructor.
 */
Server::~Server() {
  Close();

  delete config_manager_;
  delete client_manager_;
}

}   // namespace server
}   // namespace senscord
