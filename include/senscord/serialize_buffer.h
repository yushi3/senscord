/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_SERIALIZE_BUFFER_H_
#define SENSCORD_SERIALIZE_BUFFER_H_

#include <stddef.h>
#include <stdint.h>
#include <vector>

#include "senscord/config.h"
#include "senscord/status.h"

namespace senscord {
namespace serialize {

/**
 * @brief Buffer interface for serialization.
 */
class Buffer {
 public:
  virtual ~Buffer() {}

  /**
   * @brief Write to the buffer.
   *
   * Do not change the function name and argument of write(),
   * as it matches the std::basic_ostream::write() function.
   *
   * @param[in] buffer  Pointer to the buffer to write.
   * @param[in] size    Number of bytes to write.
   */
  virtual Status write(const void* buffer, size_t size) = 0;

  // for msgpack 3.2.0 or later.
  virtual Status write(const char* buffer, size_t size) {
    return write(reinterpret_cast<const void*>(buffer), size);
  }

  /**
   * @brief Clear the buffer.
   */
  virtual Status clear() = 0;

  /**
   * @brief Get the buffer size.
   */
  virtual size_t size() const = 0;

  /**
   * @brief Get the buffer pointer.
   */
  virtual const uint8_t* data() const = 0;
};

/**
 * @brief A buffer that stores serialized data.
 */
class SerializedBuffer : public Buffer {
#ifdef SENSCORD_SERIALIZE
 private:
  static const size_t kDefaultReserveSize = 0x400;  // 1024 bytes.

 public:
  /**
   * @brief SerializedBuffer default constructor.
   */
  SerializedBuffer();

  /**
   * @brief SerializedBuffer constructor.
   * @param[in] reserve_size  Reserve buffer size.
   */
  explicit SerializedBuffer(size_t reserve_size);

  /**
   * @brief SerializedBuffer destructor.
   */
  virtual ~SerializedBuffer();

  /**
   * @brief Swap the buffer.
   * @param[in,out] buffer  Container to exchange contents.
   */
  Status swap(std::vector<uint8_t>* buffer);

  /**
   * @brief Write to the buffer.
   * @param[in] buffer  Pointer to the buffer to write.
   * @param[in] size    Number of bytes to write.
   */
  virtual Status write(const void* buffer, size_t size);

  /**
   * @brief Clear the buffer.
   */
  virtual Status clear() {
    buffer_.clear();
    return Status::OK();
  }

  /**
   * @brief Get the buffer size.
   */
  virtual size_t size() const { return buffer_.size(); }

  /**
   * @brief Get the buffer pointer.
   */
  virtual const uint8_t* data() const {
    return buffer_.empty() ? NULL : &buffer_[0];
  }

 private:
  std::vector<uint8_t> buffer_;
#else
 public:
  SerializedBuffer() {}
  explicit SerializedBuffer(size_t reserve_size) {}

  Status swap(std::vector<uint8_t>* buffer) {
    return SENSCORD_STATUS_FAIL(
        senscord::kStatusBlockCore, senscord::Status::kCauseNotSupported,
        "feature is disabled. (SENSCORD_SERIALIZE=OFF)");
  }
  virtual Status write(const void* buffer, size_t size) {
    return SENSCORD_STATUS_FAIL(
        senscord::kStatusBlockCore, senscord::Status::kCauseNotSupported,
        "feature is disabled. (SENSCORD_SERIALIZE=OFF)");
  }
  virtual Status clear() { return Status::OK(); }
  virtual size_t size() const { return 0; }
  virtual const uint8_t* data() const { return NULL; }
#endif  // SENSCORD_SERIALIZE
};

}  // namespace serialize
}  // namespace senscord

#endif  // SENSCORD_SERIALIZE_BUFFER_H_
