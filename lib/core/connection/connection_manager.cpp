/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string>
#include <map>
#include "senscord/connection_manager.h"
#include "core/internal_types.h"
#include "util/autolock.h"
#include "util/mutex.h"
#include "util/senscord_utils.h"
#include "util/singleton.h"
#include "logger/logger.h"
#include "connection/connection_config_manager.h"
#include "connection/connection_dynamic_loader.h"

namespace senscord {

struct ConnectionManager::Impl {
  util::Mutex mutex;
  bool initialized;
  ConnectionConfigManager config_manager;
  ConnectionDynamicLoader loader;
  std::map<Connection*, std::string> connection_libraries;
};

/**
 * @brief Constructor.
 */
ConnectionManager::ConnectionManager() : pimpl_(new Impl()) {
}

/**
 * @brief Destructor.
 */
ConnectionManager::~ConnectionManager() {
  delete pimpl_;
}

/**
 * @brief Get the manager instance.
 * @return Manager instance.
 */
ConnectionManager* ConnectionManager::GetInstance() {
  // for private constructor / destructor
  struct InnerConnectionManager : public ConnectionManager {
  };
  return util::Singleton<InnerConnectionManager>::GetInstance();
}

/**
 * @brief Initialize and read config file.
 * @return Status object.
 */
Status ConnectionManager::Init() {
  util::AutoLock autolock(&pimpl_->mutex);
  if (pimpl_->initialized) {
    return Status::OK();
  }

  // get connection config path
  std::string path;
  if (util::SearchFileFromEnv(kConnectionConfigFile, &path)) {
    // read configure
    Status status = pimpl_->config_manager.ReadConfig(path);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  } else {
    // if no connection config, through initialize.
    SENSCORD_LOG_DEBUG("%s not found.", kConnectionConfigFile);
  }
  pimpl_->initialized = true;
  return Status::OK();
}

/**
 * @brief Create the new connection instance.
 * @param[in]  (key) Connection key.
 * @param[out] (connection) New connection instance.
 * @return Status object.
 */
Status ConnectionManager::CreateConnection(
    const std::string& key, Connection** connection) {
  if (connection == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  util::AutoLock autolock(&pimpl_->mutex);

  // get the library name
  std::string library_name;
  Status status = pimpl_->config_manager.GetLibraryName(key, &library_name);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // load connection
  Connection* tmp_connection = NULL;
  status = pimpl_->loader.Create(library_name, &tmp_connection);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // save library name
  pimpl_->connection_libraries[tmp_connection] = library_name;

  *connection = tmp_connection;
  return Status::OK();
}

/**
 * @brief Release the connection instance.
 * @param[in] (key) Connection key.
 * @param[in] (connection) connection instance.
 * @return Status object.
 */
Status ConnectionManager::ReleaseConnection(Connection* connection) {
  if (connection == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  util::AutoLock autolock(&pimpl_->mutex);

  // search library
  std::map<Connection*, std::string>::iterator itr =
      pimpl_->connection_libraries.find(connection);
  if (itr == pimpl_->connection_libraries.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "connection does not exist.");
  }

  // release connection
  Status status = pimpl_->loader.Destroy(itr->second, connection);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  pimpl_->connection_libraries.erase(itr);

  return Status::OK();
}

/**
 * @brief Get the connection arguments.
 * @param[in] (key) Connection key.
 * @param[in,out] (arguments) Connection arguments.
 * @return Status object.
 */
Status ConnectionManager::GetArguments(
    const std::string& key,
    std::map<std::string, std::string>* arguments) {
  if (arguments == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "arguments == NULL");
  }
  util::AutoLock autolock(&pimpl_->mutex);
  std::map<std::string, std::string> config_arguments;
  Status status = pimpl_->config_manager.GetArguments(key, &config_arguments);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  // discard duplicate keys.
  arguments->insert(config_arguments.begin(), config_arguments.end());
  return status;
}

}  // namespace senscord
