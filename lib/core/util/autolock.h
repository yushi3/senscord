/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_UTIL_AUTOLOCK_H_
#define LIB_CORE_UTIL_AUTOLOCK_H_

#include "senscord/noncopyable.h"
#include "util/mutex.h"

namespace senscord {
namespace util {

/**
 * @brief Auto lock utillity class.
 */
class AutoLock : private Noncopyable {
 public:
  // auto lock
  explicit AutoLock(Mutex* lock) : lock_(lock) {
    lock_->Lock();
  }

  // auto unlock
  ~AutoLock() {
    lock_->Unlock();
  }

 private:
  // lock object
  Mutex* lock_;
};

}   // namespace util
}   // namespace senscord
#endif  // LIB_CORE_UTIL_AUTOLOCK_H_
