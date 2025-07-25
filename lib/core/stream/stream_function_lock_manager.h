/*
 * SPDX-FileCopyrightText: 2020-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_STREAM_STREAM_FUNCTION_LOCK_MANAGER_H_
#define LIB_CORE_STREAM_STREAM_FUNCTION_LOCK_MANAGER_H_

#include <vector>

#include "senscord/stream.h"
#include "senscord/osal.h"
#include "stream/stream_core.h"
#include "util/mutex.h"

namespace senscord {

/**
 * @brief Exclusive lock management class for StreamSource.
 */
class StreamFunctionLockManager {
 public:
  /**
   * @brief Constructor.
   */
  StreamFunctionLockManager();

  /**
   * @brief Destructor.
   */
  ~StreamFunctionLockManager();

  /**
   * @brief The other thread is being accessed or returned.
   * @return True is accessing.
   */
  bool IsAnotherThreadAccessing() const;

  /**
   * @brief Wait until all access is done.
   */
  void WaitAllAccessDone();

  /**
   * @brief Wait until access is done.
   * @param[in] (type) Target function type.
   */
  void WaitAccessDone(StreamFunctionType type);

  /**
   * @brief Check to see if this thread is already locked.
   * @return True is already locked.
   */
  bool IsLockedThisThread();

  /**
   * @brief Lock for internal process function.
   * @return Status object.
   */
  Status LockForInternal();

  /**
   * @brief Lock for component process function.
   * @return Status object.
   */
  Status LockForComponent();

  /**
   * @brief Lock for state change function.
   * @return Status object.
   */
  Status LockForState();

  /**
   * @brief Unlock a locked function.
   */
  void Unlock();

  /**
   * @brief Get local state.
   * @return Stream local state.
   */
  StreamLocalState GetStreamLocalState();

  /**
   * @brief Get local state.
   * @param[out] (is_changing) State changing progress
   * @return Stream local state.
   */
  StreamLocalState GetStreamLocalState(bool* is_changing);

  /**
   * @brief Begin the state change.
   * @param[in] (state) New state to set.
   * @return Status object.
   */
  Status BeginStateChange(StreamLocalState state);

  /**
   * @brief Commit the state change.
   */
  void CommitStateChange();

  /**
   * @brief Cancel the state change.
   */
  void CancelStateChange();

 private:
  /**
   * @brief Insert data into the function list.
   * @param[in] (type)   Type of function.
   * @param[in] (thread) Current thread.
   */
  void Insert(StreamFunctionType type, osal::OSThread* thread);

  /**
   * @brief Remove data from the function list.
   */
  void Remove();

  /**
   * @brief Waking up the other accesses that are waiting.
   */
  void WakeupWaitAceess();

  util::Mutex* mutex_;
  osal::OSCond* cond_;

  /**
   * @brief Information of the function.
   */
  struct FunctionInfo {
    StreamFunctionType type;
    osal::OSThread* thread;
  };

  typedef std::vector<FunctionInfo> FunctionList;

  // List of running functions.
  FunctionList running_functions_;

  // current state
  StreamLocalState current_state_;

  // destination state
  StreamLocalState dest_state_;

  // state mutex
  util::Mutex* mutex_state_;
};

}  // namespace senscord

#endif  // LIB_CORE_STREAM_STREAM_FUNCTION_LOCK_MANAGER_H_
