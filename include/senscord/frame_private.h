/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_FRAME_PRIVATE_H_
#define SENSCORD_FRAME_PRIVATE_H_

#include <string>

#include "senscord/config.h"
#include "senscord/frame.h"
#include "senscord/serialize.h"

namespace senscord {

/**
 * @brief Get the property related to this raw data.
 * @param[in] (property_key) Key of property to get.
 * @param[out] (property) Location of property.
 * @return Status object.
 */
template <typename T>
Status Channel::GetProperty(const std::string& property_key,
                            T* property) const {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "property is null");
  }
  void* serialized = NULL;
  size_t serialized_size = 0;
  Status status = GetSerializedProperty(property_key, &serialized,
      &serialized_size);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    serialize::Decoder decoder(serialized, serialized_size);
    status = decoder.Pop(*property);
    SENSCORD_STATUS_TRACE(status);
  }
  return status;
}

/**
 * @brief Get the binary property related to this raw data.
 * @param[in] (property_key) Key of property to get.
 * @param[out] (property) Location of serialized property.
 * @return Status object.
 */
template <>
inline Status Channel::GetProperty(const std::string& property_key,
                                   BinaryProperty* property) const {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "property is null");
  }
  void* serialized = NULL;
  size_t serialized_size = 0;
  Status status = GetSerializedProperty(property_key, &serialized,
      &serialized_size);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    uint8_t* ptr = reinterpret_cast<uint8_t*>(serialized);
    property->data.reserve(serialized_size);
    property->data.assign(ptr, ptr + serialized_size);
  }
  return status;
}

}   // namespace senscord
#endif  // SENSCORD_FRAME_PRIVATE_H_
