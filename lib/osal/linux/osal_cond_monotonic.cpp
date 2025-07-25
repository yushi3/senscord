/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"
#include "linux/osal_linuxerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Create a condition variable.
 * @param[out] cond  Pointer to the variable that receives the condition
 *                   variable.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSCreateCond(OSCond** cond) {
  static const OSFunctionId kFuncId = kIdOSCreateCond;
  if (cond == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  pthread_condattr_t attr;
  int32_t result = pthread_condattr_init(&attr);
  if (result != 0) {
    SENSCORD_OSAL_LOG_ERROR("condattr_init failed. ret=0x%" PRIx32, result);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }

  do {
    result = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (result != 0) {
      OSErrorCause cause = GetErrorCauseFromErrno(result);
      result = OSMakeErrorCode(kFuncId, cause);
      break;
    }

    pthread_cond_t* cond_temporary = reinterpret_cast<pthread_cond_t*>(
        malloc(sizeof(pthread_cond_t)));
    if (cond_temporary == NULL) {
      result = OSMakeErrorCode(kFuncId, kErrorOutOfMemory);
      break;
    }

    result = pthread_cond_init(cond_temporary, &attr);
    if (result != 0) {
      OSErrorCause cause = GetErrorCauseFromErrno(result);
      free(cond_temporary);
      result = OSMakeErrorCode(kFuncId, cause);
      break;
    }

    *cond = reinterpret_cast<OSCond*>(cond_temporary);
  } while (false);

  pthread_condattr_destroy(&attr);

  return result;
}

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

  return 0;
}

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

  return 0;
}

}  //  namespace osal
}  //  namespace senscord
