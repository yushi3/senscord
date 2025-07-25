/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <inttypes.h>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"
#include "linux/osal_linuxerror.h"

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
static OSErrorCause GetPolicyAndLevel(OSThreadPriority priority,
                                     int32_t* policy, int32_t* level);
static OSErrorCause GetOSPriority(int32_t policy, int32_t level,
                                  OSThreadPriority* priority);

/**
 * @brief Convert OSThread to thread id.
 */
extern pthread_t GetThreadId(OSThread* thread) {
  pthread_t thread_id = reinterpret_cast<pthread_t>(thread);
  return thread_id;
}

/**
 * @brief Convert thread id to OSThread.
 */
static OSThread* GetOSThread(pthread_t thread_id) {
  OSThread* thread = reinterpret_cast<OSThread*>(thread_id);
  return thread;
}

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
  if (detach_state == kOSThreadDetached) {
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  }
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

/**
 * @brief Detach a thread.
 * @param[in] thread  Thread object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDetachThread(OSThread* thread) {
  static const OSFunctionId kFuncId = kIdOSDetachThread;
  pthread_t thread_id = GetThreadId(thread);

  int32_t ret = pthread_detach(thread_id);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(ret);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

/**
 * @brief Join with a terminated thread.
 * @param[in]  thread  Thread object.
 * @param[out] result  Pointer to the variable that receives the thread end
 *                     result. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSJoinThread(OSThread* thread, OSThreadResult* result) {
  static const OSFunctionId kFuncId = kIdOSJoinThread;
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

/**
 * @brief Set priority of a thread.
 * @param[in] thread    Thread object.
 * @param[in] priority  Thread priority.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSetThreadPriority(OSThread* thread, OSThreadPriority priority) {
  static const OSFunctionId kFuncId = kIdOSSetThreadPriority;
  if ((priority < kOSThreadPriorityDefault) ||
      (priority > kOSThreadPriorityHighest)) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  pthread_t thread_id = GetThreadId(thread);

  int32_t policy = SCHED_RR;
  int32_t level = 0;
  OSErrorCause cause = GetPolicyAndLevel(priority, &policy, &level);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }
  sched_param param = {};
  param.sched_priority = level;

  int32_t ret = pthread_setschedparam(thread_id, policy, &param);
  if (ret != 0) {
    cause = GetErrorCauseFromErrno(ret);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

/**
 * @brief Get priority of a thread.
 * @param[in]  thread    Thread object.
 * @param[out] priority  Thread priority.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetThreadPriority(OSThread* thread, OSThreadPriority* priority) {
  static const OSFunctionId kFuncId = kIdOSGetThreadPriority;
  if (priority == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  pthread_t thread_id = GetThreadId(thread);

  int32_t policy = 0;
  sched_param param = {};
  int32_t ret = pthread_getschedparam(thread_id, &policy, &param);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(ret);
    return OSMakeErrorCode(kFuncId, cause);
  }

  OSErrorCause cause = GetOSPriority(policy, param.sched_priority, priority);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

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

/**
 * @brief Get the scheduling policy and priority level.
 */
static OSErrorCause GetPolicyAndLevel(OSThreadPriority priority,
                                      int32_t* policy, int32_t* level) {
  if ((policy == NULL) || (level == NULL)) {
    return kErrorInvalidArgument;
  }

  *policy = SCHED_RR;

  // Find the median from min and max.
  int32_t min = sched_get_priority_min(*policy);
  int32_t max = sched_get_priority_max(*policy);
  int32_t center = (min + max) / 2;

  if (priority == kOSThreadPriorityDefault) {
    priority = kOSThreadPriorityNormal;
  }

  *level = center + (priority - kOSThreadPriorityNormal);

  return kErrorNone;
}

/**
 * @brief Get the priority of OSThread.
 */
static OSErrorCause GetOSPriority(int32_t policy, int32_t level,
                                  OSThreadPriority* priority) {
  if (priority == NULL) {
    return kErrorInvalidArgument;
  }
  if (policy != SCHED_RR) {
    return kErrorNotPermitted;
  }

  // Find the median from min and max.
  int32_t min = sched_get_priority_min(policy);
  int32_t max = sched_get_priority_max(policy);
  int32_t center = (min + max) / 2;

  int32_t val = kOSThreadPriorityNormal + (level - center);

  if ((val < kOSThreadPriorityIdle) ||
      (val > kOSThreadPriorityHighest)) {
    SENSCORD_OSAL_LOG_ERROR("invalid priority. val=0x%" PRIx32, val);
    return kErrorInternal;
  }

  *priority = static_cast<OSThreadPriority>(val);

  return kErrorNone;
}

}  // namespace osal
}  // namespace senscord
