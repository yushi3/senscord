/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_COMPONENT_STREAM_SOURCE_FUNCTION_LOCK_MANAGER_H_
#define LIB_CORE_COMPONENT_STREAM_SOURCE_FUNCTION_LOCK_MANAGER_H_

#include <string>
#include <vector>

#include "senscord/status.h"
#include "senscord/osal.h"
#include "util/mutex.h"

namespace senscord {

/**
 * @brief Type of function.
 */
enum StreamSourceFunctionType {
  kFunctionTypeState,
  kFunctionTypeReleaseFrame,
  kFunctionTypeProperty,
};

/**
 * @brief Exclusive lock management class for StreamSource.
 */
class StreamSourceFunctionLockManager {
 public:
  /**
   * @brief Constructor.
   */
  StreamSourceFunctionLockManager();

  /**
   * @brief Destructor.
   */
  ~StreamSourceFunctionLockManager();

  /**
   * @brief Get the state change in progress.
   * @return true is state changing.
   */
  bool IsStateChanging() const;

  /**
   * @brief Lock for State function.
   */
  void LockForState();

  /**
   * @brief Lock for ReleaseFrame function.
   */
  void LockForReleaseFrame();

  /**
   * @brief Lock for Property functions.
   */
  void LockForProperty();

  /**
   * @brief Unlock a locked function.
   */
  void Unlock();

 private:
  /**
   * @brief Set the state change in progress.
   * @param[in] changing true is state changing.
   */
  void SetStateChanging(bool changing);

  /**
   * @brief Insert data into the function list.
   * @param[in] type  Type of function.
   * @param[in] thread Current thread.
   */
  void Insert(StreamSourceFunctionType type, osal::OSThread* thread);

  /**
   * @brief Remove data from the function list.
   */
  void Remove();

  /**
   * @brief Waking up the other accesses that are waiting.
   */
  void WakeupWaitFunction();

  util::Mutex* mutex_;
  osal::OSCond* cond_;

  // State change in progress.
  bool state_changing_;

  /**
   * @brief Information of the function.
   */
  struct FunctionInfo {
    StreamSourceFunctionType type;
    osal::OSThread* thread;
  };

  typedef std::vector<FunctionInfo> FunctionList;

  // List of running functions.
  FunctionList running_functions_;
};

/**
 * @brief RAII-style function lock.
 */
class StreamSourceFunctionLock {
 public:
  /**
   * @brief Acquire the lock of the function.
   * @param[in] manager  Lock manager.
   * @param[in] type     Type of function.
   */
  StreamSourceFunctionLock(
      StreamSourceFunctionLockManager* manager,
      StreamSourceFunctionType type);

  /**
   * @brief Release the lock of the function.
   */
  ~StreamSourceFunctionLock();

 private:
  StreamSourceFunctionLockManager* manager_;
};

}  // namespace senscord

#endif  // LIB_CORE_COMPONENT_STREAM_SOURCE_FUNCTION_LOCK_MANAGER_H_
