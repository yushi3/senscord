/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_MESSENGER_PRIVATE_H_
#define SENSCORD_MESSENGER_PRIVATE_H_

#include <string>

#include "senscord/config.h"
#include "senscord/messenger.h"
#include "senscord/serialize.h"

namespace senscord {

/**
  * @brief Update frame channel property.
  * @param[in] (channel_id) Target channel ID.
  * @param[in] (key) Property key to updated.
  * @param[in] (property) Property to updated.
  * @return Status object.
  */
template <typename T>
Status Publisher::UpdateChannelProperty(
    uint32_t channel_id, const std::string& key, const T* property) {
  if (key.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "key is empty");
  }
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "property is NULL");
  }
  Status status;
#ifdef SENSCORD_SERIALIZE
  serialize::SerializedBuffer buffer;
  if (property != NULL) {
    serialize::Encoder encoder(&buffer);
    status = encoder.Push(*property);
    SENSCORD_STATUS_TRACE(status);
  }
  if (status.ok()) {
    status = SetUpdateChannelProperty(
        channel_id, key, buffer.data(), buffer.size());
    SENSCORD_STATUS_TRACE(status);
  }
#else  // SENSCORD_SERIALIZE
  PropertyFactory<T> factory;
  status = SetUpdateChannelProperty(channel_id, key, property, factory);
  SENSCORD_STATUS_TRACE(status);
#endif  // SENSCORD_SERIALIZE
  return status;
}

#ifdef SENSCORD_SERIALIZE
/**
  * @brief Update frame channel property.
  * @param[in] (channel_id) Target channel ID.
  * @param[in] (key) Property key to updated.
  * @param[in] (property) Property to updated.
  * @return Status object.
  */
template <>
inline Status Publisher::UpdateChannelProperty(
    uint32_t channel_id, const std::string& key,
    const BinaryProperty* property) {
  const void* ptr = NULL;
  size_t size = 0;
  if ((property != NULL) && !property->data.empty()) {
    ptr = &property->data[0];
    size = property->data.size();
  }
  Status status = SetUpdateChannelProperty(channel_id, key, ptr, size);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_SERIALIZE

}    // namespace senscord
#endif  // SENSCORD_MESSENGER_PRIVATE_H_
