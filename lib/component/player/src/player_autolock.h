/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_PLAYER_SRC_PLAYER_AUTOLOCK_H_
#define LIB_COMPONENT_PLAYER_SRC_PLAYER_AUTOLOCK_H_

#include "senscord/noncopyable.h"
#include "senscord/osal.h"

namespace player {

/**
 * @brief Auto lock utility class.
 */
class AutoLock : private senscord::util::Noncopyable {
 public:
  // auto lock
  explicit AutoLock(senscord::osal::OSMutex* lock) : lock_(lock) {
    senscord::osal::OSLockMutex(lock_);
  }

  // auto unlock
  ~AutoLock() {
    senscord::osal::OSUnlockMutex(lock_);
  }

 private:
  // lock object
  senscord::osal::OSMutex* lock_;
};

}   // namespace player
#endif  // LIB_COMPONENT_PLAYER_SRC_PLAYER_AUTOLOCK_H_
