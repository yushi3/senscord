/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>  /* std::max */
#include "senscord/osal.h"
#include "common/osal_error.h"
#include "linux/osal_linuxerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Convert a string to a 64-bit signed integer.
 * @param[in]  target_string  String to convert.
 * @param[out] end_string     Pointer to the variable that receives the stop
 *                            position.
 * @param[in]  radix          Radix to use.
 * @param[out] convert_value  Pointer to the variable that receives the
 *                            converted number.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorOutOfRange When the low-order 1 byte is kErrorOutOfRange,
 *                        it means overflow or underflow.
 */
int32_t OSStrtoll(const char* target_string,
                  char** end_string, uint8_t radix, int64_t* convert_value) {
  static const OSFunctionId kFuncId = kIdOSStrtoll;
  if (target_string == NULL || convert_value == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  if (!(radix == kOSRadixAuto
    || (kOSRadixMin <= radix && radix <= kOSRadixMax))) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  errno = 0;
  *convert_value = strtoll(target_string, end_string, radix);

  if (errno != 0 && errno != EINVAL) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

/**
 * @brief Convert a string to a 64-bit unsigned integer.
 * @param[in]  target_string  String to convert.
 * @param[out] end_string     Pointer to the variable that receives the stop
 *                            position.
 * @param[in]  radix          Radix to use.
 * @param[out] convert_value  Pointer to the variable that receives the
 *                            converted number.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorOutOfRange When the low-order 1 byte is kErrorOutOfRange,
 *                        it means overflow.
 */
int32_t OSStrtoull(const char* target_string,
                   char** end_string, uint8_t radix, uint64_t* convert_value) {
  static const OSFunctionId kFuncId = kIdOSStrtoull;
  if (target_string == NULL || convert_value == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  if (!(radix == kOSRadixAuto
    || (kOSRadixMin <= radix && radix <= kOSRadixMax))) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  errno = 0;
  *convert_value = strtoull(target_string, end_string, radix);

  if (errno != 0 && errno != EINVAL) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

}  // namespace osal
}  // namespace senscord
