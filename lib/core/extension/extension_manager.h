/*
 * SPDX-FileCopyrightText: 2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_EXTENSION_EXTENSION_MANAGER_H_
#define LIB_CORE_EXTENSION_EXTENSION_MANAGER_H_

#include <map>
#include <string>
#include <vector>

#include "senscord/status.h"
#include "senscord/noncopyable.h"
#include "senscord/develop/extension.h"

#include "core/internal_types.h"
#include "util/mutex.h"

namespace senscord {

/**
 * @brief A singleton class that manages extension libraries.
 */
class ExtensionManager : private util::Noncopyable {
 public:
  /**
   * @brief Gets singleton instance.
   * @return Instance of extension manager.
   */
  static ExtensionManager* GetInstance();

  /**
   * @brief Initializes the extension libraries.
   * @param[in] (core_config) Core config.
   * @return Status object.
   */
  Status Init(const CoreConfig& core_config);

  /**
   * @brief Exits the extension libraries.
   * @return Status object.
   */
  Status Exit();

  /**
   * @brief Gets the stream extension libraries.
   * @param[in] (stream_key) Stream key.
   * @return Stream extension libraries. (Empty if does not exist)
   */
  std::vector<const ExtensionLibrary*>
  GetStreamExtensionLibraries(const std::string& stream_key) const;

 private:
  ExtensionManager();
  ~ExtensionManager();

  /**
   * @brief Loads all extension libraries.
   * @param[in] (core_config) Core config.
   */
  void LoadAllLibraries(const CoreConfig& core_config);

  /**
   * @brief Unloads all extension libraries.
   */
  void UnloadAllLibraries();

  /**
   * @brief Initializes the core extension.
   * @param[in] (core_config) Core config.
   * @return Status object.
   */
  Status InitCoreExtension(const CoreConfig& core_config);

  /**
   * @brief Exits the core extension.
   * @param[in] (stop_on_error) If true, stop on error.
   * @return Status object.
   */
  Status ExitCoreExtension(bool stop_on_error);

/**
 * @brief Get arguments for core extension.
 * @param[in] (core_config) Core config.
 * @param[in] (library_name) Target CoreExtension library name.
 * @param[out] (arguments) Arguments for CoreExtension.
 */
void GetCoreExtensionArguments(
    const CoreConfig& core_config,
    const std::string& library_name,
    std::map<std::string, std::string>* arguments);

 private:
  std::vector<ExtensionLibrary*> libraries_;
  std::map<std::string, std::vector<const ExtensionLibrary*> >
      libraries_stream_;
  std::vector<CoreExtension*> core_extensions_;

  mutable util::Mutex mutex_;
  int32_t reference_count_;
};

}  // namespace senscord

#endif  // LIB_CORE_EXTENSION_EXTENSION_MANAGER_H_
