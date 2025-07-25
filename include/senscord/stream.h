/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_STREAM_H_
#define SENSCORD_STREAM_H_

#include <stdint.h>
#include <vector>
#include <string>
#include <map>
#include <set>

#include "senscord/config.h"
#include "senscord/senscord_types.h"
#include "senscord/status.h"
#include "senscord/frame.h"
#include "senscord/noncopyable.h"
#include "senscord/event_argument.h"

namespace senscord {

// internal class
class StreamFunctionLockManager;

// timeout denfinitions
const int32_t kTimeoutPolling = 0;
const int32_t kTimeoutForever = -1;

// property lock resource
struct PropertyLockResource;

/**
 * @brief Stream interface class.
 */
class Stream : private util::Noncopyable {
 public:
  /**
   * @brief Frame received callback type
   */
  typedef void (* OnFrameReceivedCallback)(
      Stream* stream, void* private_data);

  /**
   * @brief Event received callback type
   */
  typedef void (* OnEventReceivedCallback)(
      Stream* stream, const std::string& event_type,
      const EventArgument& args, void* private_data);

  /**
   * @deprecated
   * @brief Event received callback type
   */
  typedef void (* OnEventReceivedCallbackOld)(
      const std::string& event_type, const void* reserved, void* private_data);

  /**
   * @brief Start this stream.
   * @return Status object.
   */
  virtual Status Start() = 0;

  /**
   * @brief Stop this stream.
   * @return Status object.
   */
  virtual Status Stop() = 0;

  /**
   * @brief Get the received frame.
   * @param[out] (frame) Location of received frame.
   * @param[in] (timeout_msec) Time of wait msec if no received.
   *                           0 is polling, minus is forever.
   * @return Status object.
   */
  virtual Status GetFrame(Frame** frame, int32_t timeout_msec) = 0;

  /**
   * @brief Release the gotten frame.
   * @param[in] (frame) Received frame by GetFrame().
   * @return Status object.
   */
  virtual Status ReleaseFrame(Frame* frame) = 0;

  /**
   * @brief Release the gotten frame.
   *
   * Use this function if you do not refer to the raw data of the channel.
   *
   * @param[in] (frame) Received frame by GetFrame().
   * @return Status object.
   */
  virtual Status ReleaseFrameUnused(Frame* frame) = 0;

  /**
   * @brief Clear frames have not gotten.
   * @param[out] (frame_number) number of cleared frames.
   * @return Status object.
   */
  virtual Status ClearFrames(int32_t* frame_number) = 0;

  /**
   * @brief Get the property.
   * @param[in] (property_key) Key of property to get.
   * @param[out] (property) Location of property.
   * @return Status object.
   */
#ifdef SENSCORD_SERIALIZE
  template <typename T>
  Status GetProperty(const std::string& property_key, T* property);
#else
  virtual Status GetProperty(
      const std::string& property_key, void* property) = 0;
#endif  // SENSCORD_SERIALIZE

  /**
   * @brief Set the property with key.
   * @param[in] (property_key) Key of property to set.
   * @param[in] (property) Location of property.
   * @return Status object.
   */
#ifdef SENSCORD_SERIALIZE
  template <typename T>
  Status SetProperty(const std::string& property_key, const T* property);
#else
  virtual Status SetProperty(
      const std::string& property_key, const void* property) = 0;
#endif  // SENSCORD_SERIALIZE

  /**
   * @brief Get the supported property key list on this stream.
   * @param[out] (key_list) Supported property key list.
   * @return Status object.
   */
  virtual Status GetPropertyList(
    std::vector<std::string>* key_list) const = 0;

  /**
   * @brief Lock to access properties.
   * @param[in] (timeout_msec) Time of wait msec if locked already.
   *                           0 is polling, minus is forever.
   * @return Status object.
   */
  virtual Status LockProperty(int32_t timeout_msec) = 0;

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
      PropertyLockResource** lock_resource) = 0;

  /**
   * @brief Unlock to access properties.
   * @return Status object.
   */
  virtual Status UnlockProperty() = 0;

  /**
   * @brief Unlock to access properties.
   * @param[in] (lock_resource) Locked properties resource.
   * @return Status object.
   */
  virtual Status UnlockProperty(PropertyLockResource* lock_resource) = 0;

  /**
   * @brief Register the callback for frame reached.
   * @param[in] (callback) Function pointer.
   * @param[in] (private_data) Private data with callback.
   * @return Status object.
   */
  virtual Status RegisterFrameCallback(
    const OnFrameReceivedCallback callback,
    void* private_data) = 0;

  /**
   * @brief Unregister the callback for frame reached.
   * @return Status object.
   */
  virtual Status UnregisterFrameCallback() = 0;

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
    void* private_data) = 0;

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
    void* private_data) = 0;

  /**
   * @brief Unregister the event callback.
   * @param[in] (event_type) Registered event type.
   * @return Status object.
   */
  virtual Status UnregisterEventCallback(const std::string& event_type) = 0;

  /**
   * @brief Virtual destructor.
   */
  virtual ~Stream() {}

#ifdef SENSCORD_SERIALIZE
 protected:
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
    size_t* output_property_size) = 0;

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
    size_t property_size) = 0;

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
    size_t property_size) = 0;
#endif  // SENSCORD_SERIALIZE

 protected:
    // Exclusive lock manager
    StreamFunctionLockManager* lock_manager_;
};

}  // namespace senscord

// implementations of template methods.
#include "senscord/stream_private.h"
#endif  // SENSCORD_STREAM_H_
