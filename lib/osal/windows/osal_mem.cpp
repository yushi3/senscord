/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "windows/osal_winerror.h"

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

/**
 * @brief Copy block of memory.
 * @param[out] dest      Pointer to the destination array where the content is
 *                       to be copied.
 * @param[in]  dest_size Size of the destination array.
 * @param[in]  source    Pointer to the source of data to be copied.
 * @param[in]  count     Number of bytes to copy.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSMemcpy(void* dest, size_t dest_size,
                 const void* source, size_t count) {
  static const OSFunctionId kFuncId = kIdOSMemcpy;
  if (dest == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (source == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (dest_size < count) {
    memcpy_s(dest, dest_size, source, dest_size);
    return OSMakeErrorCode(kFuncId, kErrorOutOfRange);
  }

  errno_t res = memcpy_s(dest, dest_size, source, count);
  if (res != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(res);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

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
    memmove_s(dest, dest_size, source, dest_size);
    return OSMakeErrorCode(kFuncId, kErrorOutOfRange);
  }

  errno_t res = memmove_s(dest, dest_size, source, count);
  if (res != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(res);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

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
