/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_DESERIALIZED_PROPERTY_ACCESSOR_PRIVATE_H_
#define SENSCORD_DEVELOP_DESERIALIZED_PROPERTY_ACCESSOR_PRIVATE_H_

#include <stdint.h>
#include <string>

#include "senscord/config.h"
#include "senscord/serialize.h"
#include "senscord/develop/deserialized_property_accessor.h"

namespace senscord {

/**
 * @brief Constructor.
 * @param[in] (key) Key of property.
 * @param[out] (accessor) Owner of property accessors.
 */
template <class C, typename T>
DeserializedPropertyAccessor<C, T>::DeserializedPropertyAccessor(
    const std::string& key, C* accessor)
    : PropertyAccessor(key), accessor_(accessor) {
}

/**
 * @brief Set the serialized property.
 * @param[in] (key) Key of property.
 * @param[in] (serialized_property) Serialized property address.
 * @param[in] (serialized_size) Serialized property size.
 * @return Status object.
 */
template <class C, typename T>
Status DeserializedPropertyAccessor<C, T>::Set(
    const std::string& key,
    const void* serialized_property,
    size_t serialized_size) {
  Status status;

  if ((serialized_property != NULL) && (serialized_size != 0)) {
    // case of valid input data.
    T tmp = T();
    serialize::Decoder decoder(serialized_property, serialized_size);
    status = decoder.Pop(tmp);
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      // set property.
      status = accessor_->Set(key, &tmp);
      SENSCORD_STATUS_TRACE(status);
    }
  } else {
    // case of null pointer specified.
    status = accessor_->Set(key, reinterpret_cast<T*>(NULL));
    SENSCORD_STATUS_TRACE(status);
  }

  return status;
}

/**
 * @brief Get and create new serialized property.
 * @param[in] (key) Key of property.
 * @param[in] (serialized_input_property) Input serialized property address.
 * @param[in] (serialized_input_size) Input serialized property size.
 * @param[out] (serialized_property) New serialized property address.
 * @param[out] (serialized_size) Serialized property size.
 * @return Status object.
 */
template <class C, typename T>
Status DeserializedPropertyAccessor<C, T>::Get(
    const std::string& key,
    const void* serialized_input_property,
    size_t serialized_input_size,
    void** serialized_property,
    size_t* serialized_size) {
  if (serialized_property == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "serialized_property is null");
  }
  if (serialized_size == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "serialized_size is null");
  }
  Status status;
  T tmp = T();
  if (serialized_input_size == 0) {
    status = accessor_->Get(key, &tmp);
    SENSCORD_STATUS_TRACE(status);
  } else {
    if (serialized_input_property == NULL) {
      // no input
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument, "serialized_input_property is null");
    }

    // deserialize input.
    serialize::Decoder decoder(serialized_input_property,
                               serialized_input_size);
    status = decoder.Pop(tmp);
    SENSCORD_STATUS_TRACE(status);

    if (status.ok()) {
      // get property.
      status = accessor_->Get(key, &tmp);
      SENSCORD_STATUS_TRACE(status);
    }
  }

  if (status.ok()) {
    serialize::SerializedBuffer buffer;
    serialize::Encoder encoder(&buffer);
    status = encoder.Push(tmp);
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      const size_t buffer_size = buffer.size();
      if (buffer_size > 0) {
        uint8_t* new_buffer = new uint8_t[buffer_size];
        serialize::Memcpy(new_buffer, buffer_size, buffer.data(), buffer_size);
        *serialized_size = buffer_size;
        *serialized_property = new_buffer;
      } else {
        *serialized_size = 0;
        *serialized_property = NULL;
      }
    }
  }

  return status;
}

/**
 * @brief Release the serialized property.
 * @param[in] (key) Key of property.
 * @param[in] (serialized_property) Serialized property address by Get().
 * @param[in] (serialized_size) Serialized property size by Get().
 * @return Status object.
 */
template <class C, typename T>
Status DeserializedPropertyAccessor<C, T>::Release(
    const std::string& key,
    void* serialized_property,
    size_t serialized_size) {
  if ((serialized_property != NULL) && (serialized_size > 0)) {
    delete [] reinterpret_cast<uint8_t*>(serialized_property);
  }
  return Status::OK();
}

}   // namespace senscord
#endif  // SENSCORD_DEVELOP_DESERIALIZED_PROPERTY_ACCESSOR_PRIVATE_H_
