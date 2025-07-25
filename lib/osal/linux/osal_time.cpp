/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <time.h>
#include <errno.h>
#include "senscord/osal.h"
#include "common/osal_error.h"
#include "linux/osal_linuxerror.h"

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

  struct timespec ts;
  int32_t ret = clock_gettime(CLOCK_REALTIME, &ts);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  *nano_seconds = (static_cast<uint64_t>(ts.tv_sec) * 1000 * 1000 * 1000)
                 + ts.tv_nsec;

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

  struct timespec ts;
  int32_t ret = clock_gettime(CLOCK_REALTIME, &ts);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  struct tm time;
  if (localtime_r(&ts.tv_sec, &time) == NULL) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  current_time->year         = static_cast<uint16_t>(time.tm_year + 1900);
  current_time->month        = static_cast<uint8_t>(time.tm_mon + 1);
  current_time->day_of_week  = static_cast<uint8_t>(time.tm_wday);
  current_time->day          = static_cast<uint8_t>(time.tm_mday);
  current_time->hour         = static_cast<uint8_t>(time.tm_hour);
  current_time->minute       = static_cast<uint8_t>(time.tm_min);
  current_time->second       = static_cast<uint8_t>(time.tm_sec);
  current_time->milli_second = static_cast<uint16_t>(ts.tv_nsec / 1000000);

  return 0;
}

}  // namespace osal
}  // namespace senscord
