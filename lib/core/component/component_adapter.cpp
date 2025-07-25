/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "component/component_adapter.h"
#include <inttypes.h>
#include <vector>
#include <utility>    // for make_pair
#include "logger/logger.h"
#include "senscord/develop/common_types.h"
#include "senscord/develop/property_accessor.h"
#include "core/config_manager.h"
#include "util/autolock.h"
#include "allocator/memory_manager.h"
#include "component/core_component.h"

namespace senscord {

/**
 * @brief Constructor
 */
ComponentAdapter::ComponentAdapter()
    : core_(NULL), component_(NULL), refcount_(0) {
}

 /**
  * @brief Destructor
  */
ComponentAdapter::~ComponentAdapter() {
  DestroyAllPort();
  component_ = NULL;
  delete core_;
  core_ = NULL;
}

/**
 * @brief Initialize.
 * @param[in] (component_config) Component configuration.
 * @param[in] (core_behavior) Core behavior.
 * @param[in] (component) Handled component address.
 * @return Status object.
 */
Status ComponentAdapter::Init(
    const ComponentInstanceConfig& component_config,
    const CoreBehavior* core_behavior,
    Component* component) {
  if (component == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "bad component address");
  }
  if (component_ != NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "already initialized");
  }

  Status status;
  status = CreateComponentArgument(component_config, &component_argument_);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // create core instance.
  core_ = new CoreComponent(core_behavior);

  component_ = component;
  component_name_ = component_config.component_name;

  status = component_->InitComponent(core_, this, component_argument_);
  if (!status.ok()) {
    status.SetBlock(component_argument_.instance_name);
    SENSCORD_STATUS_TRACE(status);
    SENSCORD_LOG_ERROR("%s: InitComponent failed: status=%s",
        component_argument_.instance_name.c_str(), status.ToString().c_str());
    component_name_.clear();
    component_ = NULL;
    delete core_;
    core_ = NULL;
    return status;
  }
  return Status::OK();
}

/**
 * @brief Exit adapter.
 * @return Status object.
 */
Status ComponentAdapter::Exit() {
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }
  Status status = component_->ExitComponent();
  if (!status.ok()) {
    status.SetBlock(component_argument_.instance_name);
    SENSCORD_STATUS_TRACE(status);
    SENSCORD_LOG_ERROR("%s: ExitComponent failed: status=%s",
        component_argument_.instance_name.c_str(), status.ToString().c_str());
    return status;
  }
  DestroyAllPort();
  delete core_;
  core_ = NULL;
  return Status::OK();
}

/**
 * @brief Request to open port.
 * @param[in] (port_type) Port type to open.
 * @param[in] (port_id) Port ID to open.
 * @param[in] (stream_core) Stream core to connect.
 * @return Status object.
 */
Status ComponentAdapter::Open(const std::string& port_type,
                              int32_t port_id,
                              StreamCore* stream_core) {
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }

  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }
  Status status = port->Open(stream_core);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Request to close port.
 * @param[in] (port_type) Port type to close.
 * @param[in] (port_id) Port ID to close.
 * @param[in] (stream_core) Stream to close.
 * @return Status object.
 */
Status ComponentAdapter::Close(const std::string& port_type,
                               int32_t port_id,
                               const StreamCore* stream_core) {
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }

  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }
  Status status = port->Close(stream_core);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Request to start port.
 * @param[in] (port_type) Port type to start.
 * @param[in] (port_id) Port ID to start.
 * @param[in] (stream_core) Stream to start.
 * @return Status object.
 */
Status ComponentAdapter::Start(const std::string& port_type,
                               int32_t port_id,
                               StreamCore* stream_core) {
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }

  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }
  Status status = port->Start(stream_core);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Request to stop port.
 * @param[in] (port_type) Port type to stop.
 * @param[in] (port_id) Port ID to stop.
 * @param[in] (stream_core) Stream to start.
 * @return Status object.
 */
Status ComponentAdapter::Stop(const std::string& port_type,
                               int32_t port_id,
                               const StreamCore* stream_core) {
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }

  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }
  Status status = port->Stop(stream_core);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get the count of opened by stream key.
 * @param[in] (port_type) Port type.
 * @param[in] (port_id) Port ID.
 * @param[out] (count) Opened count.
 * @return Status object.
 */
Status ComponentAdapter::GetOpenedStreamCount(
    const std::string& port_type,
    int32_t port_id,
    uint32_t* count) {
  if (count == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "%s: invalid parameter", component_argument_.instance_name.c_str());
  }
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }

  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    // if no ports existed, it means no streams opened.
    *count = 0;
  } else {
    *count = port->GetOpenedStreamCount();
  }
  return Status::OK();
}

#ifdef SENSCORD_SERIALIZE
/**
 * @brief Get the serialized property from component port.
 * @param[in] (port_type) Port type to get.
 * @param[in] (port_id) Port ID to get.
 * @param[in] (stream) Owner of access to property.
 * @param[in] (property_key) Property key.
 * @param[in] (input_property) Input serialized property.
 * @param[in] (input_property_size) Input serialized property size.
 * @param[out] (output_property) Acquired serialized property.
 * @param[out] (output_property_size) Acquired serialized property size.
 * @return Status object.
 */
Status ComponentAdapter::GetSerializedProperty(
    const std::string& port_type,
    int32_t port_id,
    const StreamCore* stream,
    const std::string& property_key,
    const void* input_property,
    size_t input_property_size,
    void** output_property,
    size_t* output_property_size) {
  if ((output_property == NULL) || (output_property_size == NULL)) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "%s: invalid parameter",
        component_argument_.instance_name.c_str());
  }
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }

  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }

  bool component_property = false;
  util::PropertyKey key(property_key);
  PropertyAccessor* accessor = port->GetPropertyAccessor(key.GetPropertyKey());
  if (accessor != NULL) {
    component_property = true;
  } else {
    accessor = stream->GetSharedPropertyAccessor(property_key);
  }

  if (accessor == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound,
        "%s(%s.%" PRId32 "): unsupported property: key=%s",
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id,
        key.GetPropertyKey().c_str());
  }
  PropertyLockManager* property_locker = port->GetPropertyLocker();
  PropertyLocker locker(property_locker, stream, key, false);
  if (!locker.GetStatus().ok()) {
    return SENSCORD_STATUS_TRACE(locker.GetStatus());
  }
  // Get new property
  void* serialized_property = NULL;
  size_t serialized_size = 0;
  Status status = accessor->Get(
      property_key, input_property, input_property_size,
      &serialized_property, &serialized_size);
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);
    if (component_property) {
      status.SetBlock(component_argument_.instance_name);
    }
    SENSCORD_LOG_WARNING("%s(%s.%" PRId32 "): "
        "get property(%s) failed: status=%s",
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id,
        property_key.c_str(), status.ToString().c_str());
  } else {
    *output_property = serialized_property;
    *output_property_size = serialized_size;
  }
  return status;
}

/**
 * @brief Release the serialized property.
 * @param[in] (port_type) Port type.
 * @param[in] (port_id) Port ID.
 * @param[in] (property_key) Property key.
 * @param[in] (property) Serialized property data.
 * @param[in] (property_size) Serialized property size.
 * @return Status object.
 */
Status ComponentAdapter::ReleaseSerializedProperty(
    const std::string& port_type,
    int32_t port_id,
    const std::string& property_key,
    void* property,
    size_t property_size) {
  if (property == NULL) {
    return Status::OK();   // Do nothing.
  }
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }

  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }
  PropertyAccessor* accessor = port->GetPropertyAccessor(
      senscord::PropertyUtils::GetKey(property_key));
  if (accessor == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound,
        "%s(%s.%" PRId32 "): unsupported property: key=%s",
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id,
        property_key.c_str());
  }

  // Release property
  Status status = accessor->Release(property_key, property, property_size);
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);
    status.SetBlock(component_argument_.instance_name);
    SENSCORD_LOG_ERROR("%s(%s.%" PRId32 "): "
        "release property(%s) failed: status=%s",
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id,
        property_key.c_str(), status.ToString().c_str());
  }
  return status;
}

/**
 * @brief Set the serialized property from component port.
 * @param[in] (port_type) Port type.
 * @param[in] (port_id) Port ID.
 * @param[in] (stream) Owner of access to property.
 * @param[in] (property_key) Property key.
 * @param[in] (property) Serialized property data.
 * @param[in] (property_size) Serialized property size.
 * @return Status object.
 */
Status ComponentAdapter::SetSerializedProperty(
    const std::string& port_type,
    int32_t port_id,
    const StreamCore* stream,
    const std::string& property_key,
    const void* property,
    size_t property_size) {
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }

  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }

  bool component_property = false;
  util::PropertyKey key(property_key);
  PropertyAccessor* accessor = port->GetPropertyAccessor(key.GetPropertyKey());
  if (accessor != NULL) {
    component_property = true;
  } else {
    accessor = stream->GetSharedPropertyAccessor(property_key);
  }

  if (accessor == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound,
        "%s(%s.%" PRId32 "): unsupported property: key=%s",
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id,
        property_key.c_str());
  }
  PropertyLockManager* property_locker = port->GetPropertyLocker();
  PropertyLocker locker(property_locker, stream, key, true);
  if (!locker.GetStatus().ok()) {
    return SENSCORD_STATUS_TRACE(locker.GetStatus());
  }
  // Set property
  Status status = accessor->Set(property_key, property, property_size);
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);
    if (component_property) {
      status.SetBlock(component_argument_.instance_name);
    }
    SENSCORD_LOG_WARNING("%s(%s.%" PRId32 "): "
        "set property(%s) failed: status=%s",
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id,
        property_key.c_str(), status.ToString().c_str());
  }
  return status;
}
#else
/**
 * @brief Get the property from component port.
 * @param[in] (port_type) Port type to get.
 * @param[in] (port_id) Port ID to get.
 * @param[in] (stream) Owner of access to property.
 * @param[in] (property_key) Property key.
 * @param[in,out] (property) Property for input / output.
 * @return Status object.
 */
Status ComponentAdapter::GetProperty(
    const std::string& port_type,
    int32_t port_id,
    const StreamCore* stream,
    const std::string& property_key,
    void* property) {
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }
  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }

  bool component_property = false;
  util::PropertyKey key(property_key);
  PropertyAccessor* accessor = port->GetPropertyAccessor(key.GetPropertyKey());
  if (accessor != NULL) {
    component_property = true;
  } else {
    accessor = stream->GetSharedPropertyAccessor(property_key);
  }
  if (accessor == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound,
        "%s(%s.%" PRId32 "): unsupported property: key=%s",
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id,
        property_key.c_str());
  }

  PropertyLockManager* property_locker = port->GetPropertyLocker();
  PropertyLocker locker(property_locker, stream, key, false);
  if (!locker.GetStatus().ok()) {
    return SENSCORD_STATUS_TRACE(locker.GetStatus());
  }
  // Get property
  Status status = accessor->Get(property_key, property);
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);
    if (component_property) {
      status.SetBlock(component_argument_.instance_name);
    }
    SENSCORD_LOG_ERROR("%s(%s.%" PRId32 "): "
        "get property(%s) failed: status=%s",
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id,
        property_key.c_str(), status.ToString().c_str());
  }
  return status;
}

/**
 * @brief Set the property from component port.
 * @param[in] (port_type) Port type.
 * @param[in] (port_id) Port ID.
 * @param[in] (stream) Owner of access to property.
 * @param[in] (property_key) Property key.
 * @param[in] (property) Property data.
 * @return Status object.
 */
Status ComponentAdapter::SetProperty(
    const std::string& port_type,
    int32_t port_id,
    const StreamCore* stream,
    const std::string& property_key,
    const void* property) {
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }
  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }

  bool component_property = false;
  util::PropertyKey key(property_key);
  PropertyAccessor* accessor = port->GetPropertyAccessor(key.GetPropertyKey());
  if (accessor != NULL) {
    component_property = true;
  } else {
    accessor = stream->GetSharedPropertyAccessor(property_key);
  }
  if (accessor == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound,
        "%s(%s.%" PRId32 "): unsupported property: key=%s",
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id,
        property_key.c_str());
  }

  PropertyLockManager* property_locker = port->GetPropertyLocker();
  PropertyLocker locker(property_locker, stream, key, true);
  if (!locker.GetStatus().ok()) {
    return SENSCORD_STATUS_TRACE(locker.GetStatus());
  }
  // Set property
  Status status = accessor->Set(property_key, property);
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);
    if (component_property) {
      status.SetBlock(component_argument_.instance_name);
    }
    SENSCORD_LOG_ERROR("%s(%s.%" PRId32 "): "
        "set property(%s) failed: status=%s",
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id,
        property_key.c_str(), status.ToString().c_str());
  }
  return status;
}
#endif  // SENSCORD_SERIALIZE

/**
 * @brief Lock to access properties.
 * @param[in] (port_type) Port type to lock.
 * @param[in] (port_id) Port ID to lock.
 * @param[in] (stream_core) Stream to lock.
 * @param[in] (keys) Property keys for lock targets.
 * @param[in] (timeout_msec) Time of wait msec if locked already.
 * @param[out] (lock_resource) Locked properties resource.
 * @return Status object.
 */
Status ComponentAdapter::LockProperty(
    const std::string& port_type,
    int32_t port_id,
    const StreamCore* stream_core,
    const std::set<util::PropertyKey>& keys,
    int32_t timeout_msec,
    PropertyLockResource** lock_resource) {
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }

  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }

  // check supported keys
  for (std::set<util::PropertyKey>::const_iterator itr = keys.begin(),
        end = keys.end(); itr != end; ++itr) {
    PropertyAccessor* accessor =
        port->GetPropertyAccessor(itr->GetPropertyKey());
    if (accessor == NULL) {
      accessor = stream_core->GetSharedPropertyAccessor(itr->GetPropertyKey());
    }
    if (accessor == NULL) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseNotFound,
          "%s(%s.%" PRId32 "): unsupported property: key=%s",
          component_argument_.instance_name.c_str(),
          port_type.c_str(), port_id, itr->GetPropertyKey().c_str());
    }
  }

  // lock property
  PropertyLockManager* property_locker = port->GetPropertyLocker();
  Status status = property_locker->LockProperty(
      stream_core, keys, timeout_msec, lock_resource);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Unlock to access properties.
 * @param[in] (port_type) Port type to unlock.
 * @param[in] (port_id) Port ID to unlock.
 * @param[in] (stream_core) Stream to unlock.
 * @param[in] (lock_resource) Locked properties resource.
 * @return Status object.
 */
Status ComponentAdapter::UnlockProperty(
    const std::string& port_type,
    int32_t port_id,
    const StreamCore* stream_core,
    PropertyLockResource* lock_resource) {
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }

  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }
  PropertyLockManager* property_locker = port->GetPropertyLocker();
  Status status = property_locker->UnlockProperty(stream_core, lock_resource);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Set user data to port.
 * @param[in] (port_type) Port type.
 * @param[in] (port_id) Port ID.
 * @param[in] (user_data) New user data.
 * @return Status object.
 */
Status ComponentAdapter::SetUserData(
    const std::string& port_type,
    int32_t port_id,
    const FrameUserData& user_data) {
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }

  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }
  Status status = port->SetUserData(user_data);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get the supported property key list from the port.
 * @param[in] (port_type) Port type.
 * @param[in] (port_id) Port ID.
 * @param[out] (key_list) Supported property key list.
 * @return Status object.
 */
Status ComponentAdapter::GetSupportedPropertyList(
    const std::string& port_type,
    int32_t port_id,
    std::set<std::string>* key_list) {
  if (key_list == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "%s: invalid parameter",
        component_argument_.instance_name.c_str());
  }

  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }
  Status status = port->GetSupportedPropertyList(key_list);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Release the frame pushed from the port.
 * @param[in] (port_type) Port type.
 * @param[in] (port_id) Port ID.
 * @param[in] (stream_core) Stream of frame released.
 * @param[in] (frameinfo) Frame info to release.
 * @param[in] (referenced_channel_ids) List of referenced channel IDs.
 * @return Status object.
 */
Status ComponentAdapter::ReleaseFrame(
    const std::string& port_type,
    int32_t port_id,
    StreamCore* stream_core,
    const FrameInfo& frameinfo,
    const std::vector<uint32_t>& referenced_channel_ids) {
  if (component_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "%s: not initialized",
        component_argument_.instance_name.c_str());
  }

  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
  }
  Status status = port->ReleaseFrame(stream_core, frameinfo,
                                     &referenced_channel_ids);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get property history book address.
 * @param[in] (port_type) Port type.
 * @param[in] (port_id) Port ID.
 * @return Property history book address.
 */
PropertyHistoryBook* ComponentAdapter::GetPropertyHistoryBook(
    const std::string& port_type,
    int32_t port_id) {
  ComponentPortCore* port = GetPort(port_type, port_id);
  if (port == NULL) {
    SENSCORD_LOG_ERROR("%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), port_type.c_str(), port_id);
    return NULL;
  }
  return port->GetPropertyHistoryBook();
}

/**
 * @brief Create new port.
 * @param[in] (type) Port type.
 * @param[in] (id) Port ID.
 * @param[out] (port) Created port address. (optional)
 * @return Status object.
 */
Status ComponentAdapter::CreatePort(const std::string& type,
                                    const int32_t id,
                                    ComponentPort** port) {
  if (type.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "%s: type name is empty", component_argument_.instance_name.c_str());
  }

  util::AutoLock lock(&mutex_);
  std::pair<ComponentPortMap::iterator, bool> ret =
      port_map_.insert(
          ComponentPortMap::value_type(PortType(id, type),
          ComponentPortInfo()));
  if (!ret.second) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAlreadyExists,
        "%s: already created port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), type.c_str(), id);
  }

  PropertyHistoryBook* history_book = new PropertyHistoryBook();
  ComponentPortCore* port_core = new ComponentPortCore(
      component_, component_argument_.instance_name, type, id, history_book);
  ret.first->second.port = port_core;
  ret.first->second.history_book = history_book;
  if (port != NULL) {
    *port = port_core;
  }

  return Status::OK();
}

/**
 * @brief Create new port.
 * @param[in] (type) Port type.
 * @param[in] (id) Port ID.
 * @param[out] (port) Created port address.
 * @param[in] (history_book) Property history book.
 * @return Status object.
 */
Status ComponentAdapter::CreatePort(const std::string& type,
                                    const int32_t id,
                                    ComponentPort** port,
                                    PropertyHistoryBook* history_book) {
  if (type.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "%s: type name is empty", component_argument_.instance_name.c_str());
  }

  util::AutoLock lock(&mutex_);
  ComponentPortInfo port_info = {};
  std::pair<ComponentPortMap::iterator, bool> ret =
      port_map_.insert(
          ComponentPortMap::value_type(PortType(id, type), port_info));
  if (!ret.second) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAlreadyExists,
        "%s: already created port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(), type.c_str(), id);
  }

  ComponentPortCore* port_core = new ComponentPortCore(
      component_, component_argument_.instance_name, type, id, history_book);
  ret.first->second.port = port_core;
  ret.first->second.history_book = NULL;
  if (port != NULL) {
    *port = port_core;
  }

  return Status::OK();
}

/**
 * @brief Destroy the port.
 * @param[in] (port) Created port address.
 * @return 0 means success or negative value means failed (error code).
 * @return Status object.
 */
Status ComponentAdapter::DestroyPort(ComponentPort* port) {
  if (port == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "%s: invalid parameter", component_argument_.instance_name.c_str());
  }
  util::AutoLock lock(&mutex_);
  ComponentPortMap::iterator itr = port_map_.find(
      PortType(port->GetPortId(), port->GetPortType()));
  if ((itr != port_map_.end()) && (itr->second.port == port)) {
    delete itr->second.port;
    delete itr->second.history_book;
    port_map_.erase(itr);
  } else {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "%s: unknown port: port=%s.%" PRId32,
        component_argument_.instance_name.c_str(),
        port->GetPortType().c_str(), port->GetPortId());
  }
  return Status::OK();
}

/**
 * @brief Destroy all ports.
 * @return Status object.
 */
Status ComponentAdapter::DestroyAllPort() {
  util::AutoLock lock(&mutex_);
  while (!port_map_.empty()) {
    ComponentPortMap::iterator itr = port_map_.begin();
    delete itr->second.port;
    delete itr->second.history_book;
    port_map_.erase(itr);
  }
  return Status::OK();
}

/**
 * @brief Get the created port.
 * @param[in] (type) Port type of created.
 * @param[in] (id) Port ID of created.
 * @return Non null means created port address or null means failed.
 */
ComponentPortCore* ComponentAdapter::GetPort(
    const std::string& type, const int32_t id) {
  ComponentPortCore* port = NULL;
  util::AutoLock lock(&mutex_);
  ComponentPortMap::iterator itr = port_map_.find(PortType(id, type));
  if (itr != port_map_.end()) {
    port = itr->second.port;
  }
  if (!port) {
    itr = port_map_.find(PortType(id, kAnyPortType));
    if (itr != port_map_.end()) {
      port = itr->second.port;
    }
  }
  return port;
}

/**
 * @brief Create component argument
 * @param[in] (config) Component configuration.
 * @param[out] (argument) Component argument.
 * @return Status object.
 */
Status ComponentAdapter::CreateComponentArgument(
    const ComponentInstanceConfig& config,
    ComponentArgument* argument) {
  // setting instance argument
  argument->instance_name = config.instance_name;
  argument->arguments = config.arguments;

  MemoryManager* memory_manager = MemoryManager::GetInstance();
  MemoryAllocator* allocator = NULL;
  for (std::map<std::string, std::string>::const_iterator
      itr = config.allocator_key_list.begin(),
      end = config.allocator_key_list.end(); itr != end; ++itr) {
    Status status = memory_manager->GetAllocator(itr->second, &allocator);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    argument->allocators.insert(std::make_pair(itr->first, allocator));
  }
  // least one allocator(default)
  if (argument->allocators.empty()) {
    Status status = memory_manager->GetAllocator(kAllocatorDefaultKey,
        &allocator);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    argument->allocators.insert(
        std::make_pair(kAllocatorNameDefault, allocator));
  }
  return Status::OK();
}

}   // namespace senscord
