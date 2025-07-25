/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"
#include "linux/osal_linuxerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Timer registration function.
 */
static void expired(sigval_t sigval) {
  if (sigval.sival_ptr == NULL) {
    SENSCORD_OSAL_LOG_WARNING("Timer Handler failed");
  } else {
    OSTimer *instance = reinterpret_cast<OSTimer*>(sigval.sival_ptr);

    instance->TimerHandler();
  }
}

/**
 * @brief OSTimer constructor.
 */
OSTimer::OSTimer() {
  timer_id_ = 0;

  OSCreateMutex(&mutex_);
}

/**
 * @brief OSTimer destructor.
 */
OSTimer::~OSTimer() {
  StopTimer();

  OSDestroyMutex(mutex_);
}

/**
 * @brief Start the timer.
 * @param[in] first_milli_seconds    First interval, in milliseconds.
 * @param[in] interval_milli_seconds Second and subsequent intervals,
 *                                   in milliseconds.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSTimer::StartTimer(uint64_t first_milli_seconds,
                            uint64_t interval_milli_seconds) {
  static const OSFunctionId kFuncId = kIdOSTimerStartTimer;
  if (first_milli_seconds == 0) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int result = 0;

  OSLockMutex(mutex_);
  if (timer_id_ == 0) {
    struct sigevent sev;
    sev.sigev_value.sival_ptr = reinterpret_cast<void *>(this);
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = expired;
    sev.sigev_notify_attributes = NULL;

    timer_t* timer_id_tmp = reinterpret_cast<timer_t*>(
        malloc(sizeof(timer_t)));
    if (timer_id_tmp == NULL) {
      result = OSMakeErrorCode(kFuncId, kErrorOutOfMemory);
    } else {
      result = timer_create(CLOCK_REALTIME, &sev, timer_id_tmp);
      if (result == 0) {
        itimerspec value, ovalue;

        value.it_value.tv_sec  = first_milli_seconds / 1000;
        value.it_value.tv_nsec = (first_milli_seconds % 1000) * 1000 * 1000;

        value.it_interval.tv_sec  = interval_milli_seconds / 1000;
        value.it_interval.tv_nsec = (interval_milli_seconds % 1000)
            * 1000 * 1000;

        result = timer_settime(*timer_id_tmp, 0, &value, &ovalue);

        if (result == 0) {
          timer_id_ = reinterpret_cast<OSTimerId*>(timer_id_tmp);
        } else {
          OSErrorCause cause = GetErrorCauseFromErrno(errno);
          timer_delete(*timer_id_tmp);
          free(timer_id_tmp);
          result = OSMakeErrorCode(kFuncId, cause);
        }
      } else {
        OSErrorCause cause = GetErrorCauseFromErrno(errno);
        free(timer_id_tmp);
        result = OSMakeErrorCode(kFuncId, cause);
      }
    }
  } else {
    // already running.
    result = OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }

  OSUnlockMutex(mutex_);

  return result;
}

/**
 * @brief Stop the timer.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSTimer::StopTimer() {
  static const OSFunctionId kFuncId = kIdOSTimerStopTimer;
  int result = 0;

  OSLockMutex(mutex_);

  if (timer_id_ == 0) {
    // not running.
    result = OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  } else {
    result = timer_delete(*(reinterpret_cast<timer_t*>(timer_id_)));

    if (result == 0) {
      free(timer_id_);
      timer_id_ = 0;
    } else {
      OSErrorCause cause = GetErrorCauseFromErrno(errno);
      result = OSMakeErrorCode(kFuncId, cause);
    }
  }

  OSUnlockMutex(mutex_);

  return result;
}

}  // namespace osal
}  // namespace senscord
