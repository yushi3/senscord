/*
 * SPDX-FileCopyrightText: 2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "extension/extension_manager.h"

#include <map>
#include <set>
#include <string>
#include <vector>

#include "util/autolock.h"
#include "util/singleton.h"

namespace senscord {

/**
 * @brief Get singleton instance.
 * @return Instance of Component manager.
 */
ExtensionManager* ExtensionManager::GetInstance() {
  // for private constructor / destructor
  struct InnerExtensionManager : public ExtensionManager {
  };
  return util::Singleton<InnerExtensionManager>::GetInstance();
}

/**
 * @brief Constructor.
 */
ExtensionManager::ExtensionManager() : reference_count_() {
}

/**
 * @brief Destructor.
 */
ExtensionManager::~ExtensionManager() {
  ExitCoreExtension(false);
  UnloadAllLibraries();
}

/**
 * @brief Initializes the extension libraries.
 * @param[in] (core_config) Core config.
 * @return Status object.
 */
Status ExtensionManager::Init(const CoreConfig& core_config) {
  util::AutoLock lock(&mutex_);
  if (libraries_.empty()) {
    // Load the extension libraries.
    LoadAllLibraries(core_config);
  }
  if (reference_count_ == 0) {
    // Execute `CoreExtension::Init`.
    Status status = InitCoreExtension(core_config);
    if (!status.ok()) {
      ExitCoreExtension(false);
      return SENSCORD_STATUS_TRACE(status);
    }
  }
  ++reference_count_;
  return Status::OK();
}

/**
 * @brief Exits the extension libraries.
 * @return Status object.
 */
Status ExtensionManager::Exit() {
  util::AutoLock lock(&mutex_);
  if (reference_count_ > 0) {
    --reference_count_;
  }
  if (reference_count_ == 0) {
    // Execute `CoreExtension::Exit`.
    Status status = ExitCoreExtension(true);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    // Unload the extension libraries.
    UnloadAllLibraries();
  }
  return Status::OK();
}

/**
 * @brief Gets the stream extension libraries.
 * @param[in] (stream_key) Stream key.
 * @return Stream extension libraries. (Empty if does not exist)
 */
std::vector<const ExtensionLibrary*>
ExtensionManager::GetStreamExtensionLibraries(
    const std::string& stream_key) const {
  util::AutoLock lock(&mutex_);
  std::vector<const ExtensionLibrary*> result;
  std::map<std::string, std::vector<const ExtensionLibrary*> >::const_iterator
      itr = libraries_stream_.find(stream_key);
  if (itr != libraries_stream_.end()) {
    result = itr->second;
  }
  return result;
}

/**
 * @brief Loads all extension libraries.
 * @param[in] (core_config) Core config.
 */
void ExtensionManager::LoadAllLibraries(const CoreConfig& core_config) {
  const std::vector<StreamSetting>& streams = core_config.stream_list;
  std::map<std::string, ExtensionLibrary*> loaded_libs;  // temporary

  for (std::vector<StreamSetting>::const_iterator
      stream = streams.begin(), stream_end = streams.end();
      stream != stream_end; ++stream) {
    const std::vector<ExtensionSetting>& extensions = stream->extensions;
    if (extensions.empty()) {
      continue;
    }

    std::vector<const ExtensionLibrary*>& stream_libs =
        libraries_stream_[stream->stream_key];
    std::set<std::string> stream_libs_name;  // temporary

    for (std::vector<ExtensionSetting>::const_iterator
        extension = extensions.begin(), extension_end = extensions.end();
        extension != extension_end; ++extension) {
      typedef ExtensionLibrary* ExtensionLibraryPtr;
      ExtensionLibraryPtr& library = loaded_libs[extension->library_name];
      if (library == NULL) {
        // load library.
        library = ExtensionLibrary::Load(extension->library_name);
        if (library == NULL) {
          continue;
        }
        libraries_.push_back(library);
      }
      if (stream_libs_name.insert(library->GetLibraryName()).second) {
        stream_libs.push_back(library);
      }
    }
  }
}

/**
 * @brief Unloads all extension libraries.
 */
void ExtensionManager::UnloadAllLibraries() {
  while (!libraries_.empty()) {
    ExtensionLibrary* library = libraries_.back();
    delete library;
    libraries_.pop_back();
  }
  libraries_stream_.clear();
}

/**
 * @brief Initializes the core extension.
 * @param[in] (core_config) Core config.
 * @return Status object.
 */
Status ExtensionManager::InitCoreExtension(const CoreConfig& core_config) {
  Status status;
  for (std::vector<ExtensionLibrary*>::const_iterator
      itr = libraries_.begin(), end = libraries_.end(); itr != end; ++itr) {
    ExtensionLibrary* library = *itr;
    CoreExtension* core_extension =
        library->CreateInstance<CoreExtension>("CoreExtension");
    if (core_extension != NULL) {
      std::map<std::string, std::string> arguments;
      GetCoreExtensionArguments(
          core_config, library->GetLibraryName(), &arguments);
      status = core_extension->Init(arguments);
      if (!status.ok()) {
        delete core_extension;
        SENSCORD_STATUS_TRACE(status);
        break;
      }
      core_extensions_.push_back(core_extension);
    }
  }
  return status;
}

/**
 * @brief Exits the core extension.
 * @param[in] (stop_on_error) If true, stop on error.
 * @return Status object.
 */
Status ExtensionManager::ExitCoreExtension(bool stop_on_error) {
  Status result;
  while (!core_extensions_.empty()) {
    CoreExtension* core_extension = core_extensions_.back();
    Status status = core_extension->Exit();
    if (!status.ok()) {
      result = SENSCORD_STATUS_TRACE(status);
      if (stop_on_error) {
        break;
      }
    }
    delete core_extension;
    core_extensions_.pop_back();
  }
  return result;
}

/**
 * @brief Get arguments for core extension.
 * @param[in] (core_config) Core config.
 * @param[in] (library_name) Target CoreExtension library name.
 * @param[out] (arguments) Arguments for CoreExtension.
 */
void ExtensionManager::GetCoreExtensionArguments(
    const CoreConfig& core_config,
    const std::string& library_name,
    std::map<std::string, std::string>* arguments) {
  const std::vector<StreamSetting>& streams = core_config.stream_list;
  for (std::vector<StreamSetting>::const_iterator
      stream = streams.begin(), stream_end = streams.end();
      stream != stream_end; ++stream) {
    for (std::vector<ExtensionSetting>::const_iterator
        extension = stream->extensions.begin(),
        extension_end = stream->extensions.end();
        extension != extension_end; ++extension) {
      if (library_name == extension->library_name) {
        for (std::map<std::string, std::string>::const_iterator
            arg_itr = extension->arguments.begin(),
            arg_end = extension->arguments.end();
            arg_itr != arg_end; ++arg_itr) {
          (*arguments)[arg_itr->first] = arg_itr->second;
        }
      }
    }
  }
}

}  // namespace senscord
