/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_SERIALIZE_MSGPACK_H_
#define SENSCORD_SERIALIZE_MSGPACK_H_

#include "senscord/config.h"

#ifdef SENSCORD_SERIALIZE

#include <stdint.h>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif  // __GNUC__
#include <msgpack.hpp>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif  // __GNUC__

#include "senscord/status.h"
#include "senscord/noncopyable.h"
#include "senscord/osal_inttypes.h"
#include "senscord/serialize_buffer.h"

namespace senscord {
namespace serialize {

/**
 * @brief MsgPack encoder class.
 */
class MsgPackEncoder : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   */
  explicit MsgPackEncoder(Buffer* buffer) : packer_(buffer), buffer_(buffer) {
  }

  /**
   * @brief Destructor.
   */
  ~MsgPackEncoder() {
  }

  /**
   * @brief Push the any object.
   * @param[in] value  Any variable.
   */
  template<typename T>
  Status Push(const T& value) {
    if (buffer_ == NULL) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidArgument,
          "buffer is null");
    }

    try {
      packer_.pack(value);
    } catch (const std::exception& e) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseAborted,
          "what=%s, buffer_size=%" PRIuS,
          e.what(), buffer_->size());
    }

    return Status::OK();
  }

 private:
  msgpack::packer<Buffer> packer_;
  Buffer* buffer_;
};

/**
 * @brief MsgPack decoder class.
 */
class MsgPackDecoder : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   */
  explicit MsgPackDecoder(const void* buffer, size_t size)
      : buffer_(buffer), size_(size), offset_() {
  }

  /**
   * @brief Destructor.
   */
  ~MsgPackDecoder() {
  }

  /**
   * @brief Pop the any object.
   * @param[out] value  Any variable.
   */
  template<typename T>
  Status Pop(T& value) {
    if (buffer_ == NULL) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidArgument,
          "buffer is null");
    }
    if (size_ <= offset_) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseOutOfRange,
          "end of buffer: size=%" PRIuS ", offset=%" PRIuS,
          size_, offset_);
    }

    size_t size = 0;
    try {
      msgpack::zone zone;
      msgpack::unpack(
          zone,
          reinterpret_cast<const char*>(buffer_) + offset_,
          size_ - offset_,
          size,
          AlwaysReference).convert(value);
      offset_ += size;
    } catch (const std::exception& e) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseAborted,
          "what=%s, buffer_size=%" PRIuS ", offset=%" PRIuS
          ", current=%" PRIuS "(0x%02" PRIx8 ")",
          e.what(), size_, offset_, offset_ + size,
          ((offset_ + size) >= size_) ? 0 :
              (*(reinterpret_cast<const uint8_t*>(buffer_) + offset_ + size)));
    }

    return Status::OK();
  }

  /**
   * @brief Get the offset.
   */
  size_t GetOffset() const { return offset_; }

 private:
  static bool AlwaysReference(
      msgpack::type::object_type type, size_t size, void* user_data) {
    return true;
  }

 private:
  const void* buffer_;
  const size_t size_;
  size_t offset_;
};

}  // namespace serialize
}  // namespace senscord

#endif  // SENSCORD_SERIALIZE
#endif  // SENSCORD_SERIALIZE_MSGPACK_H_
