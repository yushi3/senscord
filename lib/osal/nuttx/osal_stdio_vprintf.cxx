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
 * @brief Print formatted output specified by va_list to the standard output
 *        stream.
 * @param[in] format  Format string.
 * @param[in] args    List of arguments.
 * @return On success, the total number of written characters is returned.
 *         Negative value is fail. (error code)
 */
int32_t OSVprintf(const char* format, va_list args) {
  static const OSFunctionId kFuncId = kIdOSVprintf;
  if (format == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int32_t ret = vprintf(format, args);
  if (ret < 0) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  return ret;
}

}  // namespace osal
}  // namespace senscord
