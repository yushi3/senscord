/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <map>
#include <string>
#include <utility>

#include "senscord/develop/extension.h"
#include "senscord/osal.h"

#include "util/senscord_utils.h"
#include "logger/logger.h"

namespace {

const char* kRegisterFunctions[] = {
    "RegisterCoreExtension",
    "RegisterStreamExtension",
};

/**
 * @brief Gets the path of the directory containing the specified library.
 * @param[in] (library_name) the name of the library.
 * @return the path of the library.
 */
std::string GetLibraryPath(const std::string& library_name) {
  std::string file_name;
  senscord::osal::OSGetDynamicLibraryFileName(library_name, &file_name);
  std::string library_path;
  senscord::util::SearchFileFromEnv(file_name, &library_path);
  return library_path;
}

}  // namespace

namespace senscord {

struct ExtensionLibrary::Impl {
  osal::OSDlHandle* handle;
  std::string library_name;
  std::map<std::string, Factory*> factories;
};

/**
 * @brief Loads an extension library.
 * @param[in] (library_name) the name of the library to load.
 * @return an instance of extension library.
 */
ExtensionLibrary* ExtensionLibrary::Load(const std::string& library_name) {
  ExtensionLibrary* library = NULL;

  std::string library_path = GetLibraryPath(library_name);
  if (!library_path.empty()) {
    std::string error_msg;
    // load library
    osal::OSDlHandle* handle = NULL;
    int32_t ret = osal::OSDlLoad(library_path.c_str(), &handle, &error_msg);
    if (!osal::error::IsError(ret)) {
      library = new ExtensionLibrary();
      library->pimpl_->handle = handle;
      library->pimpl_->library_name = library_name;

      const uint32_t count =
          sizeof(kRegisterFunctions) / sizeof(kRegisterFunctions[0]);
      for (uint32_t i = 0; i < count; ++i) {
        void* func_ptr = NULL;
        ret = osal::OSDlGetFuncPtr(handle, kRegisterFunctions[i], &func_ptr);
        if (!osal::error::IsError(ret)) {
          typedef void (*RegisterFunction)(void*);
          RegisterFunction func = reinterpret_cast<RegisterFunction>(func_ptr);
          func(library);
        }
      }

      if (library->pimpl_->factories.empty()) {
        delete library;
        library = NULL;
        error_msg = "Extension class not found";
      }
    }

    if (!error_msg.empty()) {
      SENSCORD_LOG_WARNING(
          "Failed to load the extension library. '%s', %s",
          library_name.c_str(), error_msg.c_str());
    }
  } else {
    SENSCORD_LOG_WARNING(
        "Extension library not found. name='%s'",
        library_name.c_str());
  }

  return library;
}

/**
 * @brief Constructor.
 */
ExtensionLibrary::ExtensionLibrary() : pimpl_(new Impl()) {
}

/**
 * @brief Destructor.
 *
 * Unloads an extension library.
 */
ExtensionLibrary::~ExtensionLibrary() {
  for (std::map<std::string, Factory*>::iterator
      itr = pimpl_->factories.begin(), end = pimpl_->factories.end();
      itr != end; ++itr) {
    delete itr->second;
  }
  osal::OSDlFree(pimpl_->handle);
  delete pimpl_;
}

/**
 * @brief Returns the name of the library.
 */
std::string ExtensionLibrary::GetLibraryName() const {
  return pimpl_->library_name;
}

/**
 * @brief Gets the factory.
 * @param[in] (class_name) the name of the class.
 * @return the factory object. (NULL if does not exist)
 */
const ExtensionLibrary::Factory* ExtensionLibrary::GetFactory(
    const std::string& class_name) const {
  Factory* factory = NULL;
  std::map<std::string, Factory*>::const_iterator itr =
      pimpl_->factories.find(class_name);
  if (itr != pimpl_->factories.end()) {
    factory = itr->second;
  }
  return factory;
}

/**
 * @brief Registers the factory.
 * @param[in] (class_name) the name of the class to register.
 * @param[in] (factory) the factory to register.
 */
void ExtensionLibrary::RegisterFactory(
    const std::string& class_name, Factory* factory) {
  typedef Factory* FactoryPtr;
  FactoryPtr& factory_ptr = pimpl_->factories[class_name];
  delete factory_ptr;
  factory_ptr = factory;
}

}  // namespace senscord
