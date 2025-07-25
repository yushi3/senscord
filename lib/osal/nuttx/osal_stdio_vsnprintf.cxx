/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdarg.h>

#include "senscord/osal.h"
#include "./osal_error.h"

namespace senscord {
namespace osal {

/**
 * @brief Outputs the converted character string to the buffer.
 *
 * For the following parameters, an error occurs.
 *   buffer == NULL : kErrorInvalidArgument
 *   format == NULL : kErrorInvalidArgument
 *   size   == 0    : kErrorInvalidArgument
 *
 * If the length of the converted string is greater than the buffer size,
 * the string is truncated and the return value is the total number of
 * written characters.
 *
 * @param[out] buffer  Pointer to the buffer that stores the converted
 *                     character string.
 * @param[in]  size    Size of buffer.
 * @param[in]  format  Format string.
 * @param[in]  args    List of arguments.
 * @return On success, the total number of written characters is returned.
 *         Negative value is fail. (error code)
 */
int32_t OSVsnprintf(char* buffer, size_t size,
                    const char* format, va_list args) {
  static const OSFunctionId kFuncId = kIdOSVsnprintf;
  if (buffer == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (format == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (size == 0) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int32_t ret = vsnprintf(buffer, size, format, args);
  if (ret < 0) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t max_size = static_cast<int32_t>(size) - 1;
  if (ret > max_size) {
    // total number of written characters.
    ret = max_size;
  }
  return ret;
}

}  // namespace osal
}  // namespace senscord
