/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <inttypes.h>

#include "senscord/osal.h"
#include "./osal_error.h"
#include "./osal_logger.h"
#include "./osal_linuxerror.h"

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

}  //  namespace osal
}  //  namespace senscord
