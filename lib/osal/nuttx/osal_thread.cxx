/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "./osal_thread.h"

#include <stdint.h>

namespace senscord {
namespace osal {

/**
 * @brief Convert OSThread to thread id.
 */
pthread_t GetThreadId(OSThread* thread) {
  uintptr_t temp = reinterpret_cast<uintptr_t>(thread);
  pthread_t thread_id = static_cast<pthread_t>(temp);
  return thread_id;
}

/**
 * @brief Convert thread id to OSThread.
 */
OSThread* GetOSThread(pthread_t thread_id) {
  uintptr_t temp = static_cast<uintptr_t>(thread_id);
  OSThread* thread = reinterpret_cast<OSThread*>(temp);
  return thread;
}

}  // namespace osal
}  // namespace senscord
