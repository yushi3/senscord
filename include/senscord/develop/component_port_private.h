/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_COMPONENT_PORT_PRIVATE_H_
#define SENSCORD_DEVELOP_COMPONENT_PORT_PRIVATE_H_

#include <stdint.h>
#include <string>

#include "senscord/config.h"
#include "senscord/serialize.h"
#include "senscord/develop/component_port.h"

namespace senscord {

#ifdef SENSCORD_SERIALIZE
/**
 * @brief Update frame channel property.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (property) Property to updated.
 * @return Status object.
 */
template <typename T>
Status ComponentPort::UpdateFrameProperty(uint32_t channel_id,
                                          const std::string& key,
                                          const T* property) {
  if (IsConnected()) {
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
  // not connected
  return SENSCORD_STATUS_FAIL(kStatusBlockCore,
      Status::kCauseInvalidOperation,
      "port is not connected");
}

/**
 * @brief Update frame channel property.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (property) Property to updated.
 * @return Status object.
 */
template <>
inline Status ComponentPort::UpdateFrameProperty(uint32_t channel_id,
    const std::string& key, const BinaryProperty* property) {
  if (IsConnected()) {
    const void* ptr = NULL;
    size_t size = 0;
    if ((property != NULL) && !property->data.empty()) {
      ptr = &property->data[0];
      size = property->data.size();
    }
    Status status = UpdateFrameSerializedProperty(channel_id, key, ptr, size);
    return SENSCORD_STATUS_TRACE(status);
  }
  // not connected
  return SENSCORD_STATUS_FAIL(kStatusBlockCore,
      Status::kCauseInvalidOperation,
      "port is not connected");
}
#else
/**
 * @brief Update frame channel property.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (property) Property to updated.
 * @param[in] (factory) Property factory.
 * @return Status object.
 */
template <typename T>
Status ComponentPort::UpdateFrameProperty(
    uint32_t channel_id, const std::string& key, const T* property) {
  PropertyFactory<T> factory;
  Status status = UpdateFramePropertyWithFactory(
      channel_id, key, property, factory);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_SERIALIZE

}   // namespace senscord
#endif  // SENSCORD_DEVELOP_COMPONENT_PORT_PRIVATE_H_
