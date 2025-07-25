/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_WAMR_SRC_SENSCORD_WAMR_UTIL_H_
#define LIB_WAMR_SRC_SENSCORD_WAMR_UTIL_H_

#include <stdint.h>

#include "senscord/osal.h"
#include "senscord/c_api/senscord_c_api.h"

/**
 * @brief Lock guard.
 */
class LockGuard {
 public:
  explicit LockGuard(senscord::osal::OSMutex* mutex) : mutex_(mutex) {
    senscord::osal::OSLockMutex(mutex_);
  }

  ~LockGuard() {
    senscord::osal::OSUnlockMutex(mutex_);
  }

 private:
  senscord::osal::OSMutex* mutex_;
};

/**
 * @brief Thread environment initializer.
 */
class WasmThreadEnv {
 public:
  WasmThreadEnv();
  ~WasmThreadEnv();

 private:
  bool thread_env_inited_;
};

/**
 * @brief Get stream key.
 * @param[in] stream  Stream handle.
 * @return Stream key.
 */
const char* senscord_stream_get_key(senscord_stream_t stream);

/**
 * @brief Get parent stream handle.
 * @param[in] frame  Frame handle.
 * @return Stream handle.
 */
senscord_stream_t senscord_frame_get_parent_stream(senscord_frame_t frame);

#endif  // LIB_WAMR_SRC_SENSCORD_WAMR_UTIL_H_
