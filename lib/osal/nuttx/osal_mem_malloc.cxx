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
 * @brief Allocate memory block.
 * @param[in] length  Length of the memory block, in bytes.
 * @return On success, it returns a pointer to the memory block.
 *         If it failed, it returns a null pointer.
 */
void* OSMalloc(size_t length) {
  if (length == 0) {
    return NULL;
  }
  void* ptr = malloc(length);
  return ptr;
}

}  // namespace osal
}  // namespace senscord
