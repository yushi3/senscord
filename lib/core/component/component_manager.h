/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_COMPONENT_COMPONENT_MANAGER_H_
#define LIB_CORE_COMPONENT_COMPONENT_MANAGER_H_

#include <map>
#include <vector>
#include <string>

#include "senscord/noncopyable.h"
#include "loader/component_dynamic_loader.h"
#include "component/component_adapter.h"
#include "component/component_config_manager.h"
#include "core/core_behavior.h"
#include "util/mutex.h"

namespace senscord {

/**
 * @brief Component manager class (singleton)
 */
class ComponentManager : private util::Noncopyable {
 public:
  /**
   * @brief Get singleton instance.
   * @return Instance of Component manager.
   */
  static ComponentManager* GetInstance();

  /**
   * @brief Get the component adapter.
   * @param[in] (instance_name) Component instance name to get.
   * @param[in] (core_behavior) Core behavior.
   * @param[out] (adapter) Location of created component adapter.
   * @return Status object.
   */
  Status OpenComponent(
      const std::string& instance_name,
      const CoreBehavior* core_behavior,
      ComponentAdapter** adapter);

  /**
   * @brief Release the component adapter.
   * @param (adapter) Adapter form OpenComponent().
   * @return Status object.
   */
  Status CloseComponent(ComponentAdapter* adapter);

  /**
   * @brief Get created adapter.
   * @param (component_name) Component name to search.
   * @return Created adapter instance.
   */
  ComponentAdapter* GetAdapter(const std::string& component_name);

#ifdef SENSCORD_STREAM_VERSION
  /**
   * @brief Read Component config
   * @param (config_manager) Config manager.
   * @param (instance_name_list) Instance name list to be read.
   * @return Status object.
   */
  Status ReadComponentConfig(
      ConfigManager* config_manager,
      const std::vector<std::string>& instance_name_list);

  /**
   * @brief Get Component config
   * @param (component_name) Target component name.
   * @param (component_config) Component config
   * @return Status object.
   */
  Status GetComponentConfig(
      const std::string& component_name,
      ComponentConfig** component_config);
#endif  // SENSCORD_STREAM_VERSION

 private:
  /**
   * @brief Constructor.
   */
  ComponentManager();

  /**
   * @brief Destructor.
   */
  ~ComponentManager();

  // loader
  ComponentDynamicLoader* loader_;

  // component list
  typedef std::map<std::string, ComponentAdapter*> ComponentAdapterMap;
  ComponentAdapterMap adapter_list_;

  // list lock object
  util::Mutex* mutex_adapter_list_;

  // component config manager
  ComponentConfigManager* config_manager_;
};

}    // namespace senscord
#endif  // LIB_CORE_COMPONENT_COMPONENT_MANAGER_H_
