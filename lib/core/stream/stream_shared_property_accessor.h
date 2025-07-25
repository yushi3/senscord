/*
 * SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_STREAM_STREAM_SHARED_PROPERTY_ACCESSOR_H_
#define LIB_CORE_STREAM_STREAM_SHARED_PROPERTY_ACCESSOR_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/senscord_types.h"
#include "senscord/develop/property_accessor.h"
#ifdef SENSCORD_SERIALIZE
#include "senscord/develop/deserialized_property_accessor.h"
#endif  // SENSCORD_SERIALIZE

namespace senscord {

#ifdef SENSCORD_SERIALIZE

/**
 * @brief Property accessor for stream shared property.
 */
template<typename C, typename T>
class StreamSharedPropertyAccessor :
    public DeserializedPropertyAccessor<C, T> {
 public:
  /**
   * @brief Constructor.
   * @param[in] (key) Key of property.
   * @param[in] (stream) Parent stream.
   */
  explicit StreamSharedPropertyAccessor(const std::string& key, C* stream) :
      DeserializedPropertyAccessor<C, T>(key, stream),
      stream_(stream) {}


  /**
   * @brief Destructor.
   */
  virtual ~StreamSharedPropertyAccessor() {}

  /**
   * @brief Set the serialized property.
   * @param[in] (key) Key of property.
   * @param[in] (serialized_property) Serialized property address.
   * @param[in] (serialized_size) Serialized property size.
   * @return Status object.
   */
  virtual Status Set(
      const std::string& key,
      const void* serialized_property,
      size_t serialized_size);

 private:
  C* stream_;
};

/**
 * @brief Set the serialized property.
 * @param[in] (key) Key of property.
 * @param[in] (serialized_property) Serialized property address.
 * @param[in] (serialized_size) Serialized property size.
 * @return Status object.
 */
template<typename C, typename T>
Status StreamSharedPropertyAccessor<C, T>::Set(
    const std::string& key,
    const void* serialized_property,
    size_t serialized_size) {
  Status status = DeserializedPropertyAccessor<C, T>::Set(
      key, serialized_property, serialized_size);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Send PropertyUpdated event.
  EventArgument args;
  status = args.Set(kEventArgumentPropertyKey, key);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  status = stream_->SendEvent(kEventPropertyUpdated, args);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return Status::OK();
}

#else

template<typename C, typename T>
class StreamSharedPropertyAccessor : public FastPropertyAccessor<C, T> {
 public:
  /**
   * @brief Constructor.
   * @param[in] (key) Key of property.
   * @param[in] (stream) Parent stream.
   */
  explicit StreamSharedPropertyAccessor(const std::string& key, C* stream) :
      FastPropertyAccessor<C, T>(key, stream), stream_(stream) {}

  /**
   * @brief Destructor.
   */
  virtual ~StreamSharedPropertyAccessor() {}

  /**
   * @brief Set the property.
   * @param[in] (key) Key of property.
   * @param[in] (property) Property address.
   * @return Status object.
   */
  virtual Status Set(const std::string& key, const void* property) {
    const T* tmp = reinterpret_cast<const T*>(property);
    Status status = FastPropertyAccessor<C, T>::Set(key, tmp);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    // Send PropertyUpdated event.
    EventArgument args;
    status = stream_->SendEvent(kEventPropertyUpdated, args);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    return Status::OK();
  }

 private:
  C* stream_;
};

#endif  // SENSCORD_SERIALIZE

}   // namespace senscord

#endif  // LIB_CORE_STREAM_STREAM_SHARED_PROPERTY_ACCESSOR_H_
