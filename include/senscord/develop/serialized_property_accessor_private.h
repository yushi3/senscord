/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_SERIALIZED_PROPERTY_ACCESSOR_PRIVATE_H_
#define SENSCORD_DEVELOP_SERIALIZED_PROPERTY_ACCESSOR_PRIVATE_H_

#include <stdint.h>
#include <string>

#include "senscord/config.h"
#include "senscord/serialize.h"
#include "senscord/property_types.h"
#include "senscord/develop/serialized_property_accessor.h"

namespace senscord {

/**
 * @brief Constructor.
 * @param[in] (key) Key of property.
 * @param[out] (accessor) Owner of property accessors.
 */
template <class C>
SerializedPropertyAccessor<C>::SerializedPropertyAccessor(
    const std::string& key, C* accessor)
    : PropertyAccessor(key), accessor_(accessor) {}

/**
 * @brief Set the serialized property.
 * @param[in] (key) Key of property.
 * @param[in] (serialized_property) Serialized property address.
 * @param[in] (serialized_size) Serialized property size.
 * @return Status object.
 */
template <class C>
Status SerializedPropertyAccessor<C>::Set(
    const std::string& key,
    const void* serialized_property,
    size_t serialized_size) {
  Status status;

  if ((serialized_property != NULL) && (serialized_size != 0)) {
    // case of valid input data.
    BinaryProperty tmp = {};
    tmp.data.reserve(serialized_size);
    tmp.data.assign(reinterpret_cast<const uint8_t*>(serialized_property),
        reinterpret_cast<const uint8_t*>(serialized_property) +
        serialized_size);

    // set property.
    status = accessor_->Set(key, const_cast<const BinaryProperty*>(&tmp));
    SENSCORD_STATUS_TRACE(status);
  } else {
    // case of null pointer specified.
    status = accessor_->Set(
        key, reinterpret_cast<const BinaryProperty*>(NULL));
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
template <class C>
Status SerializedPropertyAccessor<C>::Get(
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
  BinaryProperty tmp = {};
  if (serialized_input_size > 0) {
    if (serialized_input_property == NULL) {
      // no input
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument, "serialized_input_property is null");
    }

    // copy input property.
    tmp.data.reserve(serialized_input_size);
    tmp.data.assign(
        reinterpret_cast<const uint8_t*>(serialized_input_property),
        reinterpret_cast<const uint8_t*>(serialized_input_property)
        + serialized_input_size);
  }

  // get property.
  status = accessor_->Get(key, &tmp);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    // set to return value.
    const size_t buffer_size = tmp.data.size();
    if (buffer_size > 0) {
      uint8_t* new_buffer = new uint8_t[buffer_size];
      serialize::Memcpy(new_buffer, buffer_size, &tmp.data[0], buffer_size);
      *serialized_size = buffer_size;
      *serialized_property = new_buffer;
    } else {
      *serialized_size = 0;
      *serialized_property = NULL;
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
template <class C>
Status SerializedPropertyAccessor<C>::Release(
    const std::string& key,
    void* serialized_property,
    size_t serialized_size) {
  if ((serialized_property != NULL) && (serialized_size > 0)) {
    delete [] reinterpret_cast<uint8_t*>(serialized_property);
  }
  return Status::OK();
}

}   // namespace senscord
#endif  // SENSCORD_DEVELOP_SERIALIZED_PROPERTY_ACCESSOR_PRIVATE_H_
