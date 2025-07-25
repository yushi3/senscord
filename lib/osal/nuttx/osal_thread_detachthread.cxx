/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include "senscord/osal.h"
#include "./osal_thread.h"
#include "./osal_error.h"
#include "./osal_logger.h"
#include "./osal_linuxerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Detach a thread.
 * @param[in] thread  Thread object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDetachThread(OSThread* thread) {
  static const OSFunctionId kFuncId = kIdOSDetachThread;
  if (thread == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorNotFound);
  }
  pthread_t thread_id = GetThreadId(thread);

  int32_t ret = pthread_detach(thread_id);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(ret);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

}  // namespace osal
}  // namespace senscord
