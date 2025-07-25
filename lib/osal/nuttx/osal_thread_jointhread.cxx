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
 * @brief Join with a terminated thread.
 * @param[in]  thread  Thread object.
 * @param[out] result  Pointer to the variable that receives the thread end
 *                     result. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSJoinThread(OSThread* thread, OSThreadResult* result) {
  static const OSFunctionId kFuncId = kIdOSJoinThread;
  if (thread == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorNotFound);
  }
  pthread_t thread_id = GetThreadId(thread);

  void* thread_result = NULL;
  int32_t ret = pthread_join(thread_id, &thread_result);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(ret);
    return OSMakeErrorCode(kFuncId, cause);
  }

  if (result != NULL) {
    *result = reinterpret_cast<OSThreadResult>(thread_result);
  }

  return 0;
}

}  // namespace osal
}  // namespace senscord
