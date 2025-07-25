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
 * @brief Search character in block of memory.
 * @param[in] source  Pointer to the block of memory where the search is
 *                    performed.
 * @param[in] character  Character to be searched.
 * @param[in] length  Length of bytes to be analyzed.
 * @return A pointer to the first occurrence of value in the block of memory
 *         pointed by source.
 *         If the character is not found, the function returns a null pointer.
 */
const void* OSMemchr(const void* source, int32_t character, size_t length) {
  if (source == NULL) {
    return NULL;
  }
  const void* ptr = memchr(source, character, length);
  return ptr;
}

/**
 * @brief Search character in block of memory.
 * @param[in] source  Pointer to the block of memory where the search is
 *                    performed.
 * @param[in] character  Character to be searched.
 * @param[in] length  Length of bytes to be analyzed.
 * @return A pointer to the first occurrence of value in the block of memory
 *         pointed by source.
 *         If the character is not found, the function returns a null pointer.
 */
void* OSMemchr(void* source, int32_t character, size_t length) {
  if (source == NULL) {
    return NULL;
  }
  void* ptr = memchr(source, character, length);
  return ptr;
}

}  // namespace osal
}  // namespace senscord
