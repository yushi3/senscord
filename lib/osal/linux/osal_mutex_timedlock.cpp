/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <inttypes.h>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"
#include "linux/osal_linuxerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Lock a mutex. (specify absolute time)
 * @param[in] mutex         Mutex object to lock.
 * @param[in] nano_seconds  Timeout absolute time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSTimedLockMutex(OSMutex* mutex, uint64_t nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSTimedLockMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  lldiv_t ans = lldiv(nano_seconds, 1000 * 1000 * 1000);
  struct timespec wait_time;
  wait_time.tv_sec = ans.quot;
  wait_time.tv_nsec = ans.rem;

  int result = pthread_mutex_timedlock(
      reinterpret_cast<pthread_mutex_t*>(mutex), &wait_time);

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

}  //  namespace osal
}  //  namespace senscord
