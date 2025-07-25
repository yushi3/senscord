/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include "windows/thread_manager.h"
#include "common/osal_logger.h"

namespace senscord {
namespace osal {

// first get singleton instance for thread safe.
static ThreadManager* dummy = ThreadManager::GetInstance();

/**
 * @brief Get singleton instance.
 */
ThreadManager* ThreadManager::GetInstance() {
  static ThreadManager instance;
  return &instance;
}

/**
 * @brief ThreadManager class constructor.
 */
ThreadManager::ThreadManager()
  : thread_list_(), critical_() {
  InitializeCriticalSection(&critical_);
}

/**
 * @brief ThreadManager class destructor.
 */
ThreadManager::~ThreadManager() {
  EnterCriticalSection(&critical_);
  for (List::iterator itr = thread_list_.begin();
       itr != thread_list_.end(); ++itr) {
    ThreadInfo* info = itr->second;
    delete info;
  }
  thread_list_.clear();
  LeaveCriticalSection(&critical_);
  DeleteCriticalSection(&critical_);
}

/**
 * @brief Register thread information in the management list.
 * @param[in] thread_id     Thread identifier.
 * @param[in] handle        Thread handle.
 * @param[in] detach_state  Detached state of thread.
 * @return OSAL error cause. On success, it returns kErrorNone.
 */
OSErrorCause ThreadManager::Register(uint32_t thread_id, HANDLE handle,
                                     OSThreadDetachState detach_state) {
  OSErrorCause ret = kErrorNone;

  ThreadInfo* info = new ThreadInfo;
  info->handle = handle;
  info->detach_state = detach_state;
  info->joining_thread_id = 0;
  info->end_result = 0;
  info->is_terminate = false;

  EnterCriticalSection(&critical_);

  bool result = thread_list_.insert(List::value_type(thread_id, info)).second;

  LeaveCriticalSection(&critical_);

  if (!result) {
    delete info;
    SENSCORD_OSAL_LOG_ERROR("failed (already exist)");
    ret = kErrorAlreadyExists;
  }

  return ret;
}

/**
 * @brief Find thread information from management list.
 */
ThreadManager::ThreadInfo* ThreadManager::Find(uint32_t thread_id) const {
  List::const_iterator itr = thread_list_.find(thread_id);
  if (itr != thread_list_.end()) {
    return itr->second;
  }
  return NULL;
}

/**
 * @brief Returns true if the thread is contained in the management list.
 * @param[in] thread_id  Thread identifier.
 */
bool ThreadManager::Contains(uint32_t thread_id) const {
  EnterCriticalSection(&critical_);

  ThreadInfo* info = Find(thread_id);

  LeaveCriticalSection(&critical_);

  return (info != NULL);
}

/**
 * @brief Detach a thread.
 * @param[in] thread_id  Thread identifier.
 * @return OSAL error cause. On success, it returns kErrorNone.
 */
OSErrorCause ThreadManager::Detach(uint32_t thread_id) {
  OSErrorCause ret = kErrorNone;

  EnterCriticalSection(&critical_);

  do {
    ThreadInfo* info = Find(thread_id);
    if (info == NULL) {
      SENSCORD_OSAL_LOG_ERROR("failed (no such thread)");
      ret = kErrorNotFound;
      break;
    }

    if (info->detach_state != kOSThreadJoinable) {
      SENSCORD_OSAL_LOG_ERROR("failed (already detached)");
      ret = kErrorInvalidArgument;
      break;
    }

    if (info->joining_thread_id != 0) {
      // If another thread is Joining, it does nothing.
      SENSCORD_OSAL_LOG_WARNING("another thread joining");
      ret = kErrorNone;
      break;
    }

    info->detach_state = kOSThreadDetached;

    if (info->is_terminate) {
      Release(thread_id);
    }
  } while (false);

  LeaveCriticalSection(&critical_);

  return ret;
}

/**
 * @brief Join the thread and get the end result.
 * @param[in]  thread_id     Thread identifier.
 * @param[in]  nano_seconds  Timeout relative time, in nanoseconds.
 * @param[out] result        Pointer to the variable that receives the thread
 *                           end result. (optional)
 * @return OSAL error cause. On success, it returns kErrorNone.
 */
OSErrorCause ThreadManager::Join(uint32_t thread_id,
                                 const uint64_t* nano_seconds,
                                 OSThreadResult* result) {
  uint32_t curr_id = static_cast<uint32_t>(GetCurrentThreadId());
  if (thread_id == curr_id) {
    SENSCORD_OSAL_LOG_ERROR("failed (deadlock)");
    return kErrorDeadLock;
  }

  OSErrorCause ret = kErrorNone;
  HANDLE handle = NULL;

  // Get handle and change status.
  EnterCriticalSection(&critical_);

  do {
    ThreadInfo* info = Find(thread_id);
    if (info == NULL) {
      SENSCORD_OSAL_LOG_ERROR("failed (no such thread)");
      ret = kErrorNotFound;
      break;
    }

    if (info->detach_state != kOSThreadJoinable) {
      SENSCORD_OSAL_LOG_ERROR("failed (already detached)");
      ret = kErrorInvalidArgument;
      break;
    }

    if (info->joining_thread_id != 0) {
      SENSCORD_OSAL_LOG_ERROR("failed (another thread joining)");
      ret = kErrorInvalidArgument;
      break;
    }

    info->joining_thread_id = thread_id;
    handle = info->handle;
  } while (false);

  LeaveCriticalSection(&critical_);

  if (ret != 0) {
    return ret;
  }

  // Wait for thread termination.
  DWORD timeout = 0;
  if (nano_seconds != NULL) {
    timeout = static_cast<DWORD>(*nano_seconds / (1000 * 1000));
  } else {
    timeout = INFINITE;
  }
  DWORD wait_result = WaitForSingleObject(handle, timeout);

  if (wait_result != WAIT_OBJECT_0) {
    if (wait_result == WAIT_TIMEOUT) {
      SENSCORD_OSAL_LOG_ERROR("failed (timedout)");
      ret = kErrorTimedOut;
    } else if (wait_result == WAIT_FAILED) {
      SENSCORD_OSAL_LOG_ERROR(
          "failed (WaitForSingleObject err=%u)", GetLastError());
      ret = kErrorUnknown;
    } else {
      SENSCORD_OSAL_LOG_ERROR("failed (WaitForSingleObject ret=%u)",
          wait_result);
      ret = kErrorUnknown;
    }

    // Restore the status.
    EnterCriticalSection(&critical_);

    do {
      ThreadInfo* info = Find(thread_id);
      if (info == NULL) {
        SENSCORD_OSAL_LOG_ERROR("failed (no such thread)");
        ret = kErrorNotFound;
        break;
      }

      info->joining_thread_id = 0;
    } while (false);

    LeaveCriticalSection(&critical_);

    return ret;
  }

  // Get the end result and releases the thread.
  EnterCriticalSection(&critical_);

  if (result != NULL) {
    do {
      ThreadInfo* info = Find(thread_id);
      if (info == NULL) {
        SENSCORD_OSAL_LOG_ERROR("failed (no such thread)");
        ret = kErrorNotFound;
        break;
      }

      *result = info->end_result;
    } while (false);
  }

  Release(thread_id);

  LeaveCriticalSection(&critical_);

  return ret;
}

/**
 * @brief Set thread end result.
 * @param[in] thread_id  Thread identifier.
 * @param[in] result     Thread end result.
 * @return OSAL error cause. On success, it returns kErrorNone.
 */
OSErrorCause ThreadManager::Terminate(uint32_t thread_id,
                                      OSThreadResult result) {
  OSErrorCause ret = kErrorNone;

  EnterCriticalSection(&critical_);

  do {
    ThreadInfo* info = Find(thread_id);
    if (info == NULL) {
      SENSCORD_OSAL_LOG_ERROR("failed (no such thread)");
      ret = kErrorNotFound;
      break;
    }

    if (info->detach_state == osal::kOSThreadJoinable) {
      info->is_terminate = true;
      info->end_result = result;
    } else if (info->detach_state == osal::kOSThreadDetached) {
      Release(thread_id);
    } else {
      SENSCORD_OSAL_LOG_ERROR("failed (invalid state)");
      ret = kErrorInternal;
      break;
    }
  } while (false);

  LeaveCriticalSection(&critical_);

  return ret;
}

/**
 * @brief Set priority of a thread.
 * @param[in] thread_id  Thread identifier.
 * @param[in] level      Thread priority.
 * @return OSAL error cause. On success, it returns kErrorNone.
 */
OSErrorCause ThreadManager::SetPriority(uint32_t thread_id, int32_t level) {
  OSErrorCause ret = kErrorNone;

  EnterCriticalSection(&critical_);

  do {
    ThreadInfo* info = Find(thread_id);
    if (info == NULL) {
      SENSCORD_OSAL_LOG_ERROR("failed (no such thread)");
      ret = kErrorNotFound;
      break;
    }

    if (info->is_terminate) {
      SENSCORD_OSAL_LOG_ERROR("failed (already finished)");
      ret = kErrorNotFound;
      break;
    }

    HANDLE handle = info->handle;
    if (handle == NULL) {
      uint32_t curr_id = static_cast<uint32_t>(GetCurrentThreadId());
      if (thread_id == curr_id) {
        // Get pseudo thread handle.
        handle = GetCurrentThread();
      } else {
        SENSCORD_OSAL_LOG_ERROR("failed (invalid handle)");
        ret = kErrorInvalidArgument;
        break;
      }
    }

    if (!SetThreadPriority(handle, level)) {
      SENSCORD_OSAL_LOG_ERROR(
          "failed (SetThreadPriority err=%u)", GetLastError());
      ret = kErrorUnknown;
      break;
    }
  } while (false);

  LeaveCriticalSection(&critical_);

  return ret;
}

/**
 * @brief Get priority of a thread.
 * @param[in]  thread_id  Thread identifier.
 * @param[out] level      Thread priority.
 * @return OSAL error cause. On success, it returns kErrorNone.
 */
OSErrorCause ThreadManager::GetPriority(uint32_t thread_id, int32_t* level) {
  if (level == NULL) {
    return kErrorInvalidArgument;
  }

  OSErrorCause ret = kErrorNone;

  EnterCriticalSection(&critical_);

  do {
    ThreadInfo* info = Find(thread_id);
    if (info == NULL) {
      SENSCORD_OSAL_LOG_ERROR("failed (no such thread)");
      ret = kErrorNotFound;
      break;
    }

    if (info->is_terminate) {
      SENSCORD_OSAL_LOG_ERROR("failed (already finished)");
      ret = kErrorNotFound;
      break;
    }

    HANDLE handle = info->handle;
    if (handle == NULL) {
      uint32_t curr_id = static_cast<uint32_t>(GetCurrentThreadId());
      if (thread_id == curr_id) {
        // Get pseudo thread handle.
        handle = GetCurrentThread();
      } else {
        SENSCORD_OSAL_LOG_ERROR("failed (invalid handle)");
        ret = kErrorInvalidArgument;
        break;
      }
    }

    int32_t val = 0;
    val = GetThreadPriority(handle);
    if (val == THREAD_PRIORITY_ERROR_RETURN) {
      SENSCORD_OSAL_LOG_ERROR(
          "failed (GetThreadPriority err=%u)", GetLastError());
      ret = kErrorUnknown;
      break;
    }

    *level = val;
  } while (false);

  LeaveCriticalSection(&critical_);

  return ret;
}

/**
 * @brief Release thread infomation and erase from management list.
 */
void ThreadManager::Release(uint32_t thread_id) {
  ThreadInfo* info = Find(thread_id);
  if (info == NULL) {
    SENSCORD_OSAL_LOG_ERROR("failed (thread_id=%" PRIu32 ")", thread_id);
    return;
  }

  if (info->handle != NULL) {
    CloseHandle(info->handle);
  }

  delete info;
  thread_list_.erase(thread_id);
}

}  // namespace osal
}  // namespace senscord
