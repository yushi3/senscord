/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CONNECTION_WS_INCLUDE_AUTO_MUTEX_H
#define LIB_CONNECTION_WS_INCLUDE_AUTO_MUTEX_H

#include "senscord/noncopyable.h"
#include "senscord/osal.h"

namespace senscord {

/**
 * @brief RAII-style mutex lock class.
 */
class AutoMutex : private util::Noncopyable {
 public:
  explicit AutoMutex(osal::OSMutex* mutex) : auto_mutex_(mutex) {
    osal::OSLockMutex(auto_mutex_);
  }

  ~AutoMutex() {
    osal::OSUnlockMutex(auto_mutex_);
  }

 private:
  osal::OSMutex* auto_mutex_;
};

}  // namespace senscord

#endif  // LIB_CONNECTION_WS_INCLUDE_AUTO_MUTEX_H
