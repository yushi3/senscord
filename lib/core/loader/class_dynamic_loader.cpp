/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "loader/class_dynamic_loader.h"

#include <vector>
#include <utility>      // std::make_pair

#include "logger/logger.h"
#include "util/autolock.h"
#include "util/senscord_utils.h"

namespace senscord {

/**
 * @brief Constructor.
 */
ClassDynamicLoader::ClassDynamicLoader() {
  mutex_ = new util::Mutex();
}

/**
 * @brief Destructor.
 */
ClassDynamicLoader::~ClassDynamicLoader() {
  {
    util::AutoLock lock(mutex_);

    FactoryIterator it = factory_map_.begin();

    while (it != factory_map_.end()) {
      Unload(it->first);
      delete it->second;
      ++it;
    }

    factory_map_.clear();
  }
  delete mutex_;
}

/**
 * @brief Generate an instance based on the library name of the argument.
 * @param[in] (name) Key name of library.
 * @param[out] (instance) Where to store the created instance.
 * @return Status object.
 */
Status ClassDynamicLoader::Create(const std::string& name, void** instance) {
  util::AutoLock lock(mutex_);

  SENSCORD_LOG_DEBUG("create instance : name=%s", name.c_str());

  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  if (factory_map_.count(name) == 0) {
    Status ret = Load(name);
    SENSCORD_STATUS_TRACE(ret);
    if (!ret.ok()) {
      return ret;
    }
  }

  ClassDynamicFactory* factory = NULL;

  Status ret = GetFactory(name, &factory);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    return ret;
  }

  if (factory == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
       Status::kCauseNotFound, "factory not found : name=%s", name.c_str());
  }

  ret = factory->CreateInstance(instance);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    if (factory->GetInstanceNum() == 0) {
      Unload(name);
      factory_map_.erase(name);
      delete factory;
      return ret;
    }
  }

  return Status::OK();
}

/**
 * @brief Delete the instance passed in the argument.
 * @param[in] (name) Key name of library.
 * @param[in] (instance) Instance to delete.
 * @return Status object.
 */
Status ClassDynamicLoader::Destroy(const std::string& name, void* instance) {
  util::AutoLock lock(mutex_);

  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  ClassDynamicFactory* factory = NULL;

  Status ret = GetFactory(name, &factory);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    return ret;
  }

  if (factory == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "factory not found : name=%s", name.c_str());
  }

  ret = factory->DestroyInstance(instance);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    return ret;
  }

  // When the instance becomes 0, unload the library and delete the factory.
  if (factory->GetInstanceNum() == 0) {
    ret = Unload(name);
    SENSCORD_STATUS_TRACE(ret);
    factory_map_.erase(name);
    delete factory;
    if (!ret.ok()) {
      return ret;
    }
  }

  return Status::OK();
}

/**
 * @brief Register factory in association with component name.
 * @param[in] (name) Key name of library.
 * @param[in] (factory) Factory to register.
 * @return Status object.
 */
Status ClassDynamicLoader::SetFactory(const std::string& name,
                                      ClassDynamicFactory* factory) {
  util::AutoLock lock(mutex_);

  if (factory == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  bool ret = factory_map_.insert(std::make_pair(name, factory)).second;
  if (!ret) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "Factory is already exists : name=%s", name.c_str());
  }

  factory->SetInstanceName(name);

  return Status::OK();
}

/**
 * @brief Get the factory with the component name as the key.
 * @param[in] (name) Key name of library.
 * @param[out] (factory) Factory pointer.
 * @return Status object.
 */
Status ClassDynamicLoader::GetFactory(const std::string& name,
                                      ClassDynamicFactory** factory) {
  util::AutoLock lock(mutex_);

  if (factory == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  std::map<std::string, ClassDynamicFactory*>::const_iterator itr =
      factory_map_.find(name);
  if (itr == factory_map_.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "Factory does not exist : name=%s", name.c_str());
  }

  *factory = itr->second;

  return Status::OK();
}

/**
 * @brief Get the path of the directory containing the specified library.
 * @param[in] (name) Filename of library.
 * @param[out] (lib_path) Path storage location.
 * @return Status object.
 */
Status ClassDynamicLoader::GetLibraryPath(const std::string& name,
                                          std::string* lib_path) {
  if (lib_path == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  std::string lib_name;
  osal::OSGetDynamicLibraryFileName(name, &lib_name);

  if (!util::SearchFileFromEnv(lib_name, lib_path)) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "library is not found : lib_name=%s", lib_name.c_str());
  }

  return Status::OK();
}

/**
 * @brief Unload the loaded library.
 * @param[in] (name) Key name of library.
 * @return Status object.
 */
Status ClassDynamicLoader::Unload(const std::string& name) {
  ClassDynamicFactory* factory = NULL;

  Status ret = GetFactory(name, &factory);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    return ret;
  }

  if (factory == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
       Status::kCauseNotFound, "factory not found : name=%s", name.c_str());
  }

  osal::OSDlHandle* handle = NULL;

  ret = factory->GetHandle(&handle);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    return ret;
  }

  if (handle == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "get handle fail : name=%s", name.c_str());
  }

  std::string error_msg;
  if (osal::OSDlFree(handle, &error_msg) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "failure to release handle : name=%s, error_msg=%s",
        name.c_str(), error_msg.c_str());
  }

  return Status::OK();
}

/**
 * @brief Load library and register factory.
 * @param[in] (file_path) Path to the directory where the library is located.
 * @param[in] (create_function) Name of the function that creates the instance.
 * @param[in] (destroy_function) Name of function to destroy instance.
 * @param[in] (factory) Pointer of factory to register.
 * @return Status object.
 */
Status ClassDynamicLoader::LoadAndRegisterLibrary(
    const std::string& file_path,
    const std::string& create_function,
    const std::string& destroy_function,
    ClassDynamicFactory* factory) {
  SENSCORD_LOG_DEBUG("load library : file_path=%s", file_path.c_str());

  if (factory == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  // Dynamically load the library and obtain the function pointer.
  std::string error_msg;
  osal::OSDlHandle* handle = NULL;

  if ((osal::OSDlLoad(file_path.c_str(), &handle, &error_msg) != 0)) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "failed to load library : file_path=%s, error_msg=%s",
        file_path.c_str(), error_msg.c_str());
  }

  if (handle == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "failed to load library : file_path=%s", file_path.c_str());
  }

  void* create_instance = NULL;

  if ((osal::OSDlGetFuncPtr(
      handle, create_function.c_str(), &create_instance, &error_msg) != 0)
      || (create_instance == NULL)) {
    osal::OSDlFree(handle);
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "failed to get create function : file_path=%s, error_msg=%s",
        file_path.c_str(), error_msg.c_str());
  }

  void* destroy_instance = NULL;

  if ((osal::OSDlGetFuncPtr(
      handle, destroy_function.c_str(), &destroy_instance, &error_msg) != 0)
      || (destroy_instance == NULL)) {
    osal::OSDlFree(handle);
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "failed to get destroy function : file_path=%s, error_msg=%s",
        file_path.c_str(), error_msg.c_str());
  }

  factory->SetHandle(handle, create_instance, destroy_instance);

  return Status::OK();
}

}    // namespace senscord
