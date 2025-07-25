/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "senscord/osal.h"
#include "./osal_error.h"
#include "./osal_linuxerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Sleep for the specified time.
 * @param[in] nano_seconds  Sleep time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSleep(uint64_t nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSSleep;
  struct timespec req;

  lldiv_t div_result = lldiv(nano_seconds, (1000 * 1000 * 1000));
  req.tv_sec = div_result.quot;
  req.tv_nsec = div_result.rem;

  int result = nanosleep(&req, NULL);

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  return 0;
}

}  // namespace osal
}  // namespace senscord
