/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <process.h>
#include <Windows.h>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"
#include "windows/osal_winerror.h"
#include "windows/thread_manager.h"

namespace {

namespace osal = senscord::osal;

/**
 * @brief Convert OSThread to thread id.
 */
uint32_t GetThreadId(osal::OSThread* thread) {
  uintptr_t temp = reinterpret_cast<uintptr_t>(thread);
  uint32_t thread_id = static_cast<uint32_t>(temp);
  return thread_id;
}

/**
 * @brief Convert thread id to OSThread.
 */
osal::OSThread* GetOSThread(uint32_t thread_id) {
  uintptr_t temp = static_cast<uintptr_t>(thread_id);
  osal::OSThread* thread = reinterpret_cast<osal::OSThread*>(temp);
  return thread;
}

/**
 * @brief Structure used by OSCreateThread function.
 */
struct ThreadProcParam {
  osal::OSThreadFunc func;  /**< Functions to be executed in new thread. */
  void* args;               /**< Argument to be passed to a new thread. */
  HANDLE event1;            /**< Event handle for synchronization. */
  HANDLE event2;
};

/**
 * @brief Thread procedure.
 */
uint32_t __stdcall _ThreadProc(void* param) {
  ThreadProcParam* proc_param = reinterpret_cast<ThreadProcParam*>(param);
  osal::OSThreadFunc func = proc_param->func;
  void* args = proc_param->args;
  HANDLE event1 = proc_param->event1;
  HANDLE event2 = proc_param->event2;

  // Notify the event (OSCreateThread <-- _ThreadProc)
  if (!SetEvent(event1)) {
    SENSCORD_OSAL_LOG_WARNING("failed (SetEvent err=%u)", GetLastError());
  }

  // Wait for the event (OSCreateThread --> _ThreadProc)
  DWORD wait_result = WaitForSingleObject(event2, INFINITE);
  if (wait_result != WAIT_OBJECT_0) {
    if (wait_result == WAIT_FAILED) {
      SENSCORD_OSAL_LOG_WARNING(
          "failed (WaitForSingleObject err=%u)", GetLastError());
    } else {
      SENSCORD_OSAL_LOG_WARNING("failed (WaitForSingleObject ret=%u)",
          wait_result);
    }
  }
  CloseHandle(event2);

  uint32_t thread_id = static_cast<uint32_t>(GetCurrentThreadId());

  if (osal::ThreadManager::GetInstance()->Contains(thread_id)) {
    // Run thread function.
    osal::OSThreadResult result = func(args);

    osal::ThreadManager::GetInstance()->Terminate(thread_id, result);
  } else {
    SENSCORD_OSAL_LOG_WARNING("failed (no such thread, thread_id=%"
                              PRIu32 ")", thread_id);
  }

  _endthreadex(0);

  // not reach.
  return 0;
}

}  // end of namespace


namespace senscord {
namespace osal {

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

  ThreadProcParam proc_param = {};
  proc_param.func = thread_func;
  proc_param.args = thread_argument;
  proc_param.event1 = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (proc_param.event1 == NULL) {
    SENSCORD_OSAL_LOG_ERROR(
        "failed (CreateEvent(event1) err=%u)", GetLastError());
    return OSMakeErrorCode(kFuncId, kErrorResourceExhausted);
  }
  proc_param.event2 = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (proc_param.event2 == NULL) {
    SENSCORD_OSAL_LOG_ERROR(
        "failed (CreateEvent(event2) err=%u)", GetLastError());
    CloseHandle(proc_param.event1);
    return OSMakeErrorCode(kFuncId, kErrorResourceExhausted);
  }

  // Create a new thread.
  uint32_t thread_id = 0;
  HANDLE thread_handle = reinterpret_cast<HANDLE>(
      _beginthreadex(NULL, 0, _ThreadProc, &proc_param, 0, &thread_id));
  if (thread_handle == NULL) {
    int32_t err = errno;
    SENSCORD_OSAL_LOG_ERROR("failed (_beginthreadex err=%" PRId32 ")", err);
    CloseHandle(proc_param.event1);
    CloseHandle(proc_param.event2);
    return OSMakeErrorCode(kFuncId, GetErrorCauseFromErrno(err));
  }

  // Wait for the event (OSCreateThread <-- _ThreadProc)
  DWORD wait_result = WaitForSingleObject(proc_param.event1, INFINITE);
  if (wait_result != WAIT_OBJECT_0) {
    if (wait_result == WAIT_FAILED) {
      SENSCORD_OSAL_LOG_WARNING(
          "failed (WaitForSingleObject err=%u)", GetLastError());
    } else {
      SENSCORD_OSAL_LOG_WARNING("failed (WaitForSingleObject ret=%u)",
          wait_result);
    }
  }
  CloseHandle(proc_param.event1);

  OSErrorCause cause = ThreadManager::GetInstance()->Register(
      thread_id, thread_handle, detach_state);
  if (cause == kErrorNone) {
    *thread = GetOSThread(thread_id);
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
  }

  // Notify the event (OSCreateThread --> _ThreadProc)
  if (!SetEvent(proc_param.event2)) {
    SENSCORD_OSAL_LOG_WARNING("failed (SetEvent err=%u)", GetLastError());
  }

  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

/**
 * @brief Detach a thread.
 * @param[in] thread  Thread object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDetachThread(OSThread* thread) {
  static const OSFunctionId kFuncId = kIdOSDetachThread;
  uint32_t thread_id = GetThreadId(thread);
  OSErrorCause cause = ThreadManager::GetInstance()->Detach(thread_id);
  if (cause != kErrorNone) {
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
  uint32_t thread_id = GetThreadId(thread);
  OSErrorCause cause = ThreadManager::GetInstance()->Join(thread_id, NULL,
      result);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Join with a terminated thread.
 * @param[in]  thread        Thread object.
 * @param[in]  nano_seconds  Timeout absolute time, in nanoseconds.
 * @param[out] result        Pointer to the variable that receives the thread
 *                           end result. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSTimedJoinThread(OSThread* thread,
                          uint64_t nano_seconds,
                          OSThreadResult* result) {
  static const OSFunctionId kFuncId = kIdOSTimedJoinThread;

  uint64_t curr_nanosec = 0;
  int32_t ret = OSGetTime(&curr_nanosec);
  if (ret != 0) {
    SENSCORD_OSAL_LOG_ERROR("failed (OSGetTime ret=0x%" PRIx32 ")", ret);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }
  uint64_t rel_nanosec = 0;
  if (nano_seconds > curr_nanosec) {
    rel_nanosec = nano_seconds - curr_nanosec;
  }

  uint32_t thread_id = GetThreadId(thread);
  OSErrorCause cause = ThreadManager::GetInstance()->Join(thread_id,
      &rel_nanosec, result);

  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Join with a terminated thread.
 * @param[in]  thread        Thread object.
 * @param[in]  nano_seconds  Timeout relative time, in nanoseconds.
 * @param[out] result        Pointer to the variable that receives the thread
 *                           end result. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSRelativeTimedJoinThread(OSThread* thread,
                                  uint64_t nano_seconds,
                                  OSThreadResult* result) {
  static const OSFunctionId kFuncId = kIdOSRelativeTimedJoinThread;
  uint32_t thread_id = GetThreadId(thread);
  OSErrorCause cause = ThreadManager::GetInstance()->Join(thread_id,
      &nano_seconds, result);

  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
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

  int32_t level = 0;
  switch (priority) {
    case kOSThreadPriorityIdle:
      level = THREAD_PRIORITY_IDLE;
      break;
    case kOSThreadPriorityLowest:
      level = THREAD_PRIORITY_LOWEST;
      break;
    case kOSThreadPriorityBelowNormal:
      level = THREAD_PRIORITY_BELOW_NORMAL;
      break;
    case kOSThreadPriorityDefault:  /* FALLTHROUGH */
    case kOSThreadPriorityNormal:
      level = THREAD_PRIORITY_NORMAL;
      break;
    case kOSThreadPriorityAboveNormal:
      level = THREAD_PRIORITY_ABOVE_NORMAL;
      break;
    case kOSThreadPriorityHighest:
      level = THREAD_PRIORITY_HIGHEST;
      break;
    default:
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  uint32_t thread_id = GetThreadId(thread);
  OSErrorCause cause = ThreadManager::GetInstance()->SetPriority(thread_id,
      level);

  if (cause != kErrorNone) {
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

  int32_t level = 0;
  uint32_t thread_id = GetThreadId(thread);
  OSErrorCause cause = ThreadManager::GetInstance()->GetPriority(thread_id,
      &level);

  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }

  switch (level) {
    case THREAD_PRIORITY_IDLE:
      *priority = kOSThreadPriorityIdle;
      break;
    case THREAD_PRIORITY_LOWEST:
      *priority = kOSThreadPriorityLowest;
      break;
    case THREAD_PRIORITY_BELOW_NORMAL:
      *priority = kOSThreadPriorityBelowNormal;
      break;
    case THREAD_PRIORITY_NORMAL:
      *priority = kOSThreadPriorityNormal;
      break;
    case THREAD_PRIORITY_ABOVE_NORMAL:
      *priority = kOSThreadPriorityAboveNormal;
      break;
    case THREAD_PRIORITY_HIGHEST:
      *priority = kOSThreadPriorityHighest;
      break;
    default:
    return OSMakeErrorCode(kFuncId, kErrorInternal);
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

  uint32_t thread_id = static_cast<uint32_t>(GetCurrentThreadId());

  *thread = GetOSThread(thread_id);

  return 0;
}

}  // namespace osal
}  // namespace senscord
