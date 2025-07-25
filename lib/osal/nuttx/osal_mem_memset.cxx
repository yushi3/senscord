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
 * @brief Fill block of memory.
 * @param[out] buffer    Pointer to the block of memory to fill.
 * @param[in]  character Value to be set.
 * @param[in]  length    Number of bytes to be set.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSMemset(void* buffer, int32_t character, size_t length) {
  static const OSFunctionId kFuncId = kIdOSMemset;
  if (buffer == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  // The return value of memset is the same as the argument 'buffer'.
  memset(buffer, character, length);
  return 0;
}

}  // namespace osal
}  // namespace senscord
