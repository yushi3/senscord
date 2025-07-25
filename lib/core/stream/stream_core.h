/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_STREAM_STREAM_CORE_H_
#define LIB_CORE_STREAM_STREAM_CORE_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <set>

#include "senscord/osal.h"
#include "senscord/stream.h"
#include "senscord/develop/property_accessor.h"
#include "senscord/develop/extension.h"
#include "core/internal_types.h"
#include "core/core_behavior.h"
#include "util/mutex.h"
#include "util/observer.h"
#include "util/resource_list.h"
#include "stream/frame_observer.h"
#include "stream/event_observer.h"
#include "stream/property_history_book.h"
#include "record/frame_recorder.h"

#include "senscord/develop/property_types_private.h"

namespace senscord {

/**
 * @brief Local stream state.
 */
enum StreamLocalState {
  kStreamLocalStateNotInit,
  kStreamLocalStateInit,
  kStreamLocalStateReady,
  kStreamLocalStateRunning,
};

// pre-definition
class FrameManager;
class ComponentAdapter;
class StreamRecorder;

/**
 * @brief Stream internal class.
 */
class StreamCore : public Stream {
 public:
  /**
   * @brief Initialize and setup this stream.
   * @param (config) Stream configuration.
   * @return Status object.
   */
  virtual Status Init(const StreamSetting& config);

  /**
   * @brief Finalize this stream.
   * @return Status object.
   */
  virtual Status Exit();

  /**
   * @brief Open this stream.
   * @param[in] (core_behavior) Core behavior.
   * @return Status object.
   */
  virtual Status Open(const CoreBehavior* core_behavior);

  /**
   * @brief Close this stream.
   * @return Status object.
   */
  virtual Status Close();

  /**
   * @brief Start this stream.
   * @return Status object.
   */
  virtual Status Start();

  /**
   * @brief Stop this stream.
   * @return Status object.
   */
  virtual Status Stop();

  /**
   * @brief Get the received frame.
   * @param[out] (frame) Location of received frame.
   * @param[in] (timeout_msec) Time of wait msec if no received.
   *                           0 is polling, minus is forever.
   * @return Status object.
   */
  virtual Status GetFrame(Frame** frame, int32_t timeout_msec);

  /**
   * @brief Release the gotten frame.
   * @param[in] (frame) Received frame by GetFrame().
   * @return Status object.
   */
  virtual Status ReleaseFrame(Frame* frame);

  /**
   * @brief Release the gotten frame.
   *
   * Use this function if you do not refer to the raw data of the channel.
   *
   * @param[in] (frame) Received frame by GetFrame().
   * @return Status object.
   */
  virtual Status ReleaseFrameUnused(Frame* frame);

  /**
   * @brief Clear frames have not gotten.
   * @param[out] (frame_number) number of cleared frames.
   * @return Status object.
   */
  virtual Status ClearFrames(int32_t* frame_number);

#ifdef SENSCORD_SERIALIZE
  /**
   * @brief Get the serialized property.
   * @param[in] (property_key) Key of property to get.
   * @param[in] (input_property) Location of input property.
   * @param[in] (input_property_size) Size of input property.
   * @param[out] (output_property) Location of output property.
   * @param[out] (output_property_size) Size of output property.
   * @return Status object.
   */
  virtual Status GetSerializedProperty(
    const std::string& property_key,
    const void* input_property,
    size_t input_property_size,
    void** output_property,
    size_t* output_property_size);

  /**
   * @brief Release the serialized property.
   * @param[in] (property_key) Key of property to release.
   * @param[in] (property) Location of property.
   * @param[in] (property_size) Size of property.
   * @return Status object.
   */
  virtual Status ReleaseSerializedProperty(
    const std::string& property_key,
    void* property,
    size_t property_size);

  /**
   * @brief Set the serialized property.
   * @param[in] (property_key) Key of property to set.
   * @param[in] (property) Location of property.
   * @param[in] (property_size) Size of property.
   * @return Status object.
   */
  virtual Status SetSerializedProperty(
    const std::string& property_key,
    const void* property,
    size_t property_size);
#else
  /**
   * @brief Get the property.
   * @param[in] (property_key) Key of property to get.
   * @param[out] (property) Location of property.
   * @return Status object.
   */
  virtual Status GetProperty(
      const std::string& property_key, void* property);

  /**
   * @brief Set the property with key.
   * @param[in] (property_key) Key of property to set.
   * @param[in] (property) Location of property.
   * @return Status object.
   */
  virtual Status SetProperty(
      const std::string& property_key, const void* property);
#endif  // SENSCORD_SERIALIZE

  /**
   * @brief Get the supported property key list on this stream.
   * @param[out] (key_list) Supported property key list.
   * @return Status object.
   */
  virtual Status GetPropertyList(
    std::vector<std::string>* key_list) const;

  /**
   * @brief Lock to access properties.
   * @param[in] (timeout_msec) Time of wait msec if locked already.
   *                           0 is polling, minus is forever.
   * @return Status object.
   */
  virtual Status LockProperty(int32_t timeout_msec);

  /**
   * @brief Lock to access properties.
   * @param[in] (keys) Property keys for lock targets.
   * @param[in] (timeout_msec) Time of wait msec if locked already.
   *                           0 is polling, minus is forever.
   * @param[out] (lock_resource) Locked properties resource.
   * @return Status object.
   */
  virtual Status LockProperty(
      const std::set<std::string>& keys,
      int32_t timeout_msec,
      PropertyLockResource** lock_resource);

  /**
   * @brief Unlock to access properties.
   * @return Status object.
   */
  virtual Status UnlockProperty();

  /**
   * @brief Unlock to access properties.
   * @param[in] (lock_resource) Locked properties resource.
   * @return Status object.
   */
  virtual Status UnlockProperty(PropertyLockResource* lock_resource);

  /**
   * @brief Register the callback for frame reached.
   * @param[in] (callback) Function pointer.
   * @param[in] (private_data) Private data with callback.
   * @return Status object.
   */
  virtual Status RegisterFrameCallback(
    const OnFrameReceivedCallback callback,
    void* private_data);

  /**
   * @brief Unregister the callback for frame reached.
   * @return Status object.
   */
  virtual Status UnregisterFrameCallback();

  /**
   * @brief Register the callback for event receiving.
   * @param[in] (event_type) Event type to receive.
   * @param[in] (callback) Function pointer.
   * @param[in] (private_data) Private data with callback.
   * @return Status object.
   */
  virtual Status RegisterEventCallback(
    const std::string& event_type,
    const OnEventReceivedCallback callback,
    void* private_data);

  /**
   * @deprecated
   * @brief Register the callback for event receiving.
   * @param[in] (event_type) Event type to receive.
   * @param[in] (callback) Function pointer.
   * @param[in] (private_data) Private data with callback.
   * @return Status object.
   */
  virtual Status RegisterEventCallback(
    const std::string& event_type,
    const OnEventReceivedCallbackOld callback,
    void* private_data);

  /**
   * @brief Unregister the event callback.
   * @param[in] (event_type) Registered event type.
   * @return Status object.
   */
  virtual Status UnregisterEventCallback(const std::string& event_type);

  /**
   * @brief Get this stream's type.
   * @param[in] (property_key) Key of property to get.
   * @param[out] (property) The type of stream.
   * @return Status object.
   */
  Status Get(
    const std::string& property_key,
    StreamTypeProperty* property);

  /**
   * @brief Set this stream's type.
   * @return Status object.
   */
  Status Set(
    const std::string& /* property_key */,
    const StreamTypeProperty* /* property */);

  /**
   * @brief Get this stream's key.
   * @param[in] (property_key) Key of property to get.
   * @param[out] (property) The key of stream.
   * @return Status object.
   */
  Status Get(
    const std::string& property_key,
    StreamKeyProperty* property);

  /**
   * @brief Set this stream's key.
   * @return Status object.
   */
  Status Set(
    const std::string& /* property_key */,
    const StreamKeyProperty* /* property */);

  /**
   * @brief Get the current state of this stream.
   * @param[in] (property_key) Key of property to get.
   * @param[out] (property) The current state of stream.
   * @return Status object.
   */
  Status Get(
    const std::string& property_key,
    StreamStateProperty* property);

  /**
   * @brief Set the current state of this stream.
   * @return Status object.
   */
  Status Set(
    const std::string& /* property_key */,
    const StreamStateProperty* /* property */);

  /**
   * @brief Get this stream's frame buffering settings.
   * @param[in] (property_key) Key of property to get.
   * @param[out] (property) Frame buffering settings.
   * @return Status object.
   */
  Status Get(
    const std::string& property_key,
    FrameBuffering* property);

  /**
   * @brief Set this stream's frame buffering settings.
   * @return Status object.
   */
  Status Set(
    const std::string& /* property_key */,
    const FrameBuffering* /* property */);

  /**
   * @brief Get the current status of frame buffering.
   * @param[in] (property_key) Key of property to get.
   * @param[out] (property) Frame buffering current status.
   * @return Status object.
   */
  Status Get(
    const std::string& property_key,
    CurrentFrameNumProperty* property);

  /**
   * @brief Set the current status of frame buffering.
   * @return Status object.
   */
  Status Set(
    const std::string& /* property_key */,
    const CurrentFrameNumProperty* /* property */);

#ifdef SENSCORD_RECORDER
  /**
   * @brief Get the recording settings.
   * @param[in] (property_key) Key of property to get.
   * @param[out] (property) Current recording state.
   * @return Status object.
   */
  Status Get(
    const std::string& property_key,
    RecordProperty* property);

  /**
   * @brief Set the recording property.
   * @param[in] (property_key) Key of property to set.
   * @param[in] (property) New recording state.
   * @return Status object.
   */
  Status Set(
    const std::string& property_key,
    const RecordProperty* property);

  /**
   * @brief Get the recordable formats.
   * @param[in] (property_key) Key of property to get.
   * @param[out] (property) List of recordable formats.
   * @return Status object.
   */
  Status Get(
    const std::string& property_key,
    RecorderListProperty* property);

  /**
   * @brief Set the recordable formats.
   * @return Status object.
   */
  Status Set(
    const std::string& /* property_key */,
    const RecorderListProperty* /* property */);
#endif  // SENSCORD_RECORDER

  /**
   * @brief Get the current user data.
   * @param[in] (property_key) Key of property to get.
   * @param[out] (property) Current user data.
   * @return Status object.
   */
  Status Get(
    const std::string& property_key,
    UserDataProperty* property);

  /**
   * @brief Set the user data.
   * @param[in] (property_key) Key of property to set.
   * @param[in] (property) New user data.
   * @return Status object.
   */
  Status Set(
    const std::string& property_key,
    const UserDataProperty* property);

  /**
   * @brief Get the mask of the channels.
   * @param[in] (property_key) Key of property to get.
   * @param[out] (property) Current mask.
   * @return Status object.
   */
  Status Get(
    const std::string& property_key,
    ChannelMaskProperty* property);

  /**
   * @brief Set the mask of the channels.
   * @param[in] (property_key) Key of property to set.
   * @param[in] (property) New mask.
   * @return Status object.
   */
  Status Set(
    const std::string& property_key,
    const ChannelMaskProperty* property);

  /**
   * @brief Get the skip rate of the frame.
   * @param[in] (property_key) Key of property to get.
   * @param[out] (property) Current skip rate.
   * @return Status object.
   */
  Status Get(
    const std::string& property_key,
    SkipFrameProperty* property);

  /**
   * @brief Set the skip rate of the frame.
   * @param[in] (property_key) Key of property to set.
   * @param[in] (property) New skip rate.
   * @return Status object.
   */
  Status Set(
    const std::string& property_key,
    const SkipFrameProperty* property);

#ifdef SENSCORD_SERVER
  /**
   * @brief Get the frame extension.
   * @param[in] (property_key) Key of property to get.
   * @param[out] (property) Frame extension.
   * @return Status object.
   */
  Status Get(
    const std::string& property_key,
    FrameExtensionProperty* property);

  /**
   * @brief Set the frame extension.
   * @param[in] (property_key) Key of property to set.
   * @param[in] (property) Frame extension.
   * @return Status object.
   */
  Status Set(
    const std::string& property_key,
    const FrameExtensionProperty* property);
#endif

  /**
   * @brief Get the key of this stream.
   * @return Stream key.
   */
  const std::string& GetKey() const {
    return config_.stream_key;
  }

  /**
   * @brief Get the type of this stream.
   * @return Stream type.
   */
  const std::string& GetType() const {
    return config_.radical_address.port_type;
  }

  /**
   * @brief Get the initial setting of this stream relation.
   * @return Stream setting.
   */
  const StreamSetting& GetInitialSetting() const {
    return config_;
  }

  /**
   * @brief Gets the mutex to access the frame.
   */
  util::Mutex* GetFrameMutex() const {
    return &mutex_frame_;
  }

  /**
   * @brief Notify to release frame (for only FrameManager).
   * @param[in] (frameinfo) Frame informations to release.
   * @param[in] (referenced_channel_ids) List of referenced channel IDs.
   * @return Status object.
   */
  Status ReleaseFrameInfo(const FrameInfo& frameinfo,
                          const std::vector<uint32_t>& referenced_channel_ids);

  /**
   * @brief Send the multiple frames information from Component.
   * @param[in] (frames) List of frame information to send.
   * @param[in] (sent_time) Time when frame was sent.
   * @param[out] (dropped_frames) List of dropped frames.
   * @return Status object.
   */
  Status SendFrames(const std::vector<FrameInfo>& frames,
                    uint64_t sent_time,
                    std::vector<const FrameInfo*>* dropped_frames);

  /**
   * @brief Send the event from Component.
   * @param[in] (event) Event type to send.
   * @param[in] (args) Event argument.
   * @return Status object.
   */
  Status SendEvent(const std::string& event, const EventArgument& args);

  /**
   * @brief Notify frame arrived from FrameManager.
   * @param[in] (frameinfo) Arrived frame information.
   * @return Status object.
   */
  Status FrameArrived(const FrameInfo& frameinfo);

  /**
   * @brief Set user data to frame manager.
   * @param[in] (user_data) New user data.
   * @return Status object.
   */
  Status SetUserData(const FrameUserData& user_data);

  /**
   * @brief Get the accessor of frame property history book.
   * @return History book's address.
   */
  PropertyHistoryBook* GetPropertyHistoryBook() const {
    return history_book_;
  }

  /**
   * @brief Get the shared property accessor related this stream.
   * @param[in] (property_key) Search by property key.
   * @return Property accessor. null is not found.
   */
  PropertyAccessor* GetSharedPropertyAccessor(
      const std::string& property_key) const;

  /**
   * @brief Register the shared property accessor.
   * @param[in] (accessor) Property accessor to register.
   * @return Status object.
   */
  Status RegisterSharedPropertyAccessor(PropertyAccessor* accessor);

  /**
   * @brief Register the internal property accessor.
   * @param[in] (accessor) Property accessor to register.
   * @return Status object.
   */
  Status RegisterInternalPropertyAccessor(PropertyAccessor* accessor);

  /**
   * @brief Returns whether this this instance can be released.
   * @return true is releasable.
   */
  bool IsReleasable();

  /**
   * @brief Wait until this instance can be released.
   */
  void WaitForReleasable();

  /**
   * @brief Gets the resource list.
   */
  ResourceList* GetResources() { return &resources_; }

  /**
   * @brief Constructor
   */
  StreamCore();

  /**
   * @brief Destructor
   */
  virtual ~StreamCore();

 protected:
  /**
   * @brief Get local state.
   * @return Stream local state.
   */
  StreamLocalState GetLocalState() const;

  /**
   * @brief Get local state.
   * @param[out] (is_changing) State changing progress
   * @return Stream local state.
   */
  StreamLocalState GetLocalState(bool* is_changing) const;

  /**
   * @brief Begin the state change.
   * @param[in] (state) New state to set.
   * @return Status object.
   */
  Status BeginLocalStateChange(StreamLocalState state);

  /**
   * @brief Commit the state change.
   */
  void CommitLocalStateChange();

  /**
   * @brief Cancel the state change.
   */
  void CancelLocalStateChange();

  /**
   * @brief Wakeup wait frame.
   * @param[in] (mutex) Mutex object to lock.
   * @return true is successful, false is failed.
   */
  bool WakeupWaitFrame(util::Mutex* mutex);

  /**
   * @brief Get the internal property accessor related this stream.
   * @param[in] (property_key) Search by property key.
   * @return Property accessor. null is not found.
   */
  PropertyAccessor* GetInternalPropertyAccessor(
      const std::string& property_key) const;

  /**
   * @brief Start frame observing.
   * @return Status object.
   */
  Status StartFrameObserver();

  /**
   * @brief Stop frame observing.
   * @return Status object.
   */
  Status StopFrameObserver();

 private:
  typedef std::map<std::string, PropertyAccessor*> PropertyAccessorList;

  /**
   * @brief Judge the stream state is sendable the frame.
   * @return if sendable then return true.
   */
  bool IsSendableState() const;

  /**
   * @brief Create frame manager.
   * @param[in] (config) Frame buffering configuration.
   * @return Status object.
   */
  Status CreateFrameManager(const FrameBuffering& config);

  /**
   * @brief Destroy frame manager.
   * @return Status object.
   */
  Status DestroyFrameManager();

  /**
   * @brief Create frame observer by strategy.
   * @param[in] (strategy) Observing strategy.
   * @return New frame observer.
   */
  FrameObserver* CreateFrameObserver(CallbackStrategy strategy);

  /**
   * @brief Setup frame observer.
   * @param[in] (observer) Frame observer.
   * @param[in] (callback) Setup callback.
   * @param[in] (private_data) Setup private data.
   * @param[in] (is_running) Run new observer.
   * @return Status object.
   */
  Status SetupFrameObserver(FrameObserver* observer,
                            const OnFrameReceivedCallback callback,
                            void* private_data,
                            bool is_running);

  /**
   * @brief Create event observer by strategy.
   * @param[in] (strategy) Observing strategy.
   * @return New event observer.
   */
  EventObserver* CreateEventObserver(CallbackStrategy strategy);

  /**
   * @brief Register the event callback.
   * @param[in] (setup_param) Setup parameters.
   * @return Status object.
   */
  Status RegisterEventCallbackCore(
      const EventObserver::SetupParameter& setup_param);

  /**
   * @brief Unregister the event callback main process.
   * @param[in] (event_type) Registered event type.
   * @return Status object.
   */
  Status UnregisterEventCallbackCore(const std::string& event_type);

  /**
   * @brief Unregister the event callback of all.
   * @return Status object.
   */
  Status UnregisterEventCallbackAll();

  /**
   * @brief Stop main process.
   * @return Status object.
   */
  Status StopCore();

  /**
   * @brief Release the gotten frame.
   * @param[in] (frame) Received frame by GetFrame().
   * @param[in] (rawdata_accessed) Whether the raw data has been accessed.
   * @return Status object.
   */
  Status ReleaseFrameCore(Frame* frame, bool rawdata_accessed);

  /**
   * @brief Open the stream extension.
   * @return Status object.
   */
  Status OpenStreamExtension();

  /**
   * @brief Close the stream extension.
   * @param[in] (stop_on_error) If true, stop on error.
   * @return Status object.
   */
  Status CloseStreamExtension(bool stop_on_error);

  /**
   * @brief Get the allocators by stream extension
   * @param[in] (library_name) Stream extension library name.
   * @param[in] (allocators) Allocators.
   * @return Status object.
   */
  Status GetAllocatorsByStreamExtension(
      const std::string& library_name,
      std::map<std::string, MemoryAllocator*>* allocators);

  /**
   * @brief Get the arguments by stream extension
   * @param[in] (library_name) Stream extension library name.
   * @param[out] (arguments) Arguments.
   */
  void GetArgumentsByStreamExtension(
      const std::string& library_name,
      std::map<std::string, std::string>* arguments);

  // Stream configuration.
  StreamSetting config_;

  // Resource list.
  ResourceList resources_;

  // Frame manager.
  FrameManager* frame_manager_;

  // Stream recorder.
  FrameRecorder recorder_;

  // Component adapter.
  ComponentAdapter* adapter_;

  // Mutex object for access to frames.
  mutable util::Mutex mutex_frame_;

  // Mutex object for synchronize stream properties.
  mutable util::Mutex mutex_property_;

  // Mutex object for frame callback registering.
  util::Mutex mutex_callback_frame_;

  // Mutex object for frame callback registering.
  util::Mutex mutex_callback_event_;

  // Observer of frame receiving.
  FrameObserver* frame_observer_;
  util::ObservedSubject frame_notifier_;

  // Observer of event receiving.
  std::map<std::string, EventObserver*> event_observer_;
  util::ObservedSubject event_notifier_;

  // Condition variable to GetFrame.
  osal::OSCond* cond_frame_;

  // Properties list.
  PropertyAccessorList property_list_;  // internal
  PropertyAccessorList property_list_shared_;  // shared

  // Frame property history book.
  PropertyHistoryBook* history_book_;

  // Extensions.
  std::vector<StreamExtension*> stream_extensions_;
  FrameExtensionAdapter* frame_extension_;

  // Property locked resource.
  PropertyLockResource* lock_resource_;
};

}   // namespace senscord
#endif  // LIB_CORE_STREAM_STREAM_CORE_H_
