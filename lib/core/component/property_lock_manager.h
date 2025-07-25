/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_COMPONENT_PROPERTY_LOCK_MANAGER_H_
#define LIB_CORE_COMPONENT_PROPERTY_LOCK_MANAGER_H_

#include <string>
#include <map>
#include <set>

#include "senscord/osal.h"
#include "senscord/noncopyable.h"
#include "senscord/develop/component_port.h"
#include "stream/stream_core.h"
#include "component/component_port_core.h"
#include "util/mutex.h"
#include "util/property_utils.h"

namespace senscord {

class ComponentPortCore;

/**
 * @brief Locked properties resource.
 */
struct PropertyLockResource {
  const StreamCore* stream;     /**< Stream holding resources. */
  std::set<util::PropertyKey> keys;   /**< Property keys for lock targets. */
};

/**
 * @brief Lock property manager class.
 */
class PropertyLockManager : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   * @param[in] (port) Parent component port.
   */
  explicit PropertyLockManager(ComponentPortCore* port);

  /**
   * @brief Destructor.
   */
  virtual ~PropertyLockManager();

  /**
   * @brief Lock info.
   */
  struct LockInfo {
    const StreamCore* locked_stream;    /**< Locked stream. */
    uint32_t accessing_streams;         /**< Number of accessing streams. */
  };

  /**
   * @brief Lock to access properties.
   * @param[in] (stream) Stream to lock.
   * @param[in] (keys) Property keys for lock targets.
   * @param[in] (timeout_msec) Time of wait msec if locked already.
   * @param[out] (lock_resource) Locked properties resource.
   * @return Status object.
   */
  Status LockProperty(
      const StreamCore* stream,
      const std::set<util::PropertyKey>& keys,
      int32_t timeout_msec,
      PropertyLockResource** lock_resource);

  /**
   * @brief Unlock to access properties.
   * @param[in] (stream) Stream to unlock.
   * @param[in] (lock_resource) Locked properties resource.
   * @return Status object.
   */
  Status UnlockProperty(
      const StreamCore* stream,
      PropertyLockResource* lock_resource);

  /**
   * @brief Force unlock to access properties.
   * @param[in] (stream) Stream to unlock.
   */
  void ForceUnlockProperty(const StreamCore* stream);

  /**
   * @brief Get permission to access to property.
   * @param[in] (stream) Owner of access.
   * @param[in] (key) Property key.
   * @param[in] (is_set) The flag for set property.
   * @param[out] (lock_info) Target lock info.
   * @return Status object.
   */
  Status StartPropertyAccess(
      const StreamCore* stream,
      const util::PropertyKey& key,
      bool is_set,
      LockInfo** lock_info);

  /**
   * @brief Release permission to access to property on this port.
   * @param[in] (key) Property key.
   * @param[in] (lock_info) Target lock info.
   */
  void EndPropertyAccess(const util::PropertyKey& key, LockInfo* lock_info);

  /**
   * @brief Register the callback for LockProperty
   * @param [in] (callback) The callback called by LockProperty.
   * @param [in] (private_data) Value with callback called.
   */
  virtual void RegisterLockPropertyCallback(
    ComponentPort::OnLockPropertyCallback callback, void* private_data);

  /**
   * @brief Register the callback for UnlockProperty
   * @param [in] (callback) The callback called by UnlockProperty.
   * @param [in] (private_data) Value with callback called.
   */
  virtual void RegisterUnlockPropertyCallback(
    ComponentPort::OnUnlockPropertyCallback callback, void* private_data);

 private:
  /**
   * @brief Waking up processing waiting at LockProperty.
   */
  void WakeupLockProperty();

  /**
   * @brief Check for double-locking keys.
   * @param[in] (stream) Stream to lock.
   * @param[in] (keys) Target property keys
   * @return Status object.
   * @note Need to call this function with mutex_ locked.
   */
  Status CheckDoubleLock(
      const StreamCore* stream, const std::set<util::PropertyKey>& keys) const;

  //// lock info
  // key: append info, value: lock info
  typedef std::map<std::string, LockInfo> KeyInfo;
  // key: property key, value: key info
  typedef std::map<std::string, KeyInfo> LockInfoMap;
  // lock info map
  LockInfoMap lock_info_map_;

  /**
   * @brief Check for double-locking in KeyInfo.
   * @param[in] (stream) Stream to lock.
   * @param[in] (key_info) Target KeyInfo.
   * @param[in] (append_info) Target append info.
   * @return True is locked.
   * @note Need to call this function with mutex_ locked.
   */
  bool CheckDoubleLockInKeyInfo(
      const StreamCore* stream,
      const KeyInfo& key_info,
      const std::string& append_info) const;

  /**
   * @brief Acquire a lock on the target Key.
   * @param[in] (key_info) Check to KeyInfo.
   * @param[in] (append_info) Target append info on key.
   * @param[in] (stream) Target stream.
   * @param[out] (lock_info) Locked lock info.
   * @return Status object.
   * @note Need to call this function with mutex_ locked.
   */
  Status AcquireLockInfoForLockProperty(
      KeyInfo* key_info, const std::string& append_info,
      const StreamCore* stream, LockInfo** lock_info);

  /**
   * @brief Check to see if the same key is locked to and
   *        accessing some stream.
   * @param[in] (lock_info) Check to LockInfo.
   * @return Status object.
   * @note Need to call this function with mutex_ locked.
   */
  Status IsLockableLockInfo(const LockInfo& lock_info) const;

  /**
   * @brief Acquire a lock on the target Key for set property.
   * @param[in] (key_info) Check to KeyInfo.
   * @param[in] (append_info) Target append info on key.
   * @param[in] (stream) Target stream.
   * @param[out] (lock_info) Locked lock info.
   * @return Status object.
   * @note Need to call this function with mutex_ locked.
   */
  Status AcquireLockInfoForSetProperty(
      KeyInfo* key_info, const std::string& append_info,
      const StreamCore* stream, LockInfo** lock_info);

  /**
   * @brief Check to see if the same key is locked to another stream.
   * @param[in] (stream) Target stream.
   * @param[in] (lock_info) Check to LockInfo.
   * @return Status object.
   * @note Need to call this function with mutex_ locked.
   */
  Status IsLockedOtherStreamLockInfo(
      const StreamCore* stream, const LockInfo& lock_info) const;

  /**
   * @brief Get lock info.
   * @param[in] (key) Property key.
   * @return Lock info, NULL if not found.
   * @note Need to call this function with mutex_ locked.
   */
  LockInfo* GetLockInfo(const util::PropertyKey& key);

  /**
   * @brief Remove unreferenced resources.
   * @param[in] (key) Property key.
   * @note Need to call this function with mutex_ locked.
   */
  void ReleaseLockInfo(const util::PropertyKey& key);

  // parent component
  ComponentPortCore* port_;

  // resource list
  std::set<PropertyLockResource*> resources_;

  // property locked stream mutex
  util::Mutex mutex_;

  // wait condition
  osal::OSCond* cond_;

  // the element of callback
  struct CallbackElement {
    void* private_data;     /**< Private data. */
  };

  // for lock property callback
  ComponentPort::OnLockPropertyCallback callback_lock_property_;
  CallbackElement element_lock_property_;

  // for unlock property callback
  ComponentPort::OnUnlockPropertyCallback callback_unlock_property_;
  CallbackElement element_unlock_property_;
};

/**
 * @brief Auto property lock utility.
 */
class PropertyLocker {
 public:
  /**
   * @brief Constructor.
    * @param[in] (lock_mgr) Property lock manager.
    * @param[in] (stream) Owner of access.
    * @param[in] (property_key) Property key.
    * @param[in] (is_set) The flag for set property.
   */
  explicit PropertyLocker(
      PropertyLockManager* lock_mgr, const StreamCore* stream,
      const util::PropertyKey& property_key, bool is_set) :
        lock_mgr_(lock_mgr), property_key_(property_key), lock_info_() {
    status_ = lock_mgr->StartPropertyAccess(
        stream, property_key, is_set, &lock_info_);
  }

  /**
   * @brief Destructor.
   */
  ~PropertyLocker() {
    if (status_.ok()) {
      lock_mgr_->EndPropertyAccess(property_key_, lock_info_);
    }
  }

  /**
  * @brief Get the lock status.
  * @return Status object.
  */
  Status GetStatus() const {
    return status_;
  }

 private:
  PropertyLockManager* lock_mgr_;
  const util::PropertyKey& property_key_;
  PropertyLockManager::LockInfo* lock_info_;
  Status status_;
};

}   // namespace senscord
#endif  // LIB_CORE_COMPONENT_PROPERTY_LOCK_MANAGER_H_
