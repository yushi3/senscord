/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_COMPONENT_PORT_H_
#define SENSCORD_DEVELOP_COMPONENT_PORT_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <set>

#include "senscord/config.h"
#include "senscord/noncopyable.h"
#include "senscord/senscord_types.h"
#include "senscord/status.h"
#include "senscord/stream.h"
#include "senscord/develop/common_types.h"
#include "senscord/develop/property_accessor.h"
#include "senscord/develop/deserialized_property_accessor.h"

namespace senscord {

/**
 * @brief Interface class of the component port.
 */
class ComponentPort : private util::Noncopyable {
 public:
  /**
   * @brief Send the frame to the connected stream.
   * @param[in] (frameinfo) Information of the frame to send.
   * @return Status object.
   */
  Status SendFrame(const FrameInfo& frameinfo) {
    std::vector<FrameInfo> frames(1, frameinfo);
    Status status = SendFrames(
        frames, reinterpret_cast<std::vector<const FrameInfo*>*>(NULL));
    return SENSCORD_STATUS_TRACE(status);
  }

  /**
   * @deprecated
   * @brief Send the multiple frames to the connected stream.
   * @param[in] (frames) List of frame information to send.
   * @param[out] (dropped_frames) List of sequence numbers of dropped frames.
   * @return Status object.
   */
  Status SendFrames(
      const std::vector<FrameInfo>& frames,
      std::vector<uint64_t>* dropped_frames) {
    if (dropped_frames == NULL) {
      Status status = SendFrames(
          frames, reinterpret_cast<std::vector<const FrameInfo*>*>(NULL));
      return SENSCORD_STATUS_TRACE(status);
    } else {
      std::vector<const FrameInfo*> dropped;
      Status status = SendFrames(frames, &dropped);
      for (std::vector<const FrameInfo*>::const_iterator itr = dropped.begin(),
          end = dropped.end(); itr != end; ++itr) {
        dropped_frames->push_back((*itr)->sequence_number);
      }
      return SENSCORD_STATUS_TRACE(status);
    }
  }

  /**
   * @brief Send the multiple frames to the connected stream.
   * @param[in] (frames) List of frame information to send.
   * @param[out] (dropped_frames) List of pointer of dropped frames.
   * @return Status object.
   */
  virtual Status SendFrames(
      const std::vector<FrameInfo>& frames,
      std::vector<const FrameInfo*>* dropped_frames) = 0;

  /**
   * @brief Update frame channel property.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Property to updated.
   * @return Status object.
   */
  template <typename T>
  Status UpdateFrameProperty(
    uint32_t channel_id,
    const std::string& key,
    const T* property);

#ifdef SENSCORD_SERIALIZE
  /**
   * @brief Update the serialized property for frame channel.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Serialized property.
   * @param[in] (property_size) Serialized property size.
   * @return Status object.
   */
  virtual Status UpdateFrameSerializedProperty(
    uint32_t channel_id,
    const std::string& key,
    const void* property,
    size_t property_size) = 0;
#else
  /**
   * @brief Update frame channel property.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Property to updated.
   * @param[in] (factory) Property factory.
   * @return Status object.
   */
  virtual Status UpdateFramePropertyWithFactory(
      uint32_t channel_id,
      const std::string& key,
      const void* property,
      const PropertyFactoryBase& factory) = 0;
#endif  // SENSCORD_SERIALIZE

  /**
   * @brief Send the event to the connected stream.
   * @param[in] (event) Event type to send.
   * @param[in] (args) Event argument.
   * @return Status object.
   */
  virtual Status SendEvent(
      const std::string& event, const EventArgument& args) = 0;

  /**
   * @deprecated
   * @brief Send the event to the connected stream.
   * @param[in] (event) Event type to send.
   * @param[in] (reserved) Not used.
   * @return Status object.
   */
  Status SendEvent(const std::string& event, const void* reserved) {
    EventArgument args;
    Status status = SendEvent(event, args);
    return SENSCORD_STATUS_TRACE(status);
  }

  /**
   * @brief Get the port type.
   * @return Type of this port.
   */
  virtual const std::string& GetPortType() const = 0;

  /**
   * @brief Get the port ID.
   * @return ID of this port.
   */
  virtual int32_t GetPortId() const = 0;

  /**
   * @brief Register property accessor.
   * @param[in] (accessor) Property accessor.
   * @return Status object.
   */
  virtual Status RegisterPropertyAccessor(PropertyAccessor* accessor) = 0;

  /**
   * @brief Unregister property accessor.
   * @param[in] (property_key) Key of property.
   * @param[out] (accessor) Registered accessor address. (optional)
   * @return Status object.
   */
  virtual Status UnregisterPropertyAccessor(
    const std::string& property_key,
    PropertyAccessor** accessor) = 0;

  /**
   * @brief Set user data to all connected streams.
   * @param[in] (user_data) New user data.
   * @return Status object.
   */
  virtual Status SetUserData(const FrameUserData& user_data) = 0;

  /**
   * @brief OnLockProperty callback arguments.
   */
  struct LockPropertyArguments {
    /** property keys. */
    std::set<std::string> keys;
    /** lock resource */
    PropertyLockResource* lock_resource;
    /** timeout milliseconds.
        0 means polling and the negative value means forever. */
    int32_t timeout_msec;
  };

  /**
   * @brief The type of the callback on LockProperty called.
   * @param[in] (port) The port of component.
   * @param[in] (args) The arguments of callback.
   * @param[in] (private_data) The value by callback registered.
   */
  typedef Status (* OnLockPropertyCallback)(
    ComponentPort* port,
    const LockPropertyArguments& args,
    void* private_data);

  /**
   * @brief The type of the callback on UnlockProperty called.
   * @param[in] (port) The port of component.
   * @param[in] (lock_resource) Lock resource.
   * @param[in] (private_data) The value by callback registered.
   */
  typedef Status (* OnUnlockPropertyCallback)(
    ComponentPort* port,
    PropertyLockResource* lock_resource,
    void* private_data);

  /**
   * @brief Register the callback for LockProperty
   * @oaram [in] (callback) The callback called by LockProperty.
   * @param [in] (private_data) Value with callback called.
   */
  virtual void RegisterLockPropertyCallback(
    OnLockPropertyCallback callback, void* private_data) = 0;

  /**
   * @brief Register the callback for UnlockProperty
   * @oaram [in] (callback) The callback called by UnlockProperty.
   * @param [in] (private_data) Value with callback called.
   */
  virtual void RegisterUnlockPropertyCallback(
    OnUnlockPropertyCallback callback, void* private_data) = 0;

#ifdef SENSCORD_PLAYER
  /**
   * @brief Update the port (stream) type. For only the player component.
   * @param[in] (port_type) New port (stream) type.
   * @return Status object.
   */
  virtual Status SetType(const std::string& port_type) = 0;
#endif  // SENSCORD_PLAYER

  /**
   * @brief Virtual destructor.
   */
  virtual ~ComponentPort() {}

 protected:
  /**
   * @brief Get whether be connected or not.
   * @return "true" means this port was connected a stream.
   */
  virtual bool IsConnected() const = 0;
};

}   // namespace senscord

// implementations of template methods.
#include "senscord/develop/component_port_private.h"

#endif  // SENSCORD_DEVELOP_COMPONENT_PORT_H_
