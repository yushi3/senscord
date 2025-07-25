/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <stdlib.h>
#include <pthread.h>

#include "senscord/osal.h"
#include "./osal_error.h"
#include "./osal_logger.h"
#include "./osal_linuxerror.h"

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
#if 0  // TODO: NuttX 8.2 (CLOCK_REALTIME)
    result = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (result != 0) {
      OSErrorCause cause = GetErrorCauseFromErrno(result);
      result = OSMakeErrorCode(kFuncId, cause);
      break;
    }
#endif

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

}  //  namespace osal
}  //  namespace senscord
