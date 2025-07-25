/*
 * SPDX-FileCopyrightText: 2020-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stream/stream_function_lock_manager.h"

#include <inttypes.h>

#include "logger/logger.h"
#include "util/autolock.h"

namespace senscord {

/**
 * @brief Constructor.
 */
StreamFunctionLockManager::StreamFunctionLockManager() :
    current_state_(kStreamLocalStateNotInit),
    dest_state_(kStreamLocalStateNotInit) {
  mutex_ = new util::Mutex();
  mutex_state_ = new util::Mutex();
  cond_ = NULL;
  osal::OSCreateCond(&cond_);
}

/**
 * @brief Destructor.
 */
StreamFunctionLockManager::~StreamFunctionLockManager() {
  osal::OSDestroyCond(cond_);
  cond_ = NULL;
  delete mutex_;
  mutex_ = NULL;
  delete mutex_state_;
  mutex_state_ = NULL;
}

/**
 * @brief The other thread is being accessed or returned.
 * @return True is accessing.
 */
bool StreamFunctionLockManager::IsAnotherThreadAccessing() const {
  util::AutoLock autolock(mutex_);
  if (!running_functions_.empty()) {
    return true;
  }
  return false;
}

/**
 * @brief Wait until all access is done.
 */
void StreamFunctionLockManager::WaitAllAccessDone() {
  util::AutoLock autolock(mutex_);
  while (IsAnotherThreadAccessing()) {
    osal::OSWaitCond(cond_, mutex_->GetObject());
  }
}

/**
 * @brief Wait until access is done.
 * @param[in] (type) Target function type.
 */
void StreamFunctionLockManager::WaitAccessDone(StreamFunctionType type) {
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);
  util::AutoLock autolock(mutex_);
  while (!running_functions_.empty()) {
    bool waited = false;
    StreamFunctionLockManager::FunctionList::const_iterator itr;
    for (itr = running_functions_.begin();
       itr != running_functions_.end(); ++itr) {
      if (itr->thread != thread && itr->type == type) {
        waited = true;
        osal::OSWaitCond(cond_, mutex_->GetObject());
        break;
      }
    }
    if (!waited) {
      break;  // access all done
    }
  }
}

/**
 * @brief Check to see if this thread is already locked.
 * @return True is already locked.
 */
bool StreamFunctionLockManager::IsLockedThisThread() {
  util::AutoLock autolock(mutex_);
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);
  StreamFunctionLockManager::FunctionList::const_iterator itr;
  for (itr = running_functions_.begin();
       itr != running_functions_.end(); ++itr) {
    if (itr->thread == thread) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Insert data into the function list.
 * @param[in] (type)   Type of function.
 * @param[in] (thread) Current thread.
 */
void StreamFunctionLockManager::Insert(
    StreamFunctionType type, osal::OSThread* thread) {
  FunctionInfo info = {};
  info.type = type;
  info.thread = thread;
  running_functions_.push_back(info);
}

/**
 * @brief Remove data from the function list.
 */
void StreamFunctionLockManager::Remove() {
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);
  StreamFunctionLockManager::FunctionList::iterator itr;
  for (itr = running_functions_.begin();
       itr != running_functions_.end(); ++itr) {
    // check only those that have been accessed before yourself
    if (itr->thread == thread) {
      running_functions_.erase(itr);
      break;
    }
  }
}

/**
 * @brief Waking up the other accesses that are waiting.
 */
void StreamFunctionLockManager::WakeupWaitAceess() {
  osal::OSBroadcastCond(cond_);
}

/**
 * @brief Lock for state change function.
 * @return Status object.
 */
Status StreamFunctionLockManager::LockForState() {
  util::AutoLock autolock(mutex_);

  // entry accessing function
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);
  Insert(kStreamFunctionTypeState, thread);

  // [1] Failure:          none
  // [2] Wait and recheck: contains(State | Component)
  // [3] Success:          empty or contains(Internal)
  while (!running_functions_.empty()) {
    bool wait = false;
    StreamFunctionLockManager::FunctionList::const_iterator itr;
    for (itr = running_functions_.begin();
         itr != running_functions_.end(); ++itr) {
      // no one else is accessing, so self the first.
      if (itr->thread == thread) {
        break;
      } else if (itr->type == kStreamFunctionTypeState ||
                 itr->type == kStreamFunctionTypeComponent) {
        wait = true;
        break;
      }
    }
    if (!wait) {
      break;  // Success.
    }
    // Wait and recheck.
    int32_t ret = osal::OSWaitCond(cond_, mutex_->GetObject());
    if (ret < 0) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
          "failed to wait until the state change. (ret=%" PRIx32 ")", ret);
    }
  }
  return Status::OK();
}

/**
 * @brief Lock for internal process function.
 * @return Status object.
 */
Status StreamFunctionLockManager::LockForInternal() {
  util::AutoLock autolock(mutex_);

  // entry accessing function
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);
  Insert(kStreamFunctionTypeInternal, thread);

  // [1] Failure:          contains(State(dest isn't Ready/Running))
  // [2] Wait and recheck: none
  // [3] Success:          empty or contains(Internal | Component |
  //                       State(dest is Ready/Running))
  while (!running_functions_.empty()) {
    StreamFunctionLockManager::FunctionList::const_iterator itr;
    for (itr = running_functions_.begin();
         itr != running_functions_.end(); ++itr) {
      // no one else is accessing, so self the first.
      if (itr->thread == thread) {
        break;
      } else if (itr->type == kStreamFunctionTypeState) {
        util::AutoLock state_lock(mutex_state_);
        if (dest_state_ != kStreamLocalStateReady &&
            dest_state_ != kStreamLocalStateRunning) {
          return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
              "invalid state(closing stream).");
        }
      }
    }
    break;  // Success.
  }
  return Status::OK();
}

/**
 * @brief Lock for component process function.
 * @return Status object.
 */
Status StreamFunctionLockManager::LockForComponent() {
  util::AutoLock autolock(mutex_);

  // entry accessing function
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);
  Insert(kStreamFunctionTypeComponent, thread);

  // [1] Failure:          contains(State(dest isn't Ready/Running))
  // [2] Wait and recheck: none
  // [3] Success:          empty or contains(Internal | Component |
  //                       State(dest is Ready/Running))
  while (!running_functions_.empty()) {
    StreamFunctionLockManager::FunctionList::const_iterator itr;
    for (itr = running_functions_.begin();
         itr != running_functions_.end(); ++itr) {
      // no one else is accessing, so self the first.
      if (itr->thread == thread) {
        break;
      } else if (itr->type == kStreamFunctionTypeState) {
        util::AutoLock state_lock(mutex_state_);
        if (dest_state_ != kStreamLocalStateReady &&
            dest_state_ != kStreamLocalStateRunning) {
          return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
              "invalid state(closing stream).");
        }
      }
    }
    break;  // Success.
  }
  return Status::OK();
}

/**
 * @brief Unlock a locked function.
 */
void StreamFunctionLockManager::Unlock() {
  util::AutoLock autolock(mutex_);
  Remove();
  WakeupWaitAceess();
}

/**
 * @brief Get local state.
 * @return Stream local state.
 */
StreamLocalState StreamFunctionLockManager::GetStreamLocalState() {
  util::AutoLock autolock(mutex_state_);
  return current_state_;
}

/**
 * @brief Get local state.
 * @param[out] (is_changing) State changing progress
 * @return Stream local state.
 */
StreamLocalState StreamFunctionLockManager::GetStreamLocalState(
    bool* is_changing) {
  util::AutoLock autolock(mutex_state_);
  *is_changing = (current_state_ != dest_state_);
  return GetStreamLocalState();
}

/**
 * @brief Begin the state change.
 * @param[in] (state) New state to set.
 * @return Status object.
 */
Status StreamFunctionLockManager::BeginStateChange(StreamLocalState state) {
  util::AutoLock autolock(mutex_state_);
  StreamLocalState latest_state =
      (current_state_ == dest_state_) ? current_state_ : dest_state_;
  switch (latest_state) {
    case kStreamLocalStateNotInit:
      if (state != kStreamLocalStateInit) {
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation,
            "invalid set state(%d)", state);
      }
      break;
    case kStreamLocalStateInit:
      if ((state != kStreamLocalStateNotInit) &&
          (state != kStreamLocalStateReady)) {
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation,
            "invalid set state(%d)", state);
      }
      break;
    case kStreamLocalStateReady:
      if ((state != kStreamLocalStateInit) &&
          (state != kStreamLocalStateRunning)) {
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation,
            "invalid set state(%d)", state);
      }
      break;
    case kStreamLocalStateRunning:
      if (state != kStreamLocalStateReady) {
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation,
            "invalid set state(%d)", state);
      }
      break;
    default:
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument,
          "unknown state(%d)", state);
  }
  current_state_ = latest_state;
  dest_state_ = state;
  return Status::OK();
}

/**
 * @brief Commit the state change.
 */
void StreamFunctionLockManager::CommitStateChange() {
  util::AutoLock autolock(mutex_state_);
  current_state_ = dest_state_;
}

/**
 * @brief Cancel the state change.
 */
void StreamFunctionLockManager::CancelStateChange() {
  util::AutoLock autolock(mutex_state_);
  dest_state_ = current_state_;
}

/**
 * @brief Acquire the lock of the function.
 * @param[in] (manager) Lock manager.
 * @param[in] (type) Type of function.
 */
StreamFunctionLock::StreamFunctionLock(
    StreamFunctionLockManager* manager, StreamFunctionType type)
    : manager_(manager), locked_(false) {
  if (manager_ == NULL) {
    status_ = SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "manager == NULL");
    return;
  }
  // check double lock
  if (manager_->IsLockedThisThread()) {
    return;
  }
  switch (type) {
    case kStreamFunctionTypeState:
      status_ = manager->LockForState();
      locked_ = true;
      break;
    case kStreamFunctionTypeInternal:
      status_ = manager->LockForInternal();
      locked_ = true;
      break;
    case kStreamFunctionTypeComponent:
      status_ = manager->LockForComponent();
      locked_ = true;
      break;
    default:
      status_ = SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument, "invalid function type(%d)", type);
      break;
  }
}

/**
 * @brief Release the lock of the function.
 */
StreamFunctionLock::~StreamFunctionLock() {
  if (locked_) {
    manager_->Unlock();
  }
}

/**
 * @brief Get the lock status.
 * @return Status object.
 */
Status StreamFunctionLock::GetStatus() const {
  return status_;
}

}  // namespace senscord
