/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "component/stream_source_function_lock_manager.h"

#include <inttypes.h>

#include "util/autolock.h"
#include "senscord/logger.h"

namespace senscord {

/**
 * @brief Constructor.
 */
StreamSourceFunctionLockManager::StreamSourceFunctionLockManager() {
  mutex_ = new util::Mutex();
  cond_ = NULL;
  osal::OSCreateCond(&cond_);
  state_changing_ = false;
}

/**
 * @brief Destructor.
 */
StreamSourceFunctionLockManager::~StreamSourceFunctionLockManager() {
  osal::OSDestroyCond(cond_);
  cond_ = NULL;
  delete mutex_;
  mutex_ = NULL;
}

/**
 * @brief Get the state change in progress.
 * @return true is state changing.
 */
bool StreamSourceFunctionLockManager::IsStateChanging() const {
  util::AutoLock autolock(mutex_);
  return state_changing_;
}

/**
 * @brief Set the state change in progress.
 * @param[in] changing true is state changing.
 */
void StreamSourceFunctionLockManager::SetStateChanging(bool changing) {
  util::AutoLock autolock(mutex_);
  state_changing_ = changing;
}

/**
 * @brief Lock for State function.
 */
void StreamSourceFunctionLockManager::LockForState() {
  util::AutoLock autolock(mutex_);

  // entry accessing function
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);
  Insert(kFunctionTypeState, thread);

  // [1] Failure:          none
  // [2] Wait and recheck: contains(State | ReleaseFrame | Property)
  // [3] Success:          empty
  while (!running_functions_.empty()) {
    // no one else is accessing, so self the first.
    if (running_functions_.begin()->thread == thread) {
      break;
    }
    // Wait.
    int32_t ret = osal::OSWaitCond(cond_, mutex_->GetObject());
    if (ret < 0) {
      SENSCORD_LOG_ERROR(
          "failed to wait until the state change. (ret=%" PRIx32 ")", ret);
      return;
    }
  }
  SetStateChanging(true);
}

/**
 * @brief Lock for ReleaseFrame function.
 */
void StreamSourceFunctionLockManager::LockForReleaseFrame() {
  util::AutoLock autolock(mutex_);

  // entry accessing function
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);
  Insert(kFunctionTypeReleaseFrame, thread);

  // [1] Failure:          none
  // [2] Wait and recheck: contains(State | ReleaseFrame)
  // [3] Success:          empty or Property
  while (!running_functions_.empty()) {
    bool wait = false;
    StreamSourceFunctionLockManager::FunctionList::const_iterator itr;
    for (itr = running_functions_.begin();
         itr != running_functions_.end(); ++itr) {
      // check only those that have been accessed before yourself
      if (itr->thread == thread) {
        break;
      }
      if (itr->type == kFunctionTypeState ||
          itr->type == kFunctionTypeReleaseFrame) {
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
      SENSCORD_LOG_ERROR(
          "failed to wait until the state change. (ret=%" PRIx32 ")", ret);
      return;
    }
  }
}

/**
 * @brief Lock for Property functions.
 */
void StreamSourceFunctionLockManager::LockForProperty() {
  util::AutoLock autolock(mutex_);

  // entry accessing function
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);
  Insert(kFunctionTypeProperty, thread);

  // [1] Failure:          none
  // [2] Wait and recheck: contains(State | Property)
  // [3] Success:          empty or ReleaseFrame
  while (!running_functions_.empty()) {
    bool wait = false;
    StreamSourceFunctionLockManager::FunctionList::const_iterator itr;
    for (itr = running_functions_.begin();
         itr != running_functions_.end(); ++itr) {
      // check only those that have been accessed before yourself
      if (itr->thread == thread) {
        break;
      }
      if (itr->type == kFunctionTypeState ||
          itr->type == kFunctionTypeProperty) {
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
      SENSCORD_LOG_ERROR(
          "failed to wait until the state change. (ret=%" PRIx32 ")", ret);
      return;
    }
  }
}

/**
 * @brief Unlock a locked function.
 */
void StreamSourceFunctionLockManager::Unlock() {
  util::AutoLock autolock(mutex_);
  Remove();
  WakeupWaitFunction();
}

/**
 * @brief Insert data into the function list.
 * @param[in] type  Type of function.
 * @param[in] key   Key to identify.
 */
void StreamSourceFunctionLockManager::Insert(
    StreamSourceFunctionType type, osal::OSThread* thread) {
  FunctionInfo info = {};
  info.type = type;
  info.thread = thread;
  running_functions_.push_back(info);
}

/**
 * @brief Remove data from the function list.
 */
void StreamSourceFunctionLockManager::Remove() {
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);
  StreamSourceFunctionLockManager::FunctionList::iterator itr;
  for (itr = running_functions_.begin();
       itr != running_functions_.end(); ++itr) {
    // check only those that have been accessed before yourself
    if (itr->thread == thread) {
      StreamSourceFunctionType type = itr->type;
      running_functions_.erase(itr);
      if (type == kFunctionTypeState) {
        SetStateChanging(false);
      }
      break;
    }
  }
}

/**
 * @brief Waking up the other accesses that are waiting.
 */
void StreamSourceFunctionLockManager::WakeupWaitFunction() {
  osal::OSBroadcastCond(cond_);
}

/**
 * @brief Acquire the lock of the function.
 * @param[in] manager  Lock manager.
 * @param[in] type     Type of function.
 */
StreamSourceFunctionLock::StreamSourceFunctionLock(
    StreamSourceFunctionLockManager* manager, StreamSourceFunctionType type)
    : manager_(manager) {
  if (manager_ == NULL) {
    SENSCORD_LOG_ERROR("manager == NULL");
    return;
  }
  switch (type) {
    case kFunctionTypeState:
      manager_->LockForState();
      break;
    case kFunctionTypeReleaseFrame:
      manager_->LockForReleaseFrame();
      break;
    case kFunctionTypeProperty:
      manager_->LockForProperty();
      break;
    default:
      SENSCORD_LOG_ERROR("invalid function type: %d", type);
      break;
  }
}

/**
 * @brief Release the lock of the function.
 */
StreamSourceFunctionLock::~StreamSourceFunctionLock() {
  manager_->Unlock();
}

}  // namespace senscord
