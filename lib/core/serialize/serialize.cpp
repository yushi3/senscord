/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/serialize.h"
#include "senscord/osal.h"

namespace senscord {
namespace serialize {

/**
 * @brief Copy memory.
 * @param[out] (dest) Destination area's top address.
 * @param[in] (dest_size) Destination area size.
 * @param[in] (source) Source area's top address.
 * @param[in] (source_size) Source area size.
 * @return 0 means success or negative value means failed (error code).
 */
int32_t Memcpy(void* dest, size_t dest_size, const void* source,
               size_t source_size) {
  return osal::OSMemcpy(dest, dest_size, source, source_size);
}

}   // namespace serialize
}  //  namespace senscord
