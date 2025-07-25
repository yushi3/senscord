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
#include "darwin/osal_darwinerror.h"

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

  pthread_cond_t* cond_temporary = reinterpret_cast<pthread_cond_t*>(
      malloc(sizeof(pthread_cond_t)));
  if (cond_temporary == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorOutOfMemory);
  }

  int result = pthread_cond_init(cond_temporary, NULL);

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    free(cond_temporary);
    return OSMakeErrorCode(kFuncId, cause);
  }

  *cond = reinterpret_cast<OSCond*>(cond_temporary);

  return 0;
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

  return 0;
}

}  //  namespace osal
}  //  namespace senscord
