/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
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
 * @brief Sleep for the specified time.
 * @param[in] nano_seconds  Sleep time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSleep(uint64_t nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSSleep;
  HANDLE timer = NULL;

  timer = CreateWaitableTimer(NULL, TRUE, NULL);
  if (timer == NULL) {
    SENSCORD_OSAL_LOG_ERROR(
        "failed (CreateWaitableTimer err=%u)", GetLastError());
    return OSMakeErrorCode(kFuncId, kErrorUnknown);
  }

  int32_t ret = 0;
  LARGE_INTEGER interval;
  interval.QuadPart =
      -1 * static_cast<int64_t>(std::ceil(nano_seconds / 100.0));
  if (!SetWaitableTimer(timer, &interval, 0, NULL, NULL, FALSE)) {
    SENSCORD_OSAL_LOG_ERROR(
        "failed (SetWaitableTimer err=%u)", GetLastError());
    ret = OSMakeErrorCode(kFuncId, kErrorUnknown);
  } else {
    if (WaitForSingleObject(timer, INFINITE) != WAIT_OBJECT_0) {
      SENSCORD_OSAL_LOG_ERROR(
          "failed (WaitForSingleObject err=%u)", GetLastError());
      ret = OSMakeErrorCode(kFuncId, kErrorUnknown);
    }
  }

  CloseHandle(timer);

  return ret;
}

}  // namespace osal
}  // namespace senscord
