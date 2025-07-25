/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <Windows.h>
#include "senscord/osal.h"

namespace senscord {
namespace osal {

/**
 * @brief Constructor.
 */
OSExclusiveLock::OSExclusiveLock() : lock_object_(NULL) {
  lock_object_ = new CRITICAL_SECTION;
  CRITICAL_SECTION* cs = reinterpret_cast<CRITICAL_SECTION*>(lock_object_);
  InitializeCriticalSection(cs);
}

/**
 * @brief Destrouctor.
 */
OSExclusiveLock::~OSExclusiveLock() {
  CRITICAL_SECTION* cs = reinterpret_cast<CRITICAL_SECTION*>(lock_object_);
  DeleteCriticalSection(cs);
  delete cs;
}

/**
 * @brief Exclusive lock.
 */
void OSExclusiveLock::Lock() {
  CRITICAL_SECTION* cs = reinterpret_cast<CRITICAL_SECTION*>(lock_object_);
  EnterCriticalSection(cs);
}

/**
 * @brief Exclusive unlock.
 */
void OSExclusiveLock::Unlock() {
  CRITICAL_SECTION* cs = reinterpret_cast<CRITICAL_SECTION*>(lock_object_);
  LeaveCriticalSection(cs);
}

}  //  namespace osal
}  //  namespace senscord
