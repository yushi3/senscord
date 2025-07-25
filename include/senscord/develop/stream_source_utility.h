/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_STREAM_SOURCE_UTILITY_H_
#define SENSCORD_DEVELOP_STREAM_SOURCE_UTILITY_H_

#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/event_argument.h"
#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/memory_allocator.h"
#include "senscord/develop/property_accessor.h"

namespace senscord {

/**
 * @brief The utility for the stream source's implementaion.
 */
class StreamSourceUtility : private util::Noncopyable {
 public:
  /**
   * @brief Get the key of stream.
   * @return The key of stream.
   */
  virtual const std::string& GetStreamKey() const = 0;

  /**
   * @brief Get the instance name of this component.
   * @return The name of instance.
   */
  virtual const std::string& GetInstanceName() const = 0;

  /**
   * @brief Get the argument value of instance by the name.
   * @param[in] (name) The argument name.
   * @param[out] (value) The argument value.
   * @return Status object.
   */
  virtual Status GetInstanceArgument(
    const std::string& name, std::string* value) const = 0;

  virtual Status GetInstanceArgument(
    const std::string& name, int64_t* value) const = 0;

  virtual Status GetInstanceArgument(
    const std::string& name, uint64_t* value) const = 0;

  /**
   * @brief Get the argument value of stream by the name.
   * @param[in] (name) The argument name.
   * @param[out] (value) The argument value.
   * @return Status object.
   */
  virtual Status GetStreamArgument(
    const std::string& name, std::string* value) const = 0;

  virtual Status GetStreamArgument(
    const std::string& name, int64_t* value) const = 0;

  virtual Status GetStreamArgument(
    const std::string& name, uint64_t* value) const = 0;

  /**
   * @brief Get the memory allocator by the name.
   * @param[in] (name) The memory allocator name.
   * @param[out] (allocator) The accessor of memory allocator.
   * @return Status object.
   */
  virtual Status GetAllocator(
    const std::string& name, MemoryAllocator** allocator) const = 0;

  /**
   * @brief Register the new property with deserialization.
   * @param[in] (key) The key of property.
   * @param[in] (source) The pointer of stream source.
   * @return Status object.
   */
  template <typename T, class C>
  Status CreateProperty(const std::string& key, C* source);

  /**
   * @def Macro for property registration
   * @param[in] (UTIL) The pointer of this class.
   * @param[in] (KEY) The key of property.
   * @param[in] (PROPERTY_TYPE) The type name of property.
   * @return Status object.
   */
  #define SENSCORD_REGISTER_PROPERTY(UTIL, KEY, PROPERTY_TYPE) \
    (UTIL)->CreateProperty<PROPERTY_TYPE>((KEY), this)

#ifdef SENSCORD_SERIALIZE
  /**
   * @brief Register the new property with BinaryProperty.
   * @param[in] (key) The key of property.
   * @param[in] (source) The pointer of stream source.
   * @return Status object.
   */
  template <class C>
  Status CreateProperty(const std::string& key, C* source);

  /**
   * @def Macro for un-deserialize property registration
   * @param[in] (UTIL) The pointer of this class.
   * @param[in] (KEY) The key of property.
   * @return Status object.
   */
  #define SENSCORD_REGISTER_SERIALIZED_PROPERTY(UTIL, KEY) \
    (UTIL)->CreateProperty((KEY), this)
#endif  // SENSCORD_SERIALIZE

  /**
   * @deprecated
   * @brief Send the event to the connected stream.
   * @param[in] (event_type) Event type to send.
   * @param[in] (reserved) Not used.
   * @return Status object.
   */
  Status SendEvent(
      const std::string& event_type, const void* reserved) {
    EventArgument args;  // empty args.
    Status status = SendEvent(event_type, args);
    return SENSCORD_STATUS_TRACE(status);
  }

  /**
   * @brief Send the event to the connected stream.
   * @param[in] (event_type) Event type to send.
   * @param[in] (args) Event argument.
   * @return Status object.
   */
  virtual Status SendEvent(
      const std::string& event_type, const EventArgument& args) = 0;

  /**
   * @brief Send the event for kEventError or kEventFatal.
   * @param[in] (error_status) Error status to send.
   * @return Status object.
   */
  virtual Status SendEventError(const Status& error_status) = 0;

  /**
   * @brief Send the event for kEventFrameDropped.
   * @param[in] (sequence_number) The sequence number that was dropped.
   * @return Status object.
   */
  virtual Status SendEventFrameDropped(uint64_t sequence_number) = 0;

  /**
   * @brief Send the event for kEventPropertyUpdated.
   * @param[in] (property_key) The key of the updated property.
   * @return Status object.
   */
  virtual Status SendEventPropertyUpdated(const std::string& property_key) = 0;

  /**
   * @brief Update frame channel property.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Property to updated.
   * @return Status object.
   */
  template <typename T>
  Status UpdateChannelProperty(uint32_t channel_id,
    const std::string& key, const T* property);

 protected:
  /**
   * @brief Register the new property accessor.
   * @param[in] (accessor) Property accessor.
   * @return Status object.
   */
  virtual Status RegisterPropertyAccessor(PropertyAccessor* accessor) = 0;

#ifdef SENSCORD_SERIALIZE
  /**
   * @brief Update the serialized property for frame channel.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Serialized property.
   * @param[in] (property_size) Serialized property size.
   * @return Status object.
   */
  virtual Status UpdateFrameSerializedProperty(uint32_t channel_id,
    const std::string& key, const void* property, size_t property_size) = 0;
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
      const PropertyFactoryBase& factory) = 0;
#endif  // SENSCORD_SERIALIZE

  /**
   * @brief Destructor
   */
  virtual ~StreamSourceUtility() {}
};

}   // namespace senscord

// implementations of template methods.
#include "senscord/develop/stream_source_utility_private.h"
#endif    // SENSCORD_DEVELOP_STREAM_SOURCE_UTILITY_H_
