/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include "senscord/serialize_buffer.h"

namespace senscord {
namespace serialize {

/**
 * @brief SerializedBuffer default constructor.
 */
SerializedBuffer::SerializedBuffer() : buffer_() {
  buffer_.reserve(kDefaultReserveSize);
}

/**
 * @brief SerializedBuffer constructor.
 * @param[in] reserve_size  Reserve buffer size.
 */
SerializedBuffer::SerializedBuffer(size_t reserve_size) : buffer_() {
  buffer_.reserve(reserve_size);
}

/**
 * @brief SerializedBuffer destructor.
 */
SerializedBuffer::~SerializedBuffer() {
}

/**
 * @brief Swap the buffer.
 * @param[in,out] buffer  Container to exchange contents.
 */
Status SerializedBuffer::swap(std::vector<uint8_t>* buffer) {
  if (buffer == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "buffer is NULL");
  }
  buffer_.swap(*buffer);
  return Status::OK();
}

/**
 * @brief Write to the buffer.
 * @param[in] buffer  Pointer to the buffer to write.
 * @param[in] size    Number of bytes to write.
 */
Status SerializedBuffer::write(const void* buffer, size_t size) {
  if (size == 0) {
    return Status::OK();
  }
  if (buffer == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "buffer is NULL");
  }
  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buffer);
  buffer_.insert(buffer_.end(), ptr, ptr + size);
  return Status::OK();
}

}  // namespace serialize
}  // namespace senscord
