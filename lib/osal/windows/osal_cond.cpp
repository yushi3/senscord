/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <Windows.h>

#include <inttypes.h>
#include <vector>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"
#include "windows/osal_winerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Thread data for each condition variable.
 */
struct CondPerThread {
  OSThread* thread;  /**< Thread object. */
  HANDLE    event;   /**< Event handle for notification. */
};

/**
 * @brief Management data for condition variable.
 */
struct CondData {
  /** List of threads that are not waiting */
  std::vector<CondPerThread*> idling_threads;
  /** List of waiting threads */
  std::vector<CondPerThread*> waiting_threads;
  /** Mutex object */
  OSMutex* mutex;
};

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

  CondData* cond_temporary = new CondData();
  OSCreateMutex(&cond_temporary->mutex);
  *cond = reinterpret_cast<OSCond*>(cond_temporary);

  return 0;
}

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

  int32_t result = 0;
  CondData* cond_data = reinterpret_cast<CondData*>(cond);

  OSLockMutex(cond_data->mutex);
  if (!cond_data->waiting_threads.empty()) {
    result = OSMakeErrorCode(kFuncId, kErrorBusy);
  } else {
    while (!cond_data->idling_threads.empty()) {
      CondPerThread* per_thread = cond_data->idling_threads.back();
      CloseHandle(per_thread->event);
      delete per_thread;
      cond_data->idling_threads.pop_back();
    }
  }
  OSUnlockMutex(cond_data->mutex);

  if (result == 0) {
    OSDestroyMutex(cond_data->mutex);
    delete cond_data;
  }

  return result;
}

/**
 * @brief Wait for timeout or until notified. (Common function)
 * @param[in] cond   Condition variable object.
 * @param[in] mutex  Mutex object that is currently locked by this thread.
 * @param[in] milli_seconds  Timeout relative time, in milliseconds.
 * @return OSAL error cause. On success, it returns kErrorNone.
 */
static OSErrorCause WaitCond(OSCond* cond, OSMutex* mutex,
                             DWORD milli_seconds) {
  CondData* cond_list = reinterpret_cast<CondData*>(cond);
  OSThread* current_therad;
  HANDLE*   curennt_event = NULL;

  int32_t result = OSGetCurrentThread(&current_therad);
  if (result != 0) {
    SENSCORD_OSAL_LOG_ERROR("OSGetCurrentThread failed. ret=0x%" PRIx32,
        result);
    return kErrorInternal;
  }

  OSErrorCause cause = kErrorNone;
  {
    CondPerThread* temporary_per_thread = NULL;
    std::vector<CondPerThread*>::iterator per_thread;

    OSLockMutex(cond_list->mutex);
    for (per_thread = cond_list->idling_threads.begin();
      per_thread != cond_list->idling_threads.end(); ++per_thread) {
      if ((*per_thread)->thread == current_therad) {
        temporary_per_thread = *per_thread;
        cond_list->idling_threads.erase(per_thread);
        break;
      }
    }

    if (temporary_per_thread == NULL) {
      temporary_per_thread = new CondPerThread();
      temporary_per_thread->thread = current_therad;
      temporary_per_thread->event = CreateEvent(NULL, FALSE, FALSE, NULL);
      if (temporary_per_thread->event == NULL) {
        delete temporary_per_thread;
        SENSCORD_OSAL_LOG_ERROR("CreateEvent failed.");
        cause = kErrorInternal;
      }
    }
    if (cause == kErrorNone) {
      cond_list->waiting_threads.push_back(temporary_per_thread);
      curennt_event = &temporary_per_thread->event;
    }
    OSUnlockMutex(cond_list->mutex);
  }

  if (cause == kErrorNone) {
    DWORD wait_result;
    {
      OSUnlockMutex(mutex);
      wait_result = WaitForSingleObject(*curennt_event, milli_seconds);
      OSLockMutex(mutex);
    }
    if (wait_result == WAIT_TIMEOUT) {
      cause = kErrorTimedOut;
    } else if (wait_result != WAIT_OBJECT_0) {
      SENSCORD_OSAL_LOG_ERROR(
          "failed (WaitForSingleObject err=%u)", GetLastError());
      cause = kErrorInternal;
    }

    std::vector<CondPerThread*>::iterator per_thread;

    OSLockMutex(cond_list->mutex);
    for (per_thread = cond_list->waiting_threads.begin();
      per_thread != cond_list->waiting_threads.end(); ++per_thread) {
      CondPerThread* temporary_per_thread = *per_thread;
      if (temporary_per_thread->thread == current_therad) {
        cond_list->waiting_threads.erase(per_thread);
        cond_list->idling_threads.push_back(temporary_per_thread);
        break;
      }
    }
    OSUnlockMutex(cond_list->mutex);
  }

  return cause;
}

/**
 * @brief Wait until notified.
 * @param[in] cond   Condition variable object.
 * @param[in] mutex  Mutex object that is currently locked by this thread.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSWaitCond(OSCond* cond, OSMutex* mutex) {
  static const OSFunctionId kFuncId = kIdOSWaitCond;
  if (cond == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  OSErrorCause cause = WaitCond(cond, mutex, INFINITE);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Wait for timeout or until notified. (specify absolute time)
 * @param[in] cond   Condition variable object.
 * @param[in] mutex  Mutex object that is currently locked by this thread.
 * @param[in] nano_seconds  Timeout absolute time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSTimedWaitCond(OSCond* cond, OSMutex* mutex,
                        uint64_t nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSTimedWaitCond;
  if (cond == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  uint64_t current_time = 0;
  int32_t result = OSGetTime(&current_time);
  if (result != 0) {
    SENSCORD_OSAL_LOG_ERROR("OSGetTime failed. ret=0x%" PRIx32, result);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }

  DWORD milli_seconds;
  if (nano_seconds <= current_time) {
    milli_seconds = 0;
  } else {
    milli_seconds = static_cast<DWORD>(
        (nano_seconds - current_time + 999999) / 1000000);
  }

  OSErrorCause cause = WaitCond(cond, mutex, milli_seconds);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Wait for timeout or until notified. (specify relative time)
 * @param[in] cond   Condition variable object.
 * @param[in] mutex  Mutex object that is currently locked by this thread.
 * @param[in] nano_seconds  Timeout relative time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSRelativeTimedWaitCond(OSCond* cond, OSMutex* mutex,
                                uint64_t nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSRelativeTimedWaitCond;
  if (cond == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  DWORD milli_seconds = static_cast<DWORD>((nano_seconds + 999999) / 1000000);

  OSErrorCause cause = WaitCond(cond, mutex, milli_seconds);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Unblocks one of the threads waiting for the condition variable.
 * @param[in] cond  Condition variable object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSignalCond(OSCond* cond) {
  static const OSFunctionId kFuncId = kIdOSSignalCond;
  if (cond == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int32_t result = 0;
  CondData* cond_list = reinterpret_cast<CondData*>(cond);

  OSLockMutex(cond_list->mutex);
  if (!cond_list->waiting_threads.empty()) {
    CondPerThread* per_thread = cond_list->waiting_threads.front();
    BOOL signal_result = SetEvent(per_thread->event);
    if (!signal_result) {
      SENSCORD_OSAL_LOG_ERROR("failed (SetEvent err=%u)", GetLastError());
      result = OSMakeErrorCode(kFuncId, kErrorUnknown);
    }
  }
  OSUnlockMutex(cond_list->mutex);

  return result;
}

/**
 * @brief Unblocks all threads waiting for the condition variable.
 * @param[in] cond  Condition variable object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSBroadcastCond(OSCond* cond) {
  static const OSFunctionId kFuncId = kIdOSBroadcastCond;
  if (cond == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int32_t result = 0;
  CondData* cond_list = reinterpret_cast<CondData*>(cond);
  std::vector<CondPerThread*>::iterator per_thread;

  OSLockMutex(cond_list->mutex);
  for (per_thread = cond_list->waiting_threads.begin();
    per_thread != cond_list->waiting_threads.end(); ++per_thread) {
    BOOL signal_result = SetEvent((*per_thread)->event);
    if (!signal_result) {
      SENSCORD_OSAL_LOG_ERROR("failed (SetEvent err=%u)", GetLastError());
      result = OSMakeErrorCode(kFuncId, kErrorUnknown);
      break;
    }
  }
  OSUnlockMutex(cond_list->mutex);

  return result;
}

}  //  namespace osal
}  //  namespace senscord
