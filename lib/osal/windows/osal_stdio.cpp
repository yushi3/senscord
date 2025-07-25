/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "windows/osal_winerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Print formatted output to the standard output stream.
 * @param[in] format  Format string.
 * @param[in] ...     Optional arguments.
 * @return On success, the total number of written characters is returned.
 *         Negative value is fail. (error code)
 */
int32_t OSPrintf(const char* format, ...) {
  static const OSFunctionId kFuncId = kIdOSPrintf;
  if (format == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  va_list args;
  va_start(args, format);
  int32_t ret = vprintf_s(format, args);
  va_end(args);
  if (ret < 0) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  return ret;
}

/**
 * @brief Print formatted output specified by va_list to the standard output
 *        stream.
 * @param[in] format  Format string.
 * @param[in] args    List of arguments.
 * @return On success, the total number of written characters is returned.
 *         Negative value is fail. (error code)
 */
int32_t OSVprintf(const char* format, va_list args) {
  static const OSFunctionId kFuncId = kIdOSVprintf;
  if (format == NULL || args == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int32_t ret = vprintf_s(format, args);
  if (ret < 0) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  return ret;
}

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

  buffer[0] = '\0';
  errno = 0;
  int32_t ret = vsnprintf_s(buffer, size, _TRUNCATE, format, args);
  if (ret < 0) {
    errno_t error_val = errno;
    if (error_val != 0) {
      OSErrorCause cause = GetErrorCauseFromErrno(error_val);
      return OSMakeErrorCode(kFuncId, cause);
    }
    // total number of written characters.
    ret = static_cast<int32_t>(strlen(buffer));
  }

  return ret;
}

}  // namespace osal
}  // namespace senscord
