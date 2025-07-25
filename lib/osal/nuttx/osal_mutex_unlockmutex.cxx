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
