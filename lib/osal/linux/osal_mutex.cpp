/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
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
 * @brief Create a mutex object.
 * @param[out] mutex  Pointer to the variable that receives the mutex object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSCreateMutex(OSMutex** mutex) {
  static const OSFunctionId kFuncId = kIdOSCreateMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  pthread_mutexattr_t attribute;

  int result = pthread_mutexattr_init(&attribute);
  if (result != 0) {
    SENSCORD_OSAL_LOG_ERROR("mutexattr_init failed. ret=0x%" PRIx32,
        result);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }

  result = pthread_mutexattr_settype(&attribute, PTHREAD_MUTEX_RECURSIVE);
  if (result == 0) {
    pthread_mutex_t* mutex_temporary = reinterpret_cast<pthread_mutex_t*>(
        malloc(sizeof(pthread_mutex_t)));
    if (mutex_temporary == NULL) {
      result = OSMakeErrorCode(kFuncId, kErrorOutOfMemory);
    } else {
      result = pthread_mutex_init(mutex_temporary, &attribute);
      if (result == 0) {
        *mutex = reinterpret_cast<OSMutex*>(mutex_temporary);
      } else {
        OSErrorCause cause = GetErrorCauseFromErrno(result);
        free(mutex_temporary);
        result = OSMakeErrorCode(kFuncId, cause);
      }
    }
  } else {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    result = OSMakeErrorCode(kFuncId, cause);
  }

  pthread_mutexattr_destroy(&attribute);

  return result;
}

/**
 * @brief Destroy a mutex object.
 * @param[in] mutex  Mutex object to destroy.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDestroyMutex(OSMutex* mutex) {
  static const OSFunctionId kFuncId = kIdOSDestroyMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int result = pthread_mutex_destroy(
      reinterpret_cast<pthread_mutex_t*>(mutex));

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    return OSMakeErrorCode(kFuncId, cause);
  }

  free(mutex);

  return 0;
}

/**
 * @brief Lock a mutex.
 * @param[in] mutex  Mutex object to lock.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSLockMutex(OSMutex* mutex) {
  static const OSFunctionId kFuncId = kIdOSLockMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int result = pthread_mutex_lock(reinterpret_cast<pthread_mutex_t*>(mutex));

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

/**
 * @brief Lock a mutex. (specify relative time)
 * @param[in] mutex         Mutex object to lock.
 * @param[in] nano_seconds  Timeout relative time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSRelativeTimedLockMutex(OSMutex* mutex, uint64_t nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSRelativeTimedLockMutex;
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
  result = OSTimedLockMutex(mutex, wait_time);
  if (result != 0) {
    OSErrorCause cause = OSGetErrorCause(result);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

/**
 * @brief Try to lock a mutex.
 * @param[in] mutex  Mutex object to lock.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSTryLockMutex(OSMutex* mutex) {
  static const OSFunctionId kFuncId = kIdOSTryLockMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int result = pthread_mutex_trylock(
      reinterpret_cast<pthread_mutex_t*>(mutex));

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

/**
 * @brief Unlock a mutex.
 * @param[in] mutex  Mutex object to unlock.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSUnlockMutex(OSMutex* mutex) {
  static const OSFunctionId kFuncId = kIdOSUnlockMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int result = pthread_mutex_unlock(
      reinterpret_cast<pthread_mutex_t*>(mutex));

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

}  //  namespace osal
}  //  namespace senscord
