/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_SERIALIZE_H_
#define SENSCORD_SERIALIZE_H_

#include <stdint.h>
#include <inttypes.h>

#include "senscord/config.h"
#include "senscord/serialize_define.h"
#include "senscord/serialize_buffer.h"
#include "senscord/property_types.h"
#include "senscord/rawdata_types.h"

#ifdef SENSCORD_SERIALIZE

#include "senscord/serialize_msgpack.h"

namespace senscord {
namespace serialize {

typedef MsgPackEncoder Encoder;
typedef MsgPackDecoder Decoder;

/**
 * @brief Copy memory.
 * @param[in,out] (dest) Destination area's top address.
 * @param[in] (dest_size) Destination area size.
 * @param[in] (source) Source area's top address.
 * @param[in] (source_size) Source area size.
 * @return 0 means success or negative value means failed (error code).
 */
extern int32_t Memcpy(void* dest, size_t dest_size, const void* source,
                      size_t source_size);

}  // namespace serialize
}  // namespace senscord

#else

namespace senscord {
namespace serialize {

class Encoder {
 public:
  explicit Encoder(Buffer* buffer) {}

  template<typename T>
  Status Push(const T& value) {
    return SENSCORD_STATUS_FAIL(
        senscord::kStatusBlockCore, senscord::Status::kCauseNotSupported,
        "feature is disabled. (SENSCORD_SERIALIZE=OFF)");
  }
};

class Decoder {
 public:
  explicit Decoder(const void* buffer, size_t size) {}

  template<typename T>
  Status Pop(T& value) {
    return SENSCORD_STATUS_FAIL(
        senscord::kStatusBlockCore, senscord::Status::kCauseNotSupported,
        "feature is disabled. (SENSCORD_SERIALIZE=OFF)");
  }

  size_t GetOffset() const { return 0; }
};

}  // namespace serialize
}  // namespace senscord

#endif  // SENSCORD_SERIALIZE

#endif  // SENSCORD_SERIALIZE_H_
