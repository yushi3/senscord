/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core/core_function_lock_manager.h"

#include <inttypes.h>
#include <sstream>

#include "core/config_manager.h"
#include "util/autolock.h"

namespace senscord {

/**
 * @brief Constructor.
 */
CoreFunctionLockManager::CoreFunctionLockManager() {
  mutex_ = new util::Mutex();
  cond_ = NULL;
  osal::OSCreateCond(&cond_);
  core_initialized_ = false;
}

/**
 * @brief Destructor.
 */
CoreFunctionLockManager::~CoreFunctionLockManager() {
  osal::OSDestroyCond(cond_);
  cond_ = NULL;
  delete mutex_;
  mutex_ = NULL;
}

/**
 * @brief Get the initialization state of the Core class.
 */
bool CoreFunctionLockManager::IsCoreInitialized() const {
  util::AutoLock autolock(mutex_);
  return core_initialized_;
}

/**
 * @brief Set the initialization state of the Core class.
 */
void CoreFunctionLockManager::SetCoreInitialized(bool initialized) {
  util::AutoLock autolock(mutex_);
  core_initialized_ = initialized;
}

/**
 * @brief Insert the closing stream information.
 * @param[in] stream target stream isntance.
 */
void CoreFunctionLockManager::InsertClosingStream(Stream* stream) {
  util::AutoLock autolock(mutex_);
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);
  CloseStatusList::iterator found = close_stream_info_.find(stream);
  if (found == close_stream_info_.end()) {
    // first call
    CloseStreamStatusInfo info = {};
    info.closing_threads.insert(thread);
    close_stream_info_[stream] = info;
  } else {
    // already running (another thread)
    CloseStreamStatusInfo* info = &found->second;
    info->closing_threads.insert(thread);
  }
}

/**
 * @brief Remove the closing stream information.
 * @param[in] stream target stream isntance.
 */
void CoreFunctionLockManager::RemoveClosingStream(Stream* stream) {
  util::AutoLock autolock(mutex_);
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);
  CloseStatusList::iterator found = close_stream_info_.find(stream);
  if (found == close_stream_info_.end()) {
    SENSCORD_LOG_ERROR("stream information not found");
    return;
  }
  CloseStreamStatusInfo* info = &found->second;
  info->closing_threads.erase(thread);
  if (info->closing_threads.size() == 0) {
    // last call, erase resource
    close_stream_info_.erase(found);
  }
}

/**
 * @brief Set the closed stream status.
 * @param[in] stream  target stream isntance.
 * @param[in] status  close stream status.
 */
void CoreFunctionLockManager::SetCloseStreamStatus(
    Stream* stream, const Status& status) {
  util::AutoLock autolock(mutex_);
  CloseStatusList::iterator found = close_stream_info_.find(stream);
  if (found == close_stream_info_.end()) {
    SENSCORD_LOG_ERROR("stream information not found");
    return;
  }
  CloseStreamStatusInfo* info = &found->second;
  info->status = status;
  info->closed = true;
}

/**
 * @brief Get the closed stream status.
 * @param[in]  stream     target stream isntance.
 * @param[out] is_closed  true is already running close stream.
 * @return closed status (when is_closed is true only).
 */
Status CoreFunctionLockManager::GetCloseStreamStatus(
    Stream* stream, bool* is_closed) {
  util::AutoLock autolock(mutex_);
  CloseStatusList::iterator found = close_stream_info_.find(stream);
  if (found == close_stream_info_.end()) {
    *is_closed = false;
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "stream information not found");
  }
  CloseStreamStatusInfo* info = &found->second;
  *is_closed = info->closed;
  return info->status;
}

/**
 * @brief Insert data into the function list.
 * @param[in] type  Type of function.
 * @param[in] key   Key to identify.
 */
void CoreFunctionLockManager::Insert(CoreFunctionType type,
                                 const std::string& key) {
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);

  FunctionInfo info = {};
  info.type = type;
  info.key = key;
  running_functions_[thread] = info;
}

/**
 * @brief Remove data from the function list.
 */
void CoreFunctionLockManager::Remove() {
  osal::OSThread* thread = NULL;
  osal::OSGetCurrentThread(&thread);
  running_functions_.erase(thread);
}

/**
 * @brief Get condition variable.
 */
void CoreFunctionLockManager::WakeupWaitFunction() {
  osal::OSBroadcastCond(cond_);
}

/**
 * @brief Lock for Init function.
 */
Status CoreFunctionLockManager::LockForInit() {
  util::AutoLock autolock(mutex_);

  // [1] Failure:          contains(Init | ReadOnly | Stream)
  // [2] Wait and recheck: contains(Exit)
  // [3] Success:          empty
  while (!running_functions_.empty()) {
    CoreFunctionLockManager::FunctionList::const_iterator itr;
    for (itr = running_functions_.begin();
         itr != running_functions_.end(); ++itr) {
      if (itr->second.type == kFunctionTypeInit) {
        // Failure.
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation, "initialized by another thread");
      }
      if (itr->second.type == kFunctionTypeReadOnly ||
          itr->second.type == kFunctionTypeStream) {
        // Failure.
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation, "already initialized");
      }
    }
    // Wait and recheck.
    int32_t ret = osal::OSWaitCond(cond_, mutex_->GetObject());
    if (ret < 0) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
          "failed to wait until the state change. (ret=%" PRIx32 ")", ret);
    }
  }

  // After waiting, it fails if it is initialized.
  if (IsCoreInitialized()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "already initialized");
  }

  Insert(kFunctionTypeInit, "");
  return Status::OK();
}

/**
 * @brief Lock for Exit function.
 */
Status CoreFunctionLockManager::LockForExit() {
  util::AutoLock autolock(mutex_);

  // [1] Failure:          contains(Exit)
  // [2] Wait and recheck: contains(Init | ReadOnly | Stream)
  // [3] Success:          empty
  while (!running_functions_.empty()) {
    CoreFunctionLockManager::FunctionList::const_iterator itr;
    for (itr = running_functions_.begin();
         itr != running_functions_.end(); ++itr) {
      if (itr->second.type == kFunctionTypeExit) {
        // Failure.
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation, "terminated by another thread");
      }
    }
    // Wait and recheck.
    int32_t ret = osal::OSWaitCond(cond_, mutex_->GetObject());
    if (ret < 0) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
          "failed to wait until the state change. (ret=%" PRIx32 ")", ret);
    }
  }

  // After waiting, it fails if it is uninitialized.
  if (!IsCoreInitialized()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "not initialized");
  }

  Insert(kFunctionTypeExit, "");
  return Status::OK();
}

/**
 * @brief Lock for ReadOnly functions.
 */
Status CoreFunctionLockManager::LockForReadOnly() {
  util::AutoLock autolock(mutex_);

  // [1] Failure:          contains(Exit)
  // [2] Wait and recheck: contains(Init)
  // [3] Success:          empty or contains(ReadOnly | Stream)
  while (!running_functions_.empty()) {
    bool wait = false;
    CoreFunctionLockManager::FunctionList::const_iterator itr;
    for (itr = running_functions_.begin();
         itr != running_functions_.end(); ++itr) {
      if (itr->second.type == kFunctionTypeExit) {
        // Failure.
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation, "terminated by another thread");
      }
      if (itr->second.type == kFunctionTypeInit) {
        wait = true;
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

  // After waiting, it fails if it is uninitialized.
  if (!IsCoreInitialized()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "not initialized");
  }

  Insert(kFunctionTypeReadOnly, "");
  return Status::OK();
}

/**
 * @brief Lock for Stream functions.
 */
Status CoreFunctionLockManager::LockForStream(
    const std::string& stream_key, const ConfigManager& config_manager) {
  util::AutoLock autolock(mutex_);

  if (!IsCoreInitialized()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "not initialized");
  }
  const StreamSetting* stream_config =
      config_manager.GetStreamConfigByStreamKey(stream_key);
  if (stream_config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound,
        "unable to get config from Stream key : key=%s",
        stream_key.c_str());
  }

  // If it is same stream (instance name, port type, port id), wait.
  std::ostringstream buffer;
  buffer << stream_config->address.instance_name;
  buffer << '.' << stream_config->address.port_type;
  buffer << '.' << stream_config->address.port_id;
  std::string instance_key = buffer.str();

  // [1] Failure:          contains(Exit)
  // [2] Wait and recheck: contains(Init | Stream(same key))
  // [3] Success:          empty or contains(ReadOnly | Stream(different key))
  while (!running_functions_.empty()) {
    bool wait = false;
    CoreFunctionLockManager::FunctionList::const_iterator itr;
    for (itr = running_functions_.begin();
         itr != running_functions_.end(); ++itr) {
      if (itr->second.type == kFunctionTypeExit) {
        // Failure.
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation, "terminated by another thread");
      }
      if (itr->second.type == kFunctionTypeInit) {
        wait = true;
      } else if (itr->second.type == kFunctionTypeStream) {
        if (itr->second.key == instance_key) {
          wait = true;
        }
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

  // After waiting, it fails if it is uninitialized.
  if (!IsCoreInitialized()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "not initialized");
  }

  Insert(kFunctionTypeStream, instance_key);
  return Status::OK();
}

void CoreFunctionLockManager::Unlock(Stream* stream) {
  util::AutoLock autolock(mutex_);
  Remove();
  if (stream != NULL) {
    RemoveClosingStream(stream);
  }
  WakeupWaitFunction();
}

/**
 * @brief Acquire the lock of the function.
 * @param[in] manager  Lock manager.
 * @param[in] type     Type of function.
 *
 * Available types are as follows:
 *  - kFunctionTypeInit
 *  - kFunctionTypeExit
 *  - kFunctionTypeReadOnly
 */
CoreFunctionLock::CoreFunctionLock(
    CoreFunctionLockManager* manager, CoreFunctionType type)
    : manager_(manager), stream_(NULL), status_() {
  if (manager_ == NULL) {
    status_ = SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "manager == NULL");
    return;
  }
  switch (type) {
    case kFunctionTypeInit:
      status_ = manager->LockForInit();
      SENSCORD_STATUS_TRACE(status_);
      break;
    case kFunctionTypeExit:
      status_ = manager->LockForExit();
      SENSCORD_STATUS_TRACE(status_);
      break;
    case kFunctionTypeReadOnly:
      status_ = manager->LockForReadOnly();
      SENSCORD_STATUS_TRACE(status_);
      break;
    default:
      status_ = SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument, "invalid function type");
      break;
  }
}

/**
 * @brief Acquire the lock of the function with stream key.
 * @param[in] manager     Lock manager.
 * @param[in] stream_key  Stream key.
 * @param[in] config_manager  Config manager.
 *
 * The type is fixed to kFunctionTypeStream.
 */
CoreFunctionLock::CoreFunctionLock(
    CoreFunctionLockManager* manager,
    const std::string& stream_key,
    const ConfigManager& config_manager)
    : manager_(manager), stream_(NULL), status_() {
  if (manager_ == NULL) {
    status_ = SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "manager == NULL");
    return;
  }
  status_ = manager->LockForStream(stream_key, config_manager);
  SENSCORD_STATUS_TRACE(status_);
}

/**
 * @brief Acquire the lock of the function with stream instance.
 * @param[in] manager         Lock manager.
 * @param[in] stream_manager  Stream manager.
 * @param[in] stream          Stream instance.
 * @param[in] config_manager  Config manager.
 *
 * The type is fixed to kFunctionTypeStream.
 */
CoreFunctionLock::CoreFunctionLock(
    CoreFunctionLockManager* manager,
    StreamManager* stream_manager, Stream* stream,
    const ConfigManager& config_manager)
    : manager_(manager), stream_(stream), status_() {
  if (manager_ == NULL) {
    status_ = SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "manager == NULL");
    return;
  }
  if (stream_manager == NULL) {
    status_ = SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "stream_manager == NULL");
    return;
  }
  // Check the stream instance and get the stream key.
  std::string stream_key;
  StreamCore* stream_accessor = static_cast<StreamCore*>(stream);
  status_ = stream_manager->GetStreamKey(stream_accessor, &stream_key);
  SENSCORD_STATUS_TRACE(status_);
  if (status_.ok()) {
    manager->InsertClosingStream(stream);
    status_ = manager->LockForStream(stream_key, config_manager);
    SENSCORD_STATUS_TRACE(status_);
  }
}

/**
 * @brief Release the lock of the function.
 */
CoreFunctionLock::~CoreFunctionLock() {
  manager_->Unlock(stream_);
}

/**
 * @brief Get the lock status.
 */
Status CoreFunctionLock::status() const {
  return status_;
}

}  // namespace senscord

