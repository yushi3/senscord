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
 * @brief Deallocate memory block.
 * @param[in] ptr Pointer to a memory block previously allocated with OSMalloc.
 * @return Always returns zero.
 */
int32_t OSFree(void* ptr) {
  if (ptr == NULL) {
    return 0;
  }
  free(ptr);
  return 0;
}

}  // namespace osal
}  // namespace senscord
