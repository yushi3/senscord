/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_COMPONENT_COMPONENT_ADAPTER_H_
#define LIB_CORE_COMPONENT_COMPONENT_ADAPTER_H_

#include <map>
#include <string>
#include <vector>
#include <utility>
#include <set>

#include "core/internal_types.h"
#include "core/core_behavior.h"
#include "senscord/develop/component.h"
#include "senscord/develop/component_port_manager.h"
#include "component/component_port_core.h"
#include "stream/property_history_book.h"
#include "stream/stream_core.h"
#include "util/mutex.h"

namespace senscord {

// ignore port type
static const char kAnyPortType[] = "*";

/**
 * @brief Component adapter class.
 */
class ComponentAdapter : public ComponentPortManager {
 public:
  /**
   * @brief Constructor
   */
  ComponentAdapter();

  /**
   * @brief Destructor
   */
  ~ComponentAdapter();

  /**
   * @brief Initialize.
   * @param[in] (component_config) Component configuration.
   * @param[in] (core_behavior) Core behavior.
   * @param[in] (component) Handled component address.
   * @return Status object.
   */
  Status Init(
      const ComponentInstanceConfig& component_config,
      const CoreBehavior* core_behavior,
      Component* component);

  /**
   * @brief Exit adapter.
   * @return Status object.
   */
  Status Exit();

  /**
   * @brief Request to open port.
   * @param[in] (port_type) Port type to open.
   * @param[in] (port_id) Port ID to open.
   * @param[in] (stream_core) Stream core to connect.
   * @return Status object.
   */
  Status Open(
    const std::string& port_type,
    int32_t port_id,
    StreamCore* stream_core);

  /**
   * @brief Request to close port.
   * @param[in] (port_type) Port type to close.
   * @param[in] (port_id) Port ID to close.
   * @param[in] (stream_core) Stream to close.
   * @return Status object.
   */
  Status Close(
    const std::string& port_type,
    int32_t port_id,
    const StreamCore* stream_core);

  /**
   * @brief Request to start port.
   * @param[in] (port_type) Port type to start.
   * @param[in] (port_id) Port ID to start.
   * @param[in] (stream_core) Stream to start.
   * @return Status object.
   */
  Status Start(
    const std::string& port_type,
    int32_t port_id,
    StreamCore* stream_core);

  /**
   * @brief Request to stop port.
   * @param[in] (port_type) Port type to stop.
   * @param[in] (port_id) Port ID to stop.
   * @param[in] (stream_core) Stream to stop.
   * @return Status object.
   */
  Status Stop(
    const std::string& port_type,
    int32_t port_id,
    const StreamCore* stream_core);

  /**
   * @brief Get the count of opened by stream key.
   * @param[in] (port_type) Port type.
   * @param[in] (port_id) Port ID.
   * @param[out] (count) Opened count.
   * @return Status object.
   */
  Status GetOpenedStreamCount(
    const std::string& port_type,
    int32_t port_id,
    uint32_t* count);

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
  Status GetSerializedProperty(
    const std::string& port_type,
    int32_t port_id,
    const StreamCore* stream,
    const std::string& property_key,
    const void* input_property,
    size_t input_property_size,
    void** output_property,
    size_t* output_property_size);

  /**
   * @brief Release the serialized property.
   * @param[in] (port_type) Port type.
   * @param[in] (port_id) Port ID.
   * @param[in] (property_key) Property key.
   * @param[in] (property) Serialized property data.
   * @param[in] (property_size) Serialized property size.
   * @return Status object.
   */
  Status ReleaseSerializedProperty(
    const std::string& port_type,
    int32_t port_id,
    const std::string& property_key,
    void* property,
    size_t property_size);

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
  Status SetSerializedProperty(
    const std::string& port_type,
    int32_t port_id,
    const StreamCore* stream,
    const std::string& property_key,
    const void* property,
    size_t property_size);
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
  Status GetProperty(
      const std::string& port_type,
      int32_t port_id,
      const StreamCore* stream,
      const std::string& property_key,
      void* property);

  /**
   * @brief Set the property from component port.
   * @param[in] (port_type) Port type.
   * @param[in] (port_id) Port ID.
   * @param[in] (stream) Owner of access to property.
   * @param[in] (property_key) Property key.
   * @param[in] (property) Property data.
   * @return Status object.
   */
  Status SetProperty(
      const std::string& port_type,
      int32_t port_id,
      const StreamCore* stream,
      const std::string& property_key,
      const void* property);
#endif  // SENSCORD_SERIALIZE

  /**
   * @brief Set user data to port.
   * @param[in] (port_type) Port type.
   * @param[in] (port_id) Port ID.
   * @param[in] (user_data) New user data.
   * @return Status object.
   */
  Status SetUserData(
    const std::string& port_type,
    int32_t port_id,
    const FrameUserData& user_data);

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
  Status LockProperty(
    const std::string& port_type,
    int32_t port_id,
    const StreamCore* stream_core,
    const std::set<util::PropertyKey>& keys,
    int32_t timeout_msec,
    PropertyLockResource** lock_resource);

  /**
   * @brief Unlock to access properties.
   * @param[in] (port_type) Port type to unlock.
   * @param[in] (port_id) Port ID to unlock.
   * @param[in] (stream_core) Stream to unlock.
   * @param[in] (lock_resource) Locked properties resource.
   * @return Status object.
   */
  Status UnlockProperty(
    const std::string& port_type,
    int32_t port_id,
    const StreamCore* stream_core,
    PropertyLockResource* lock_resource);

  /**
   * @brief Release the frame pushed from the port.
   * @param[in] (port_type) Port type.
   * @param[in] (port_id) Port ID.
   * @param[in] (stream_core) Stream of frame released.
   * @param[in] (frameinfo) Frame info to release.
   * @param[in] (referenced_channel_ids) List of referenced channel IDs.
   * @return Status object.
   */
  Status ReleaseFrame(
    const std::string& port_type,
    int32_t port_id,
    StreamCore* stream_core,
    const FrameInfo& frameinfo,
    const std::vector<uint32_t>& referenced_channel_ids);

  /**
   * @brief Get the supported property key list from the port.
   * @param[in] (port_type) Port type.
   * @param[in] (port_id) Port ID.
   * @param[out] (key_list) Supported property key list.
   * @return Status object.
   */
  Status GetSupportedPropertyList(
    const std::string& port_type,
    int32_t port_id,
    std::set<std::string>* key_list);

  /**
   * @brief Get component address by Init().
   * @return Component address.
   */
  Component* GetComponent() const {
    return component_;
  }

  /**
   * @brief Get component real name by Init().
   * @return Component real name.
   */
  const std::string& GetComponentName() const {
    return component_name_;
  }

  /**
   * @brief Get component instance name by Init().
   * @return Component instance name.
   */
  const std::string& GetComponentInstanceName() const {
    return component_argument_.instance_name;
  }

  /**
   * @brief Increase the reference count.
   */
  void AddReference() { ++refcount_; }

  /**
   * @brief Decrease the reference count.
   */
  void ReleaseReference() { --refcount_; }

  /**
   * @brief Get the reference count.
   * @return Component reference count.
   */
  int GetReferenceCount() const { return refcount_; }

  /**
   * @brief Get property history book address.
   * @param[in] (port_type) Port type.
   * @param[in] (port_id) Port ID.
   * @return Property history book address.
   */
  PropertyHistoryBook* GetPropertyHistoryBook(
    const std::string& port_type,
    int32_t port_id);

  /**
   * @brief Create new port.
   * @param[in] (type) Port type.
   * @param[in] (id) Port ID.
   * @param[out] (port) Created port address. (optional)
   * @return Status object.
   */
  virtual Status CreatePort(
    const std::string& type,
    int32_t id,
    ComponentPort** port);

  /**
   * @brief Create new port (specify history book).
   * @param[in] (type) Port type.
   * @param[in] (id) Port ID.
   * @param[out] (port) Created port address.
   * @param[in] (history_book) Property history book.
   * @return Status object.
   */
  virtual Status CreatePort(
    const std::string& type,
    int32_t id,
    ComponentPort** port,
    PropertyHistoryBook* history_book);

  /**
   * @brief Destroy the port.
   * @param[in] (port) Created port address.
   * @return Status object.
   */
  virtual Status DestroyPort(ComponentPort* port);

  /**
   * @brief Destroy all ports.
   * @return Status object.
   */
  virtual Status DestroyAllPort();

  /**
   * @brief Get the created port.
   * @param[in] (type) Port type of created.
   * @param[in] (id) Port ID of created.
   * @return Non null means created port address or null means failed.
   */
  ComponentPortCore* GetPort(const std::string& type, int32_t id);

  /**
   * @brief Create component argument
   * @param[in] (config) Component configuration.
   * @param[out] (argument) Component argument.
   * @return Status object.
   */
  Status CreateComponentArgument(
      const ComponentInstanceConfig& config,
      ComponentArgument* argument);

 private:
  struct ComponentPortInfo {
    ComponentPortCore* port;
    PropertyHistoryBook* history_book;
  };
  typedef std::pair<int32_t, std::string> PortType;
  typedef std::map<PortType, ComponentPortInfo> ComponentPortMap;

  // Core instance
  Core* core_;

  // Component argument.
  ComponentArgument component_argument_;

  // Component real name.
  std::string component_name_;

  // Component instance.
  Component* component_;

  /**
   * Created ports list.
   * port index x port pointer.
   */
  ComponentPortMap port_map_;

  // Mutex for port map access.
  util::Mutex mutex_;

  // Count of refer this adapter.
  int refcount_;
};

}   // namespace senscord
#endif  // LIB_CORE_COMPONENT_COMPONENT_ADAPTER_H_
