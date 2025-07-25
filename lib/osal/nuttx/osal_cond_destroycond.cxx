/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <pthread.h>

#include "senscord/osal.h"
#include "./osal_error.h"
#include "./osal_logger.h"
#include "./osal_linuxerror.h"

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

}  //  namespace osal
}  //  namespace senscord
