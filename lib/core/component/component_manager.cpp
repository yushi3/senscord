/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "component/component_manager.h"

#include <vector>
#include <utility>

#include "util/autolock.h"
#include "util/singleton.h"
#include "core/config_manager.h"
#include "allocator/memory_manager.h"
#include "messenger/messenger_component.h"

namespace senscord {

/**
 * @brief Get singleton instance.
 * @return Instance of Component manager.
 */
ComponentManager* ComponentManager::GetInstance() {
  // for private constructor / destructor
  struct InnerComponentManager : public ComponentManager {
  };
  return util::Singleton<InnerComponentManager>::GetInstance();
}

/**
 * @brief Constructor.
 */
ComponentManager::ComponentManager() {
  loader_ = new ComponentDynamicLoader();
  config_manager_ = new ComponentConfigManager();
  mutex_adapter_list_ = new util::Mutex();
}

/**
 * @brief Destructor.
 */
ComponentManager::~ComponentManager() {
  {
    util::AutoLock lock(mutex_adapter_list_);
    while (!adapter_list_.empty()) {
      ComponentAdapterMap::iterator itr = adapter_list_.begin();
      ComponentAdapter* adapter = itr->second;
      if (adapter) {
        loader_->Destroy(adapter->GetComponentName(), adapter->GetComponent());
        delete adapter;
      }
      adapter_list_.erase(itr);
    }
  }
  delete loader_;
  loader_ = NULL;
  delete config_manager_;
  config_manager_ = NULL;
  delete mutex_adapter_list_;
  mutex_adapter_list_ = NULL;
}

/**
 * @brief Get the component adapter.
 * @param[in] (instance_name) Component instance name to get.
 * @param[in] (core_behavior) Core behavior.
 * @param[out] (adapter) Location of created component adapter.
 * @return Status object.
 */
Status ComponentManager::OpenComponent(
    const std::string& instance_name,
    const CoreBehavior* core_behavior,
    ComponentAdapter** adapter) {
  if (adapter == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  util::AutoLock lock(mutex_adapter_list_);
  std::pair<ComponentAdapterMap::iterator, bool> ret = adapter_list_.insert(
      ComponentAdapterMap::value_type(instance_name, NULL));
  if (!ret.second) {
    // if already created, return it.
    *adapter = ret.first->second;
    (*adapter)->AddReference();
    return Status::OK();
  }

  Status status;
  Component* component = NULL;
  const ComponentInstanceConfig* instance_config = NULL;
  do {
    // get component config
    ConfigManager* config_manager = core_behavior->GetConfigManager();
    instance_config =
        config_manager->GetComponentConfigByInstanceName(instance_name);
    if (instance_config == NULL) {
      status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseNotFound,
          "instance config not found : instance_name=%s",
          instance_name.c_str());
      break;
    }
    if (instance_config->component_name == kComponentNamePublisher) {
      component = new MessengerComponent();
    } else {
      // create new component and component adapter.
      status = loader_->Create(instance_config->component_name, &component);
      if (!status.ok()) {
        SENSCORD_STATUS_TRACE(status);
        break;
      }
    }
    // create adapter
    ComponentAdapter* new_adapter = new ComponentAdapter();
    status = new_adapter->Init(*instance_config, core_behavior, component);
    if (!status.ok()) {
      SENSCORD_STATUS_TRACE(status);
      delete new_adapter;
      break;
    }
    // created successfully
    ret.first->second = new_adapter;
    *adapter = new_adapter;
    (*adapter)->AddReference();
  } while (false);

  if (!status.ok()) {
    adapter_list_.erase(ret.first);
    if (instance_config != NULL && component != NULL) {
      loader_->Destroy(instance_config->component_name, component);
    }
  }

  return status;
}

/**
 * @brief Release the component adapter.
 * @param (adapter) Adapter form OpenComponent().
 * @return Status object.
 */
Status ComponentManager::CloseComponent(ComponentAdapter* adapter) {
  if (adapter == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  util::AutoLock lock(mutex_adapter_list_);
  ComponentAdapterMap::iterator itr = adapter_list_.find(
      adapter->GetComponentInstanceName());
  if (itr == adapter_list_.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation,
        "adapter is not opened or already removed");
  }

  // if noboady opened, delete component and remove from list.
  adapter->ReleaseReference();
  if (adapter->GetReferenceCount() != 0) {
    return Status::OK();
  }

  Status status = adapter->Exit();
  if (!status.ok()) {
    adapter->AddReference();
    return SENSCORD_STATUS_TRACE(status);
  }

  if (adapter->GetComponentName() == kComponentNamePublisher) {
    delete adapter->GetComponent();
  } else {
    loader_->Destroy(adapter->GetComponentName(), adapter->GetComponent());
  }
  adapter_list_.erase(itr);
  delete adapter;
  return status;
}

/**
 * @brief Get created adapter.
 * @param (component_name) Component name to search.
 * @return Created adapter instance.
 */
ComponentAdapter* ComponentManager::GetAdapter(
    const std::string& component_name) {
  ComponentAdapterMap::const_iterator itr = adapter_list_.find(component_name);
  if (itr != adapter_list_.end()) {
    return itr->second;
  }
  return NULL;
}

#ifdef SENSCORD_STREAM_VERSION
/**
 * @brief Read Component config
 * @param (config_manager) Config manager.
 * @param (instance_name_list) Instance name list to be read.
 * @return Status object.
 */
Status ComponentManager::ReadComponentConfig(
    ConfigManager* config_manager,
    const std::vector<std::string>& instance_name_list) {
  if (config_manager == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // read component config
  std::vector<std::string>::const_iterator itr = instance_name_list.begin();
  std::vector<std::string>::const_iterator end = instance_name_list.end();
  for (; itr != end; ++itr) {
    const ComponentInstanceConfig* config =
        config_manager->GetComponentConfigByInstanceName(*itr);
    if (config == NULL) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseNotFound,
          "instance config not found : instance_name=%s",
          itr->c_str());
    }
    Status status = config_manager_->ReadConfig(config->component_name);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }
  return Status::OK();
}

/**
 * @brief Get Component config
 * @param (component_name) Target component name.
 * @param (component_config) Component config
 * @return Status object.
 */
Status ComponentManager::GetComponentConfig(
    const std::string& component_name, ComponentConfig** component_config) {
  if (component_config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  Status status = config_manager_->GetConfig(
      component_name, component_config);
  return SENSCORD_STATUS_TRACE(status);
}

#endif  // SENSCORD_STREAM_VERSION

}    // namespace senscord
