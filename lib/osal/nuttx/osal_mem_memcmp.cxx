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
 * @brief Compare two blocks of memory.
 * @param[in] source1  Pointer to block of memory.
 * @param[in] source2  Pointer to block of memory.
 * @param[in] length   Length of bytes to compare.
 * @retval >0  source1 larger than source2.
 * @retval 0   source1 identical to source2.
 * @retval <0  source1 smaller than source2.
 */
int32_t OSMemcmp(const void* source1, const void* source2, size_t length) {
  if ((source1 == NULL) && (source2 == NULL)) {
    return 0;
  }
  if (source1 == NULL) {
    return -1;
  }
  if (source2 == NULL) {
    return 1;
  }
  int32_t ret = memcmp(source1, source2, length);
  return ret;
}

}  // namespace osal
}  // namespace senscord
