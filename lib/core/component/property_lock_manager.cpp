/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "component/property_lock_manager.h"

#include <inttypes.h>
#include <string>
#include <vector>

#include "logger/logger.h"
#include "senscord/osal_inttypes.h"
#include "util/autolock.h"
#include "util/property_utils.h"

namespace senscord {

/**
 * @brief Constructor
 * @param[in] (port) Parent component port.
 */
PropertyLockManager::PropertyLockManager(
    ComponentPortCore* port)
    : port_(port)
    , callback_lock_property_(NULL)
    , callback_unlock_property_(NULL) {
  osal::OSCreateCond(&cond_);
}

/**
 * @brief Destructor
 */
PropertyLockManager::~PropertyLockManager() {
  util::AutoLock lock(&mutex_);
  lock_info_map_.clear();
  while (!resources_.empty()) {
    PropertyLockResource* resource = *(resources_.begin());
    resources_.erase(resources_.begin());
    delete resource;
  }
  osal::OSDestroyCond(cond_);
}

/**
 * @brief Lock to access properties.
 * @param[in] (stream) Stream to lock.
 * @param[in] (keys) Target property keys
 * @param[in] (timeout_msec) Time of wait msec if locked already.
 * @param[out] (lock_resource) Locked properties resource.
 * @return Status object.
 */
Status PropertyLockManager::LockProperty(
    const StreamCore* stream,
    const std::set<util::PropertyKey>& keys,
    int32_t timeout_msec,
    PropertyLockResource** lock_resource) {
  util::AutoLock lock(&mutex_);
  // check to double lock
  Status status = CheckDoubleLock(stream, keys);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // setup timeout time
  uint64_t timeout_nsec = 0;
  if (timeout_msec > 0) {
    osal::OSGetTime(&timeout_nsec);
    timeout_nsec += static_cast<uint64_t>(timeout_msec) * 1000000;
  }

  // lock for properties
  std::vector<const util::PropertyKey*> locked_keys;
  for (std::set<util::PropertyKey>::const_iterator itr = keys.begin();
      itr != keys.end(); ++itr) {
    // try to get lock in the process.
    while (1) {
      KeyInfo& key_info = lock_info_map_[itr->GetPropertyKey()];
      // check stream instance
      if (!port_->IsOpenedStream(stream)) {
        status = SENSCORD_STATUS_FAIL(
            kStatusBlockCore, Status::kCauseInvalidOperation,
            "invalid stream core");
        break;
      }
      // if no streams access any property, success lock.
      LockInfo* lock_info = NULL;
      Status lock_status = AcquireLockInfoForLockProperty(
          &key_info, itr->GetAppendInfo(), stream, &lock_info);
      if (lock_status.ok()) {
        locked_keys.push_back(&(*itr));
        break;  // OK
      }
      // if failed to lock, wait to change lock status.
      int32_t ret = 0;
      if (timeout_msec == 0) {
        // polling timeout
        ret = -1;
      } else if (timeout_msec < 0) {
        // forever wait
        ret = osal::OSWaitCond(cond_, mutex_.GetObject());
      } else {
        // timed wait
        ret = osal::OSTimedWaitCond(cond_, mutex_.GetObject(), timeout_nsec);
      }
      // failed to wait condition then cancel to lock.
      if (ret < 0) {
        SENSCORD_LOG_ERROR("%s", lock_status.ToString().c_str());
        status = SENSCORD_STATUS_FAIL(
            kStatusBlockCore, Status::kCauseTimeout,
            "%s", lock_status.message().c_str());
        break;
      }
    }
    if (!status.ok()) {
      break;
    }
  }

  // add resource
  PropertyLockResource* resource = NULL;
  if (status.ok()) {
    resource = new PropertyLockResource();
    resource->keys = keys;
    resource->stream = stream;
  }

  // if the callback registered, call the callback and wait the result.
  if (status.ok()) {
    if (callback_lock_property_) {
      // calculate remain time
      if (timeout_msec > 0) {
        uint64_t current_nsec = 0;
        osal::OSGetTime(&current_nsec);
        timeout_msec =
            static_cast<int32_t>((timeout_nsec - current_nsec) / 1000000);
        if (timeout_msec < 0) {
          status = SENSCORD_STATUS_FAIL(
              kStatusBlockCore, Status::kCauseTimeout,
              "lock property timeout");
        }
      }
      if (status.ok()) {
        std::set<std::string> arg_keys;
        for (std::set<util::PropertyKey>::const_iterator itr = keys.begin();
            itr != keys.end(); ++itr) {
          arg_keys.insert(itr->GetFullKey());
        }
        ComponentPort::LockPropertyArguments args = {};
        args.keys = arg_keys;
        args.lock_resource = resource;
        args.timeout_msec = timeout_msec;
        mutex_.Unlock();
        status = callback_lock_property_(
            port_, args, element_lock_property_.private_data);
        mutex_.Lock();
      }
    }
  }

  // rollback if failed to lock
  if (!status.ok()) {
    for (std::vector<const util::PropertyKey*>::const_iterator
        itr = locked_keys.begin(); itr != locked_keys.end(); ++itr) {
      ReleaseLockInfo(*(*itr));
    }
    delete resource;
    WakeupLockProperty();
  }

  // add resource
  if (status.ok()) {
    resources_.insert(resource);
    *lock_resource = resource;
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Unlock to access properties.
 * @param[in] (stream) Stream to unlock.
 * @param[in] (lock_resource) Resource of lock.
 * @return Status object.
 */
Status PropertyLockManager::UnlockProperty(
    const StreamCore* stream, PropertyLockResource* lock_resource) {
  util::AutoLock lock(&mutex_);
  // check to manage resource
  if (resources_.find(lock_resource) == resources_.end()) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidArgument,
          "%s(%s.%" PRId32 "): invalid resourece: %p(%p)",
          port_->GetInstanceName().c_str(), port_->GetPortType().c_str(),
          port_->GetPortId(), stream, lock_resource);
  }
  // check to locked stream
  for (std::set<util::PropertyKey>::const_iterator itr =
      lock_resource->keys.begin();
      itr != lock_resource->keys.end(); ++itr) {
    LockInfo* info = GetLockInfo(*itr);
    if (info == NULL || info->locked_stream != stream) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidArgument,
          "%s(%s.%" PRId32 "): no locked stream: %p(%p)",
          port_->GetInstanceName().c_str(), port_->GetPortType().c_str(),
          port_->GetPortId(), stream, (info ? info->locked_stream : NULL));
    }
  }

  // if the callback registered, call the callback and wait the result.
  Status status;
  if (callback_unlock_property_) {
    mutex_.Unlock();
    status = callback_unlock_property_(
        port_, lock_resource, element_unlock_property_.private_data);
    mutex_.Lock();
  }

  // unlock properties
  if (status.ok()) {
    for (std::set<util::PropertyKey>::const_iterator itr =
        lock_resource->keys.begin(); itr != lock_resource->keys.end(); ++itr) {
      ReleaseLockInfo(*itr);
    }
    resources_.erase(lock_resource);
    delete lock_resource;
    WakeupLockProperty();
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Force unlock to access properties.
 * @param[in] (stream) Stream to unlock.
 */
void PropertyLockManager::ForceUnlockProperty(const StreamCore* stream) {
  util::AutoLock lock(&mutex_);
  std::set<PropertyLockResource*>::iterator res_itr = resources_.begin();
  while (res_itr != resources_.end()) {
    if ((*res_itr)->stream == stream) {
      PropertyLockResource* resource = *res_itr;
      for (std::set<util::PropertyKey>::const_iterator key_itr =
          resource->keys.begin(); key_itr != resource->keys.end(); ++key_itr) {
        ReleaseLockInfo(*key_itr);
      }
      resources_.erase(res_itr++);
      delete resource;
    } else {
      ++res_itr;
    }
  }
  WakeupLockProperty();
}

/**
 * @brief Get permission to access to property.
 * @param[in] (stream) Owner of access.
 * @param[in] (key) Target property key
 * @param[in] (is_set) True if set property.
 * @param[out] (lock_info) Target lock info.
 * @return Status object.
 */
Status PropertyLockManager::StartPropertyAccess(
    const StreamCore* stream, const util::PropertyKey& key,
    bool is_set, LockInfo** lock_info) {
  Status status;
  if (is_set) {
    util::AutoLock lock(&mutex_);
    LockInfo* info = NULL;
    KeyInfo& key_info = lock_info_map_[key.GetPropertyKey()];
    status = AcquireLockInfoForSetProperty(
        &key_info, key.GetAppendInfo(), stream, &info);
    if (status.ok()) {
      *lock_info = info;
    }
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Release permission to access to property.
 * @param[in] (key) Target property key.
 * @param[in] (lock_info) Lock info for the key accessing.
 */
void PropertyLockManager::EndPropertyAccess(
    const util::PropertyKey& key, LockInfo* lock_info) {
  if (lock_info) {
    util::AutoLock lock(&mutex_);
    --(lock_info->accessing_streams);
    if ((lock_info->accessing_streams == 0) &&
        (lock_info->locked_stream == NULL)) {
      ReleaseLockInfo(key);
    }
    WakeupLockProperty();
  }
}

/**
 * @brief Register the callback for LockProperty
 * @oaram [in] (callback) The callback called by LockProperty.
 * @param [in] (private_data) Value with callback called.
 */
void PropertyLockManager::RegisterLockPropertyCallback(
    ComponentPort::OnLockPropertyCallback callback, void* private_data) {
  util::AutoLock lock(&mutex_);
  callback_lock_property_ = callback;
  element_lock_property_.private_data = private_data;
}

/**
 * @brief Register the callback for UnlockProperty
 * @oaram [in] (callback) The callback called by UnlockProperty.
 * @param [in] (private_data) Value with callback called.
 */
void PropertyLockManager::RegisterUnlockPropertyCallback(
    ComponentPort::OnUnlockPropertyCallback callback, void* private_data) {
  util::AutoLock lock(&mutex_);
  callback_unlock_property_ = callback;
  element_unlock_property_.private_data = private_data;
}

/**
 * @brief Waking up processing waiting at LockProperty.
 *        mutex_ locking should be applied when calling this function.
 */
void PropertyLockManager::WakeupLockProperty() {
  osal::OSBroadcastCond(cond_);
}

/**
 * @brief Check for double-locking keys.
 * @param[in] (stream) Stream to lock.
 * @param[in] (keys) Target property keys
 * @return Status object.
 * @note Need to call this function with mutex_ locked.
 */
Status PropertyLockManager::CheckDoubleLock(
    const StreamCore* stream, const std::set<util::PropertyKey>& keys) const {
  for (std::set<util::PropertyKey>::const_iterator key_itr = keys.begin();
      key_itr != keys.end(); ++key_itr) {
    std::string append_info = key_itr->GetAppendInfo();
    LockInfoMap::const_iterator found =
        lock_info_map_.find(key_itr->GetPropertyKey());
    if (found != lock_info_map_.end()) {
      const KeyInfo& key_info = found->second;
      bool is_double_lock = false;
      if (append_info.empty()) {
        for (KeyInfo::const_iterator info_itr = key_info.begin(),
            info_end = key_info.end(); info_itr != info_end; ++info_itr) {
          is_double_lock = (info_itr->second.locked_stream == stream);
          if (is_double_lock) {
            break;
          }
        }
      } else {
        is_double_lock = (
            CheckDoubleLockInKeyInfo(stream, key_info, append_info) ||
            CheckDoubleLockInKeyInfo(stream, key_info, ""));
      }
      if (is_double_lock) {
        return SENSCORD_STATUS_FAIL(
            kStatusBlockCore, Status::kCauseInvalidOperation,
            "%s(%s.%" PRId32 "): double locking: %s",
            port_->GetInstanceName().c_str(), port_->GetPortType().c_str(),
            port_->GetPortId(), key_itr->GetFullKey().c_str());
      }
    }
  }
  return Status::OK();
}

/**
 * @brief Check for double-locking in KeyInfo.
 * @param[in] (stream) Stream to lock.
 * @param[in] (key_info) Target KeyInfo.
 * @param[in] (append_info) Target append info.
 * @return True is locked.
 * @note Need to call this function with mutex_ locked.
 */
bool PropertyLockManager::CheckDoubleLockInKeyInfo(
    const StreamCore* stream, const KeyInfo& key_info,
    const std::string& append_info) const {
  KeyInfo::const_iterator info_itr = key_info.find(append_info);
  if (info_itr != key_info.end() &&
      info_itr->second.locked_stream == stream) {
    return true;  // double lock
  }
  return false;
}

/**
 * @brief Acquire a lock on the target Key for lock property.
 * @param[in] (key_info) Check to KeyInfo.
 * @param[in] (append_info) Target append info on key.
 * @param[in] (stream) Target stream.
 * @param[out] (lock_info) Locked lock info.
 * @return Status object.
 * @note Need to call this function with mutex_ locked.
 */
Status PropertyLockManager::AcquireLockInfoForLockProperty(
    KeyInfo* key_info, const std::string& append_info,
    const StreamCore* stream, LockInfo** lock_info) {
  Status status;
  if (append_info.empty()) {
    for (KeyInfo::const_iterator itr = key_info->begin(),
        end = key_info->end(); itr != end; ++itr) {
      status = IsLockableLockInfo(itr->second);
      SENSCORD_STATUS_TRACE(status);
      if (!status.ok()) {
        break;
      }
    }
  } else {
    KeyInfo::const_iterator empty_key = key_info->find("");
    if (empty_key != key_info->end()) {
      status = IsLockableLockInfo(empty_key->second);
      SENSCORD_STATUS_TRACE(status);
    }
    if (status.ok()) {
      KeyInfo::const_iterator same_append_key = key_info->find(append_info);
      if (same_append_key != key_info->end()) {
        status = IsLockableLockInfo(same_append_key->second);
        SENSCORD_STATUS_TRACE(status);
      }
    }
  }
  if (status.ok()) {
    // set lock state
    LockInfo* lock = &(*key_info)[append_info];
    lock->locked_stream = stream;
    *lock_info = lock;
  }
  return status;
}

/**
 * @brief Check to see if the same key is locked to and accessing some stream.
 * @param[in] (lock_info) Check to LockInfo.
 * @return Status object.
 * @note Need to call this function with mutex_ locked.
 */
Status PropertyLockManager::IsLockableLockInfo(
    const LockInfo& lock_info) const {
  if (lock_info.locked_stream == NULL && lock_info.accessing_streams == 0) {
    return Status::OK();
  }
  return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidOperation,
        "%s(%s.%" PRId32 "): locked other stream: %p, accessing: %" PRIu32,
        port_->GetInstanceName().c_str(), port_->GetPortType().c_str(),
        port_->GetPortId(), lock_info.locked_stream,
        lock_info.accessing_streams);
}

/**
 * @brief Acquire a lock on the target Key for set property.
 * @param[in] (key_info) Check to KeyInfo.
 * @param[in] (append_info) Target append info on key.
 * @param[in] (stream) Target stream.
 * @param[out] (lock_info) Locked lock info.
 * @return Status object.
 * @note Need to call this function with mutex_ locked.
 */
Status PropertyLockManager::AcquireLockInfoForSetProperty(
    KeyInfo* key_info, const std::string& append_info,
    const StreamCore* stream, LockInfo** lock_info) {
  Status status;
  if (append_info.empty()) {
    for (KeyInfo::const_iterator itr = key_info->begin();
        itr != key_info->end(); ++itr) {
      status = IsLockedOtherStreamLockInfo(stream, itr->second);
      SENSCORD_STATUS_TRACE(status);
      if (!status.ok()) {
        break;
      }
    }
  } else {
    KeyInfo::const_iterator found = key_info->find("");
    if (found != key_info->end()) {
      status = IsLockedOtherStreamLockInfo(stream, found->second);
      SENSCORD_STATUS_TRACE(status);
    }
    if (status.ok()) {
      found = key_info->find(append_info);
      if (found != key_info->end()) {
        status = IsLockedOtherStreamLockInfo(stream, found->second);
        SENSCORD_STATUS_TRACE(status);
      }
    }
  }
  if (status.ok()) {
    // set accessing count
    LockInfo* lock = &(*key_info)[append_info];
    ++(lock->accessing_streams);
    *lock_info = lock;
  }
  return status;
}

/**
 * @brief Check to see if the same key is locked to another stream.
 * @param[in] (stream) Target stream.
 * @param[in] (lock_info) Check to LockInfo.
 * @return Status object.
 * @note Need to call this function with mutex_ locked.
 */
Status PropertyLockManager::IsLockedOtherStreamLockInfo(
    const StreamCore* stream, const LockInfo& lock_info) const {
  if (lock_info.locked_stream != NULL && lock_info.locked_stream != stream) {
  return SENSCORD_STATUS_FAIL(
      kStatusBlockCore, Status::kCauseBusy,
      "%s(%s.%" PRId32 "): property accessing is locked"
      " by other stream: %p",
      port_->GetInstanceName().c_str(), port_->GetPortType().c_str(),
      port_->GetPortId(), lock_info.locked_stream);
  }
  return Status::OK();
}

/**
 * @brief Get lock info.
 * @param[in] (key) Property key.
 * @return Lock info, NULL if not found.
 * @note Need to call this function with mutex_ locked.
 */
PropertyLockManager::LockInfo* PropertyLockManager::GetLockInfo(
    const util::PropertyKey& key) {
  LockInfo* info = NULL;
  LockInfoMap::iterator itr = lock_info_map_.find(key.GetPropertyKey());
  if (itr != lock_info_map_.end()) {
    KeyInfo& key_info = itr->second;
    KeyInfo::iterator found = key_info.find(key.GetAppendInfo());
    if (found != key_info.end()) {
      info = &(found->second);
    }
  }
  return info;
}

/**
 * @brief Release the LockInfo for the target key from the LockInfoMap.
 * @param[in] (key) Property key.
 * @note Need to call this function with mutex_ locked.
 */
void PropertyLockManager::ReleaseLockInfo(const util::PropertyKey& key) {
  LockInfoMap::iterator itr = lock_info_map_.find(key.GetPropertyKey());
  if (itr != lock_info_map_.end()) {
    KeyInfo& key_info = itr->second;
    KeyInfo::iterator lockinfo_itr = key_info.find(key.GetAppendInfo());
    if (lockinfo_itr != key_info.end()) {
      lockinfo_itr->second.locked_stream = NULL;
      SENSCORD_LOG_DEBUG("lockinfo: %s[%s]: %" PRIu32,
          itr->first.c_str(), lockinfo_itr->first.c_str(),
          lockinfo_itr->second.accessing_streams);
      if (lockinfo_itr->second.accessing_streams == 0) {
        key_info.erase(lockinfo_itr);
      }
    }
    SENSCORD_LOG_DEBUG("keyinfo: %s: %" PRIuS,
        itr->first.c_str(), key_info.size());
    if (key_info.empty()) {
      lock_info_map_.erase(itr);
    }
  }
}

}   // namespace senscord
