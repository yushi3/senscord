/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>

#include "senscord/osal.h"
#include "./osal_thread.h"
#include "./osal_error.h"
#include "./osal_logger.h"
#include "./osal_linuxerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Structure used by OSCreateThread function.
 */
struct ThreadProcParam {
  OSThreadFunc func;  /**< Functions to be executed in new thread. */
  void* args;         /**< Argument to be passed to a new thread. */
};

static void* _ThreadProc(void* param);

/**
 * @brief Create a new thread.
 * @param[out] thread          Pointer to the variable that receives the
 *                             thread object.
 * @param[in]  thread_func     Functions to be executed in new thread.
 * @param[in]  thread_argument Argument to be passed to a new thread.
 * @param[in]  thread_attr     Attributes for a new thread.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSCreateThread(OSThread** thread,
                       OSThreadFunc thread_func,
                       void* thread_argument,
                       const OSThreadAttribute* thread_attr) {
  static const OSFunctionId kFuncId = kIdOSCreateThread;
  if (thread == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (thread_func == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  OSThreadDetachState detach_state = kOSThreadJoinable;
  OSThreadPriority priority = kOSThreadPriorityDefault;
  if (thread_attr != NULL) {
    detach_state = thread_attr->detach_state;
    priority = thread_attr->priority;
  }

  if ((detach_state < kOSThreadJoinable) ||
      (detach_state > kOSThreadDetached)) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if ((priority < kOSThreadPriorityDefault) ||
      (priority > kOSThreadPriorityHighest)) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  // Set the thread attributes.
  pthread_attr_t attr;
  pthread_attr_init(&attr);
#if 0  // TODO: NuttX 8.2
  if (detach_state == kOSThreadDetached) {
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  }
#endif
  // Set the priority of thread after pthread_create.
  // Reason: Setting priority by pthread_attr_t is because error factors
  //         cannot be acquired.

  ThreadProcParam* proc_param = new ThreadProcParam;
  proc_param->func = thread_func;
  proc_param->args = thread_argument;

  pthread_t new_thread = 0;
  int32_t ret = pthread_create(&new_thread, &attr, _ThreadProc, proc_param);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(ret);
    ret = OSMakeErrorCode(kFuncId, cause);
    delete proc_param;
  } else {
    *thread = GetOSThread(new_thread);
    // TODO:
    //   Be disable the thread priority and scheduling policy.
#if 0
    // Set the priority of thread.
    int32_t prio_ret = OSSetThreadPriority(*thread, priority);
    if (prio_ret != 0) {
      static bool once = true;
      if (once) {
        once = false;
        SENSCORD_OSAL_LOG_WARNING("OSCreateThread set priority failed. ret=0x%"
            PRIx32, prio_ret);
      }
    }
#endif
    // NuttX 8.2
    if (detach_state == kOSThreadDetached) {
      pthread_detach(new_thread);
    }
  }

  pthread_attr_destroy(&attr);

  return ret;
}

/**
 * @brief Thread procedure.
 */
static void* _ThreadProc(void* param) {
  ThreadProcParam* proc_param = reinterpret_cast<ThreadProcParam*>(param);
  OSThreadFunc func = proc_param->func;
  void* args = proc_param->args;
  delete proc_param;

  // Execute the thread function.
  OSThreadResult result = func(args);

  return reinterpret_cast<void*>(result);
}

}  // namespace osal
}  // namespace senscord
