/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_SERIALIZE_SERIALIZE_UTILS_H_
#define LIB_CORE_SERIALIZE_SERIALIZE_UTILS_H_

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include "senscord/serialize.h"

namespace senscord {
namespace serialize {

/**
 * @brief Serialize to the binary array.
 * @param[in] (src) Source to serialize.
 * @param[out] (dest) Serialized binary array.
 * @return Status object.
 */
template <typename T>
Status SerializeToVector(const T* src, std::vector<uint8_t>* dest) {
  if (dest == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "dest is null");
  }
  SerializedBuffer buffer;
  Encoder encoder(&buffer);
  Status status = encoder.Push(*src);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  status = buffer.swap(dest);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Deserialize from the binary array.
 * @param[in] (src) Source binary vector to deserialize.
 * @param[out] (dest) Deserialized object.
 * @return Status object.
 */
template <typename T>
Status DeserializeFromVector(const std::vector<uint8_t>& src, T* dest) {
  if (dest == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "dest is null");
  }
  if (src.size() == 0) {
    return Status::OK();
  }
  Decoder decoder(&src[0], src.size());
  Status status = decoder.Pop(*dest);
  return SENSCORD_STATUS_TRACE(status);
}

}  // namespace serialize
}  // namespace senscord

#endif  // LIB_CORE_SERIALIZE_SERIALIZE_UTILS_H_
