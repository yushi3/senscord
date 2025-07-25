/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_MESSAGE_PACK_COMMON_CLASS_H_
#define LIB_MESSAGE_PACK_COMMON_CLASS_H_

#include <inttypes.h>
#include <string>
#include <vector>

#include "senscord/status.h"
#include "senscord/serialize.h"
#include "senscord/connection_types.h"
#include "../ws_log_macro.h"

namespace senscord {

/**
 * @brief The component type of message pack.
 */
enum ComponentType {
  kComponentTypeProperty = 1,
  kComponentTypeFrame,
};

/**
 * @brief Property data to Binary.
 * @param[in] (property) Property data.
 * @param[out] (dst) Binary data by Messagepack.
 * @return Status object.
 */
template <typename T>
Status SerializeToVector(T* property, std::vector<uint8_t>* dst) {
  senscord::serialize::SerializedBuffer buffer;
  senscord::serialize::Encoder encoder(&buffer);
  Status status = encoder.Push(*property);
  if (!status.ok()) {
    LOG_E("[Error] Encoder.Push(Property): ret=%s\n",
        status.ToString().c_str());
  } else {
    dst->assign(buffer.data(), buffer.data() + buffer.size());
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Binary to Property data.
 * @param[in] (src) Binary data by Messagepack.
 * @param[out] (property) Property data.
 * @return Status object.
 */
template <typename T>
Status DeserializeFromVector(std::vector<uint8_t>& src, T* property) {
  senscord::serialize::Decoder decoder(src.data(), src.size());
  Status status = decoder.Pop(*property);
  if (!status.ok()) {
    LOG_E("[Error] Decoder.Pop(Property): ret=%s\n",
        status.ToString().c_str());
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Property data to Binary.
 * @param[in] (property) Property data.
 * @param[out] (dst) Binary data by Messagepack.
 * @return Status object.
 */
template <typename T>
Status PropertyToBinary(T &property, std::vector<uint8_t>* dst) {
  return SENSCORD_STATUS_TRACE(SerializeToVector(&property, dst));
}

/**
 * @brief Binary to Property data.
 * @param[in] (src) Binary data by Messagepack.
 * @param[out] (property) Property data.
 * @return Status object.
 */
template <typename T>
Status BinaryToProperty(std::vector<uint8_t>* src, T &property) {
  return SENSCORD_STATUS_TRACE(DeserializeFromVector(*src, &property));
}

/**
 * @brief Message data to Binary.
 * @param[in] (msg) Message data.
 * @param[out] (vect) Binary data by Messagepack.
 * @return Status object.
 */
template <typename T>
void SerializeMsg(T* msg, std::vector<uint8_t> *vect) {
  SerializeToVector(msg, vect);
}

/**
 * @brief Binary to Message data.
 * @param[in] (payload) Binary data by Messagepack.
 * @param[in] (len) Binary data by Messagepack.
 * @param[out] (data) Message data.
 * @return Status object.
 */
template <typename T>
Status DeserializeMsg(char* payload, int len, T* data) {
  senscord::serialize::Decoder decoder(payload, len);
  Status status = decoder.Pop(*data);
  if (!status.ok()) {
    LOG_E("[Error] Decoder.Pop(Property): ret=%s\n",
        status.ToString().c_str());
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Binary to Message data.
 * @param[in] (payload) Binary data by Messagepack.
 * @param[in] (len) Binary data by Messagepack.
 * @param[out] (data) Message data.
 * @return Status object.
 */
template <typename T>
Status DeserializeMsg(char* payload, size_t len, T* data) {
  return SENSCORD_STATUS_TRACE(
      DeserializeMsg(payload, static_cast<int>(len), data));
}

}  // namespace senscord

#endif  // LIB_MESSAGE_PACK_COMMON_CLASS_H_
