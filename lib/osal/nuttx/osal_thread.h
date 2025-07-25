/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_OSAL_NUTTX_OSAL_THREAD_H_
#define LIB_OSAL_NUTTX_OSAL_THREAD_H_

#include <pthread.h>

#include "senscord/osal.h"

namespace senscord {
namespace osal {

/**
 * @brief Convert OSThread to thread id.
 */
pthread_t GetThreadId(OSThread* thread);

/**
 * @brief Convert thread id to OSThread.
 */
OSThread* GetOSThread(pthread_t thread_id);

}  // namespace osal
}  // namespace senscord

#endif  // LIB_OSAL_NUTTX_OSAL_THREAD_H_
