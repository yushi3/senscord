/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/timeb.h>
#include <windows.h>
#include "senscord/osal.h"
#include "common/osal_error.h"
#include "windows/osal_winerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Get current time.
 * Time in nanoseconds since the epoch time(1970-1-1 00:00:00 UTC).
 * @param[out] nano_seconds  Pointer to the variable that receives the current
 *                           time. (in nanoseconds)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetTime(uint64_t* nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSGetTime;
  if (nano_seconds == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  __timeb64 time;
  errno_t result = _ftime64_s(&time);

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    return OSMakeErrorCode(kFuncId, cause);
  }

  // Not supported in less than milliseconds
  *nano_seconds = (static_cast<uint64_t>(time.time) * 1000 * 1000 * 1000)
                + (static_cast<uint64_t>(time.millitm) * 1000 * 1000);

  return 0;
}

/**
 * @brief Get current time.
 * The function corrects for the timezone.
 * @param[out] current_time  Pointer to the variable that receives the current
 *                           time. (OSSystemTime)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetLocalTime(OSSystemTime* current_time) {
  static const OSFunctionId kFuncId = kIdOSGetLocalTime;
  if (current_time == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  SYSTEMTIME time;
  GetLocalTime(&time);

  current_time->year         = static_cast<uint16_t>(time.wYear);
  current_time->month        = static_cast<uint8_t>(time.wMonth);
  current_time->day_of_week  = static_cast<uint8_t>(time.wDayOfWeek);
  current_time->day          = static_cast<uint8_t>(time.wDay);
  current_time->hour         = static_cast<uint8_t>(time.wHour);
  current_time->minute       = static_cast<uint8_t>(time.wMinute);
  current_time->second       = static_cast<uint8_t>(time.wSecond);
  current_time->milli_second = static_cast<uint16_t>(time.wMilliseconds);

  return 0;
}

}  // namespace osal
}  // namespace senscord
