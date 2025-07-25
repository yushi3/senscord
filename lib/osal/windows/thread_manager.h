/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_OSAL_WINDOWS_THREAD_MANAGER_H_
#define LIB_OSAL_WINDOWS_THREAD_MANAGER_H_

#include <stdint.h>
#include <Windows.h>

#include <map>

#include "senscord/osal.h"
#include "common/osal_error.h"

namespace senscord {
namespace osal {

/**
 * @brief Thread manager class.
 *
 * - It provides equivalent behavior to POSIX threads.
 */
class ThreadManager {
 private:
  /**
   * @brief Thread information.
   */
  struct ThreadInfo {
    HANDLE handle;
    uint32_t joining_thread_id;
    OSThreadDetachState detach_state;
    OSThreadResult end_result;
    bool is_terminate;
  };

  // Key: Thread identifier, Value: Thread information.
  typedef std::map<uint32_t, ThreadInfo*> List;

 public:
  /**
   * @brief Get singleton instance.
   */
  static ThreadManager* GetInstance();

  /**
   * @brief Register thread information in the management list.
   * @param[in] thread_id     Thread identifier.
   * @param[in] handle        Thread handle.
   * @param[in] detach_state  Detached state of thread.
   * @return OSAL error cause. On success, it returns kErrorNone.
   */
  OSErrorCause Register(uint32_t thread_id, HANDLE handle,
                        OSThreadDetachState detach_state);

  /**
   * @brief Returns true if the thread is contained in the management list.
   * @param[in] thread_id  Thread identifier.
   */
  bool Contains(uint32_t thread_id) const;

  /**
   * @brief Detach a thread.
   * @param[in] thread_id  Thread identifier.
   * @return OSAL error cause. On success, it returns kErrorNone.
   */
  OSErrorCause Detach(uint32_t thread_id);

  /**
   * @brief Join the thread and get the end result.
   * @param[in]  thread_id     Thread identifier.
   * @param[in]  nano_seconds  Timeout relative time, in nanoseconds.
   * @param[out] result        Pointer to the variable that receives the thread
   *                           end result. (optional)
   * @return OSAL error cause. On success, it returns kErrorNone.
   */
  OSErrorCause Join(uint32_t thread_id, const uint64_t* nano_seconds,
                    OSThreadResult* result);

  /**
   * @brief Set thread end result.
   * @param[in] thread_id  Thread identifier.
   * @param[in] result     Thread end result.
   * @return OSAL error cause. On success, it returns kErrorNone.
   */
  OSErrorCause Terminate(uint32_t thread_id, OSThreadResult result);

  /**
   * @brief Set priority of a thread.
   * @param[in] thread_id  Thread identifier.
   * @param[in] level      Thread priority.
   * @return OSAL error cause. On success, it returns kErrorNone.
   */
  OSErrorCause SetPriority(uint32_t thread_id, int32_t level);

  /**
   * @brief Get priority of a thread.
   * @param[in]  thread_id  Thread identifier.
   * @param[out] level      Thread priority.
   * @return OSAL error cause. On success, it returns kErrorNone.
   */
  OSErrorCause GetPriority(uint32_t thread_id, int32_t* level);

 private:
  /**
   * @brief ThreadManager class constructor.
   */
  ThreadManager();

  /**
   * @brief ThreadManager class destructor.
   */
  virtual ~ThreadManager();

  ThreadManager(const ThreadManager&);  // = delete;
  ThreadManager& operator=(const ThreadManager&);  // = delete;

  /**
   * @brief Find thread information from management list.
   */
  ThreadInfo* Find(uint32_t thread_id) const;

  /**
   * @brief Release thread infomation and erase from management list.
   */
  void Release(uint32_t thread_id);

 private:
  List thread_list_;
  mutable CRITICAL_SECTION critical_;
};

}  // namespace osal
}  // namespace senscord

#endif  // LIB_OSAL_WINDOWS_THREAD_MANAGER_H_
