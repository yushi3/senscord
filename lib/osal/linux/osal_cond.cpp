/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
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
 * @brief Destroy a condition variable.
 * @param[in] cond  Condition variable object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDestroyCond(OSCond* cond) {
  static const OSFunctionId kFuncId = kIdOSDestroyCond;
  if (cond == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int result = pthread_cond_destroy(
      reinterpret_cast<pthread_cond_t*>(cond));

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    return OSMakeErrorCode(kFuncId, cause);
  }

  free(cond);

  return 0;
}

/**
 * @brief Wait until notified.
 * @param[in] cond   Condition variable object.
 * @param[in] mutex  Mutex object that is currently locked by this thread.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSWaitCond(OSCond* cond, OSMutex* mutex) {
  static const OSFunctionId kFuncId = kIdOSWaitCond;
  if (cond == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int result = pthread_cond_wait(
      reinterpret_cast<pthread_cond_t*>(cond),
      reinterpret_cast<pthread_mutex_t*>(mutex));

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

/**
 * @brief Unblocks one of the threads waiting for the condition variable.
 * @param[in] cond  Condition variable object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSignalCond(OSCond* cond) {
  static const OSFunctionId kFuncId = kIdOSSignalCond;
  if (cond == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int result = pthread_cond_signal(reinterpret_cast<pthread_cond_t*>(cond));

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

/**
 * @brief Unblocks all threads waiting for the condition variable.
 * @param[in] cond  Condition variable object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSBroadcastCond(OSCond* cond) {
  static const OSFunctionId kFuncId = kIdOSBroadcastCond;
  if (cond == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int result = pthread_cond_broadcast(
      reinterpret_cast<pthread_cond_t*>(cond));

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

}  //  namespace osal
}  //  namespace senscord
