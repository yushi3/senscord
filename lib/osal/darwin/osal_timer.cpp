/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/osal.h"
#include "common/osal_error.h"

namespace senscord {
namespace osal {

/**
 * @brief OSTimer constructor.
 */
OSTimer::OSTimer() : timer_id_(), mutex_() {}

/**
 * @brief OSTimer destructor.
 */
OSTimer::~OSTimer() {}

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
  return OSMakeErrorCode(kFuncId, kErrorNotSupported);
}

/**
 * @brief Stop the timer.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSTimer::StopTimer() {
  static const OSFunctionId kFuncId = kIdOSTimerStopTimer;
  return OSMakeErrorCode(kFuncId, kErrorNotSupported);
}

}  // namespace osal
}  // namespace senscord
