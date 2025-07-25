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
 * @brief Wait for timeout or until notified. (specify relative time)
 * @param[in] cond   Condition variable object.
 * @param[in] mutex  Mutex object that is currently locked by this thread.
 * @param[in] nano_seconds  Timeout relative time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSRelativeTimedWaitCond(OSCond* cond, OSMutex* mutex,
                                uint64_t nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSRelativeTimedWaitCond;
  if (cond == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

#if 0  // TODO: NuttX 8.2 (CLOCK_REALTIME)
  struct timespec ts = {};
  int32_t result = clock_gettime(CLOCK_MONOTONIC, &ts);
  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  // relative time --> absolute time (CLOCK_MONOTONIC)
  uint64_t wait_time =
      (static_cast<uint64_t>(ts.tv_sec) * 1000 * 1000 * 1000) + ts.tv_nsec;
  wait_time += nano_seconds;

  lldiv_t ans = lldiv(wait_time, 1000 * 1000 * 1000);
  ts.tv_sec = ans.quot;
  ts.tv_nsec = ans.rem;

  result = pthread_cond_timedwait(
      reinterpret_cast<pthread_cond_t*>(cond),
      reinterpret_cast<pthread_mutex_t*>(mutex), &ts);

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    return OSMakeErrorCode(kFuncId, cause);
  }
#else
  uint64_t wait_time = 0;
  int32_t result = OSGetTime(&wait_time);
  if (result != 0) {
    SENSCORD_OSAL_LOG_ERROR("OSGetTime failed. ret=0x%" PRIx32, result);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }

  wait_time += nano_seconds;
  result = OSTimedWaitCond(cond, mutex, wait_time);
  if (result != 0) {
    OSErrorCause cause = OSGetErrorCause(result);
    return OSMakeErrorCode(kFuncId, cause);
  }
#endif

  return 0;
}

}  //  namespace osal
}  //  namespace senscord
