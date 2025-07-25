/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <inttypes.h>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"
#include "darwin/osal_darwinerror.h"

namespace {
  static const uint64_t kIntervalMsec = 10ULL * 1000 * 1000;  // 10ms
}

namespace senscord {
namespace osal {

/**
 * @brief Lock a mutex. (specify absolute time)
 * @param[in] mutex         Mutex object to lock.
 * @param[in] nano_seconds  Timeout absolute time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSTimedLockMutex(OSMutex* mutex, uint64_t nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSTimedLockMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  uint64_t abs_timeout = nano_seconds;
  // if it has already elapsed, don't lock
  uint64_t current_ts = 0;
  OSGetTime(&current_ts);
  if (abs_timeout <= current_ts) {
    return OSMakeErrorCode(kFuncId, kErrorTimedOut);
  }

  // retry until a lock is obtained
  uint64_t prev_ts = current_ts;
  int ret = 0;
  while (true) {
    ret = pthread_mutex_trylock(reinterpret_cast<pthread_mutex_t*>(mutex));
    if (ret != EBUSY) {
      break;
    }
    OSGetTime(&current_ts);
    if (abs_timeout <= current_ts) {
      return OSMakeErrorCode(kFuncId, kErrorTimedOut);
    }
    uint64_t diff = current_ts - prev_ts;
    uint64_t wait_nsec = (diff < kIntervalMsec) ? kIntervalMsec - diff : 0;
    if (0 < wait_nsec) {
      OSSleep(wait_nsec);
    }
    prev_ts = current_ts;
  }

  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(ret);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

}  //  namespace osal
}  //  namespace senscord
