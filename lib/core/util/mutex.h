/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_UTIL_MUTEX_H_
#define LIB_CORE_UTIL_MUTEX_H_

#include <stdint.h>
#include "senscord/noncopyable.h"
#include "senscord/osal.h"

namespace senscord {
namespace util {

/**
 * @brief Static lock object class.
 */
class Mutex : private Noncopyable {
 public:
  Mutex() : lock_(NULL) {
    osal::OSCreateMutex(&lock_);
  }

  ~Mutex() {
    osal::OSDestroyMutex(lock_);
  }

  /**
   * @brief Lock mutex.
   * @return 0 is success or error code.
   */
  int32_t Lock() {
    return osal::OSLockMutex(lock_);
  }
#if 0
  /**
   * @brief Lock mutex.
   * @param (timeout_msec) Wait time by msec. 0 is polling and minus is forever.
   * @return 0 is success or error code.
   */
  int32_t Lock(int32_t timeout_msec) {
    if (timeout_msec > 0) {
      uint64_t timeout_nsec = static_cast<uint64_t>(timeout_msec) * 1000 * 1000;
      return osal::OSRelativeTimedLockMutex(lock_, timeout_nsec);
    } else if (timeout_msec == 0) {
      // try lock
      return osal::OSTryLockMutex(lock_);
    } else {
      // forever wait lock
      return osal::OSLockMutex(lock_);
    }
  }
#endif

  /**
   * @brief Unlock mutex.
   * @return 0 is success or error code.
   */
  int32_t Unlock() {
    return osal::OSUnlockMutex(lock_);
  }

  /**
   * @brief Get mutex object
   * @return mutex object by OSAL
   */
  osal::OSMutex* GetObject() const {
    return lock_;
  }

 private:
  // lock object
  osal::OSMutex* lock_;
};

}   // namespace util
}   // namespace senscord
#endif  // LIB_CORE_UTIL_MUTEX_H_
