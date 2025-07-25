/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <inttypes.h>

#include "senscord/osal.h"
#include "common/osal_logger.h"
#include "linux/osal_linuxerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Convert OSThread to thread id.
 */
extern pthread_t GetThreadId(OSThread* thread);

/**
 * @brief Join with a terminated thread.
 * @param[in]  thread        Thread object.
 * @param[in]  nano_seconds  Timeout absolute time, in nanoseconds.
 * @param[out] result        Pointer to the variable that receives the thread
 *                           end result. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSTimedJoinThread(OSThread* thread,
                          uint64_t nano_seconds,
                          OSThreadResult* result) {
  static const OSFunctionId kFuncId = kIdOSTimedJoinThread;
  pthread_t thread_id = GetThreadId(thread);

  lldiv_t ans = lldiv(nano_seconds, 1000 * 1000 * 1000);
  struct timespec wait_time;
  wait_time.tv_sec = ans.quot;
  wait_time.tv_nsec = ans.rem;

  void* thread_result = 0;
  int32_t ret = pthread_timedjoin_np(thread_id, &thread_result, &wait_time);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(ret);
    return OSMakeErrorCode(kFuncId, cause);
  }

  if (result != NULL) {
    *result = reinterpret_cast<OSThreadResult>(thread_result);
  }

  return 0;
}

/**
 * @brief Join with a terminated thread.
 * @param[in]  thread        Thread object.
 * @param[in]  nano_seconds  Timeout relative time, in nanoseconds.
 * @param[out] result        Pointer to the variable that receives the thread
 *                           end result. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSRelativeTimedJoinThread(OSThread* thread,
                                  uint64_t nano_seconds,
                                  OSThreadResult* result) {
  static const OSFunctionId kFuncId = kIdOSRelativeTimedJoinThread;
  uint64_t wait_time = 0;
  int32_t res = OSGetTime(&wait_time);
  if (res != 0) {
    SENSCORD_OSAL_LOG_ERROR("OSGetTime failed. ret=0x%" PRIx32, res);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }

  wait_time += nano_seconds;
  res = OSTimedJoinThread(thread, wait_time, result);
  if (res != 0) {
    OSErrorCause cause = OSGetErrorCause(res);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

}  // namespace osal
}  // namespace senscord
