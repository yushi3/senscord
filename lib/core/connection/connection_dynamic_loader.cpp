/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "connection/connection_dynamic_loader.h"

#include <string>
#include "connection/connection_dynamic_factory.h"

namespace senscord {

// Name of the function to be obtained from the library.
static const char kCreateInstance[] = "CreateConnection";
static const char kDestroyInstance[] = "DestroyConnection";

/**
 * @brief Constructor.
 */
ConnectionDynamicLoader::ConnectionDynamicLoader() {}

/**
 * @brief Destructor.
 */
ConnectionDynamicLoader::~ConnectionDynamicLoader() {}

/**
 * @brief Generate an instance based on the connection name of the argument.
 * @param[in]  (name) Name of connection library.
 * @param[out] (connection) Where to store the created Connection.
 * @return Status object.
 */
Status ConnectionDynamicLoader::Create(
    const std::string& name, Connection** connection) {
  Status ret = ClassDynamicLoader::Create(
      name, reinterpret_cast<void**>(connection));
  return SENSCORD_STATUS_TRACE(ret);
}

/**
 * @brief Delete the connection passed in the argument.
 * @param[in] (name) Name of connection library.
 * @param[in] (connection) Connection to delete.
 * @return Status object.
 */
Status ConnectionDynamicLoader::Destroy(
    const std::string& name, Connection* connection) {
  Status ret = ClassDynamicLoader::Destroy(name, connection);
  return SENSCORD_STATUS_TRACE(ret);
}

/**
 * @brief A function that loads a library based on the argument name.
 * @param[in] (name) Key name of library.
 * @return Status object.
 */
Status ConnectionDynamicLoader::Load(const std::string& name) {
  std::string file_path;
  Status ret = GetLibraryPath(name, &file_path);
  if (!ret.ok()) {
    return ret;
  }

  // Register factory as loader.
  ConnectionDynamicFactory* factory = new ConnectionDynamicFactory();
  ret = LoadAndRegisterLibrary(file_path, kCreateInstance, kDestroyInstance,
      factory);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    delete factory;
    return ret;
  }

  SetFactory(name, factory);
  return Status::OK();
}

}   // namespace senscord
