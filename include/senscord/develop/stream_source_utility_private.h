/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_STREAM_SOURCE_UTILITY_PRIVATE_H_
#define SENSCORD_DEVELOP_STREAM_SOURCE_UTILITY_PRIVATE_H_

#include <string>

#include "senscord/config.h"
#include "senscord/status.h"
#include "senscord/develop/stream_source_utility.h"
#include "senscord/develop/property_accessor.h"
#ifdef SENSCORD_SERIALIZE
#include "senscord/develop/deserialized_property_accessor.h"
#include "senscord/develop/serialized_property_accessor.h"
#endif  // SENSCORD_SERIALIZE

namespace senscord {

/**
 * @brief Register the new property.
 * @param[in] (key) The key of property.
 * @param[in] (source) The pointer of stream source.
 * @return Status object.
 */
template <typename T, class C>
Status StreamSourceUtility::CreateProperty(const std::string& key, C* source) {
#ifdef SENSCORD_SERIALIZE
  PropertyAccessor* accessor =
      new DeserializedPropertyAccessor<C, T>(key, source);
#else
  PropertyAccessor* accessor =
      new FastPropertyAccessor<C, T>(key, source);
#endif  // SENSCORD_SERIALIZE
  Status status = RegisterPropertyAccessor(accessor);
  if (!status.ok()) {
    delete accessor;
  }
  return SENSCORD_STATUS_TRACE(status);
}

#ifdef SENSCORD_SERIALIZE
/**
 * @brief Register the new property with BinaryProperty.
 * @param[in] (key) The key of property.
 * @param[in] (source) The pointer of stream source.
 * @return Status object.
 */
template <class C>
Status StreamSourceUtility::CreateProperty(const std::string& key, C* source) {
  PropertyAccessor* accessor =
      new SerializedPropertyAccessor<C>(key, source);
  Status status = RegisterPropertyAccessor(accessor);
  if (!status.ok()) {
    delete accessor;
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Update frame channel property.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (property) Property to updated.
 * @return Status object.
 */
template <typename T>
Status StreamSourceUtility::UpdateChannelProperty(uint32_t channel_id,
    const std::string& key, const T* property) {
  Status status;
  serialize::SerializedBuffer buffer;
  if (property != NULL) {
    serialize::Encoder encoder(&buffer);
    status = encoder.Push(*property);
    SENSCORD_STATUS_TRACE(status);
  }
  if (status.ok()) {
    status = UpdateFrameSerializedProperty(channel_id, key,
        buffer.data(), buffer.size());
    SENSCORD_STATUS_TRACE(status);
  }
  return status;
}

/**
 * @brief Update frame channel property with BinaryProperty.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) BinaryProperty key to updated.
 * @param[in] (property) BinaryProperty to updated.
 * @return Status object.
 */
template <>
inline Status StreamSourceUtility::UpdateChannelProperty(uint32_t channel_id,
    const std::string& key, const BinaryProperty* property) {
  Status status;
  const void* ptr = NULL;
  if (property != NULL && !property->data.empty()) {
    ptr = &property->data[0];
  }
  status = UpdateFrameSerializedProperty(
      channel_id, key, ptr, property->data.size());
  return SENSCORD_STATUS_TRACE(status);
}
#else
/**
 * @brief Update frame channel property.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (property) Property to updated.
 * @return Status object.
 */
template <typename T>
Status StreamSourceUtility::UpdateChannelProperty(
    uint32_t channel_id, const std::string& key, const T* property) {
  PropertyFactory<T> factory;
  Status status = UpdateFramePropertyWithFactory(
      channel_id, key, property, factory);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_SERIALIZE

}   // namespace senscord
#endif    // SENSCORD_DEVELOP_STREAM_SOURCE_UTILITY_PRIVATE_H_
