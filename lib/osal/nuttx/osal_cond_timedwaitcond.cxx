/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "senscord/osal.h"
#include "./osal_error.h"
#include "./osal_logger.h"
#include "./osal_linuxerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Wait for timeout or until notified. (specify absolute time)
 * @param[in] cond   Condition variable object.
 * @param[in] mutex  Mutex object that is currently locked by this thread.
 * @param[in] nano_seconds  Timeout absolute time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSTimedWaitCond(OSCond* cond, OSMutex* mutex,
                        uint64_t nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSTimedWaitCond;
  if (cond == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

#if 0  // TODO: NuttX 8.2 (CLOCK_REALTIME)
  uint64_t curr_time = 0;
  int32_t result = OSGetTime(&curr_time);
  if (result != 0) {
    SENSCORD_OSAL_LOG_ERROR("OSGetTime failed. ret=0x%" PRIx32, result);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }
  // absolute time --> relative time
  uint64_t relative_time = 0;
  if (nano_seconds > curr_time) {
    relative_time = nano_seconds - curr_time;
  }

  result = OSRelativeTimedWaitCond(cond, mutex, relative_time);
  if (result != 0) {
    OSErrorCause cause = OSGetErrorCause(result);
    return OSMakeErrorCode(kFuncId, cause);
  }
#else
  lldiv_t ans = lldiv(nano_seconds, 1000 * 1000 * 1000);
  struct timespec wait_time;
  wait_time.tv_sec = ans.quot;
  wait_time.tv_nsec = ans.rem;

  int result = pthread_cond_timedwait(
      reinterpret_cast<pthread_cond_t*>(cond),
      reinterpret_cast<pthread_mutex_t*>(mutex), &wait_time);

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    return OSMakeErrorCode(kFuncId, cause);
  }
#endif

  return 0;
}

}  //  namespace osal
}  //  namespace senscord
