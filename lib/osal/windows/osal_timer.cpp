/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <Windows.h>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"
#include "windows/osal_winerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Timer registration function.
 */
static void CALLBACK expired(PTP_CALLBACK_INSTANCE instance, PVOID context,
                             PTP_TIMER timer) {
  if (context == NULL) {
    SENSCORD_OSAL_LOG_WARNING("Timer Handler failed");
  } else {
    OSTimer* temporary_timer = reinterpret_cast<OSTimer*>(context);

    temporary_timer->TimerHandler();
  }
}

/**
 * @brief OSTimer constructor.
 */
OSTimer::OSTimer() {
  timer_id_ = NULL;

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
  if (timer_id_ == NULL) {
    PTP_TIMER* temporary_timer = reinterpret_cast<PTP_TIMER*>(
      malloc(sizeof(PTP_TIMER)));
    if (temporary_timer == NULL) {
      result = OSMakeErrorCode(kFuncId, kErrorOutOfMemory);
    } else {
      *temporary_timer = CreateThreadpoolTimer(expired, this, NULL);

      if (*temporary_timer != NULL) {
        ULARGE_INTEGER relative_first_time;
        LONGLONG rel_time = static_cast<LONGLONG>(first_milli_seconds);
        rel_time *= 10000;  // Make it a unit of 100 nanosec.
        rel_time *= -1;  // Minus sign represents relative time.
        relative_first_time.QuadPart = static_cast<ULONGLONG>(rel_time);

        FILETIME first_time;
        first_time.dwHighDateTime = relative_first_time.HighPart;
        first_time.dwLowDateTime = relative_first_time.LowPart;

        DWORD interval_time = static_cast<DWORD>(interval_milli_seconds);
        SetThreadpoolTimer(*temporary_timer, &first_time, interval_time, 0);

        timer_id_ = reinterpret_cast<OSTimerId*>(temporary_timer);
      } else {
        SENSCORD_OSAL_LOG_ERROR(
            "failed (CreateThreadpoolTimer err=%u)", GetLastError());
        free(temporary_timer);
        result = OSMakeErrorCode(kFuncId, kErrorInternal);
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

  if (timer_id_ == NULL) {
    // not running.
    result = OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  } else {
    CloseThreadpoolTimer(*(reinterpret_cast<PTP_TIMER*>(timer_id_)));
    free(timer_id_);
    timer_id_ = NULL;
  }

  OSUnlockMutex(mutex_);

  return result;
}

}  // namespace osal
}  // namespace senscord
