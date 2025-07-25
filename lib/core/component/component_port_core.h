/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_COMPONENT_COMPONENT_PORT_CORE_H_
#define LIB_CORE_COMPONENT_COMPONENT_PORT_CORE_H_

#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include <set>

#include "senscord/osal.h"
#include "senscord/develop/component.h"
#include "senscord/develop/component_port.h"
#include "senscord/develop/property_accessor.h"
#include "core/internal_types.h"
#include "stream/stream_core.h"
#include "stream/property_history_book.h"
#include "util/mutex.h"
#include "component/property_lock_manager.h"

namespace senscord {

class PropertyLockManager;

/**
 * @brief Component's port core class.
 */
class ComponentPortCore : public ComponentPort {
 public:
  /**
   * @brief Constructor
   * @param[in] (component) Parent component.
   * @param[in] (component_instance_name) Parent instance name.
   * @param[in] (port_type) Port type.
   * @param[in] (port_id) Port ID.
   * @param[in] (history_book) Property history book.
   */
  ComponentPortCore(
      Component* component,
      const std::string& component_instance_name,
      const std::string& port_type,
      int32_t port_id,
      PropertyHistoryBook* history_book);

  /**
   * @brief Destructor
   */
  ~ComponentPortCore();

  /**
   * @brief Connect stream.
   * @param[in] (stream) Connecting stream.
   * @return Status object.
   */
  Status Open(StreamCore* stream);

  /**
   * @brief Disconnect stream.
   * @param[in] (stream) Disconnecting stream.
   * @return Status object.
   */
  Status Close(const StreamCore* stream);

  /**
   * @brief Start by stream.
   * @param[in] (stream) Startting stream.
   * @return Status object.
   */
  Status Start(StreamCore* stream);

  /**
   * @brief Start by stream.
   * @param[in] (stream) Startting stream.
   * @return Status object.
   */
  Status Stop(const StreamCore* stream);

  /**
   * @brief Get connected streams information.
   * @return Opened streams count.
   */
  uint32_t GetOpenedStreamCount();

  /**
   * @brief Get whether be connected or not.
   * @return State of connecting.
   */
  virtual bool IsConnected() const;

  /**
   * @brief Get the port type.
   * @return Port type.
   */
  virtual const std::string& GetPortType() const { return port_type_; }

  /**
   * @brief Get the port ID.
   * @return Port ID.
   */
  virtual int32_t GetPortId() const { return port_id_; }

  /**
   * @brief Get the instance name.
   * @return Instance name.
   */
  const std::string& GetInstanceName() const {
    return component_instance_name_;
  }

  /**
   * @brief Send the multiple frames to the connected stream.
   * @param[in] (frames) List of frame information to send.
   * @param[out] (dropped_frames) List of pointer of dropped frames.
   * @return Status object.
   */
  virtual Status SendFrames(
      const std::vector<FrameInfo>& frames,
      std::vector<const FrameInfo*>* dropped_frames);

  /**
   * @brief Send the event to the connected stream.
   * @param[in] (event) Event type.
   * @param[in] (args) Event argument.
   * @return Status object.
   */
  virtual Status SendEvent(
      const std::string& event, const EventArgument& args);

  /**
   * @brief Register property accessor.
   * @param[in] (accessor) Property accessor.
   * @return Status object.
   */
  virtual Status RegisterPropertyAccessor(PropertyAccessor* accessor);

  /**
   * @brief Unregister property accessor.
   * @param[in] (property_key) Key of property.
   * @param[out] (accessor) Registered accessor address. (optional)
   * @return Status object.
   */
  virtual Status UnregisterPropertyAccessor(
    const std::string& property_key,
    PropertyAccessor** accessor);

  /**
   * @brief Get property interface.
   * @param[in] (key) Key of property.
   * @return Property accessor interface.
   */
  PropertyAccessor* GetPropertyAccessor(const std::string& key);

  /**
   * @brief Get the supported property key list on this port.
   * @param[out] (key_list) Supported property key list.
   * @return Status object.
   */
  Status GetSupportedPropertyList(std::set<std::string>* key_list);

  /**
   * @brief Get property locker.
   * @return Property locker.
   */
  PropertyLockManager* GetPropertyLocker();

  /**
   * @brief Register the callback for LockProperty
   * @param [in] (callback) The callback called by LockProperty.
   * @param [in] (private_data) Value with callback called.
   */
  virtual void RegisterLockPropertyCallback(
    OnLockPropertyCallback callback, void* private_data);

  /**
   * @brief Register the callback for UnlockProperty
   * @param [in] (callback) The callback called by UnlockProperty.
   * @param [in] (private_data) Value with callback called.
   */
  virtual void RegisterUnlockPropertyCallback(
    OnUnlockPropertyCallback callback, void* private_data);

  /**
   * @brief Release frame list from stream.
   * @param[in] (stream) Stream sent frame.
   * @param[in] (frameinfo) Sent frame information.
   * @param[in] (referenced_channel_ids) List of referenced channel IDs.
   * @return Status object.
   */
  Status ReleaseFrame(StreamCore* stream, const FrameInfo& frameinfo,
                      const std::vector<uint32_t>* referenced_channel_ids);

  /**
   * @brief Set user data to all connected streams.
   * @param[in] (user_data) New user data.
   * @return Status object.
   */
  virtual Status SetUserData(const FrameUserData& user_data);

  /**
   * @brief Get property history book address.
   * @return Property history book address.
   */
  PropertyHistoryBook* GetPropertyHistoryBook();

#ifdef SENSCORD_PLAYER
  /**
   * @brief Update the port (stream) type. For only the player component.
   * @param[in] (port_type) New port (stream) type.
   * @return Status object.
   */
  virtual Status SetType(const std::string& port_type);
#endif  // SENSCORD_PLAYER

  /**
   * @brief Get to whether opened or not.
   * @param[in] (stream) Owner stream.
   * @return Whether opened or not.
   */
  bool IsOpenedStream(const StreamCore* stream) const;

 protected:
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
    size_t property_size);
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
      uint32_t channel_id, const std::string& key, const void* property,
      const PropertyFactoryBase& factory);
#endif  // SENSCORD_SERIALIZE

 private:
  /**
   * @brief Frame sending state.
   */
  enum FrameSendingState {
    kNotSendingYet = 0,
    kSendingFailed,
    kReleased
  };

  /**
   * @brief Frame sending state.
   */
  typedef std::map<StreamCore*, FrameSendingState> FrameSending;

  /**
   * @brief Parameters of the send frame.
   */
  struct SendFrameParameter {
    FrameSending sending_state;
    std::set<uint32_t> referenced_channel_ids;
  };

  /**
   * @brief List of StreamCore pointer.
   */
  typedef std::vector<StreamCore*> StreamCoreList;

  /**
   * @brief Iterator of StreamCore list.
   */
  typedef StreamCoreList::iterator StreamCoreIterator;

  /**
   * @brief Const iterator of StreamCore list.
   */
  typedef StreamCoreList::const_iterator StreamCoreConstIterator;

  /**
   * @brief Get to whether started or not.
   * @param[in] (stream) Owner stream.
   * @return Whether started or not.
   */
  bool IsStartedStream(const StreamCore* stream) const;

  /**
   * @brief Release frame list from stream.
   * @param[in] (stream) Stream sent frame.
   * @param[in] (frameinfo) Sent frame information.
   * @param[in] (referenced_channel_ids) List of referenced channel IDs.
   * @param[in] (state) Release causation.
   * @return Status object.
   */
  Status ReleaseFrame(
      StreamCore* stream,
      const FrameInfo& frameinfo,
      const std::vector<uint32_t>* referenced_channel_ids,
      FrameSendingState state);

  /**
   * @brief Remove stream from list.
   * @param[in] (list) Stream list.
   * @param[in] (stream) Removing stream.
   * @return Status object.
   */
  Status RemoveStream(StreamCoreList* list, const StreamCore* stream) const;

  /**
   * @brief Check if the arguments are for the same stream.
   * @param[in] (stream) New stream.
   * @return Status object.
   */
  Status IsSameStreamArguments(const StreamCore* stream) const;

#ifdef SENSCORD_STATUS_MESSAGE_ENABLED
  /**
   * @brief Get the frame sending state as a string. (For analysis)
   * @param[out] (value) String in sending state.
   */
  void GetFrameSendingStateString(std::string* value) const;
#endif  // SENSCORD_STATUS_MESSAGE_ENABLED

 private:
  // parent component
  Component* component_;

  // parent component instance name
  const std::string& component_instance_name_;

  // port type
  std::string port_type_;

  // port id
  int32_t port_id_;

  // port argument
  ComponentPortArgument port_args_;

  // flag means connects to client component.
  bool is_client_port_;

  // mutex for streams list
  mutable util::Mutex mutex_streams_opened_;
  mutable util::Mutex mutex_streams_started_;

  // mutex for state change
  mutable util::Mutex mutex_state_change_;

  // opened streams list
  StreamCoreList streams_opened_;

  // started streams list
  StreamCoreList streams_started_;

  // mutex for sent frames list
  util::Mutex mutex_frames_;

  // frame_sequence_number x sent stream state list
  typedef std::map<uint64_t, SendFrameParameter> SentFramesMap;
  SentFramesMap sent_frames_;

  // property key x accessor list
  std::map<std::string, PropertyAccessor*> properties_;

  // property update histories
  PropertyHistoryBook* history_book_;

  // property lock manager
  PropertyLockManager* property_locker_;
};

}   // namespace senscord
#endif  // LIB_CORE_COMPONENT_COMPONENT_PORT_CORE_H_
