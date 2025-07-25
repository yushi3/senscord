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
 * @brief Get the current thread.
 * @param[out] thread  Pointer to the variable that receives the thread object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetCurrentThread(OSThread** thread) {
  static const OSFunctionId kFuncId = kIdOSGetCurrentThread;
  if (thread == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  pthread_t thread_id = pthread_self();
  *thread = GetOSThread(thread_id);

  return 0;
}

}  // namespace osal
}  // namespace senscord
