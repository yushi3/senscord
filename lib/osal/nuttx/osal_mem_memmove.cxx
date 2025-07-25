/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include "senscord/osal.h"
#include "./osal_error.h"

namespace senscord {
namespace osal {

/**
 * @brief Move block of memory.
 * @param[out] dest      Pointer to the destination array where the content is
 *                       to be copied.
 * @param[in]  dest_size Size of the destination array.
 * @param[in]  source    Pointer to the source of data to be copied.
 * @param[in]  count     Number of bytes to copy.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSMemmove(void* dest, size_t dest_size,
                  const void* source, size_t count) {
  static const OSFunctionId kFuncId = kIdOSMemmove;
  if (dest == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (source == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (dest_size < count) {
    memmove(dest, source, dest_size);
    return OSMakeErrorCode(kFuncId, kErrorOutOfRange);
  }
  // The return value of memmove is the same as the argument 'dest'.
  memmove(dest, source, count);
  return 0;
}

}  // namespace osal
}  // namespace senscord
