/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/osal.h"
#include "linux/osal_linuxerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Join with a terminated thread.
 * @param[in]  thread        Thread object.
 * @param[in]  nano_seconds  Timeout absolute time, in nanoseconds.
 * @param[out] result        Pointer to the variable that receives the thread
 *                           end result. (optional)
 * @return It always return an unsupported error cord.
 *         Because pthread_timedjoin_np() in this function is Non-POSIX.
 */
int32_t OSTimedJoinThread(OSThread* thread,
                          uint64_t nano_seconds,
                          OSThreadResult* result) {
  static const OSFunctionId kFuncId = kIdOSTimedJoinThread;

  return OSMakeErrorCode(kFuncId, kErrorNotSupported);
}

/**
 * @brief Join with a terminated thread.
 * @param[in]  thread        Thread object.
 * @param[in]  nano_seconds  Timeout relative time, in nanoseconds.
 * @param[out] result        Pointer to the variable that receives the thread
 *                           end result. (optional)
 * @return It always return an unsupported error cord.
 *         Because pthread_timedjoin_np() in this function is Non-POSIX.
 */
int32_t OSRelativeTimedJoinThread(OSThread* thread,
                                  uint64_t nano_seconds,
                                  OSThreadResult* result) {
  static const OSFunctionId kFuncId = kIdOSRelativeTimedJoinThread;

  return OSMakeErrorCode(kFuncId, kErrorNotSupported);
}

}  // namespace osal
}  // namespace senscord
