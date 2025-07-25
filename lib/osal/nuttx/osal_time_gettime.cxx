/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <time.h>
#include <errno.h>
#include "senscord/osal.h"
#include "./osal_error.h"
#include "./osal_linuxerror.h"

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

}  // namespace osal
}  // namespace senscord
