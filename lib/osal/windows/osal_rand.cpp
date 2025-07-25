/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define _CRT_RAND_S
#include <stdlib.h>
#include "senscord/osal.h"
#include "common/osal_error.h"
#include "windows/osal_winerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Generate a random number.
 * Returns a random value between kOSRandMin and kOSRandMax.
 * @param[out] random_val  Pointer to the variable that receives the random
 *                         value.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSRand(uint16_t* random_val) {
  static const OSFunctionId kFuncId = kIdOSRand;
  if (random_val == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  unsigned int random_val_tmp;
  errno_t result = rand_s(&random_val_tmp);

  if (result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(result);
    return OSMakeErrorCode(kFuncId, cause);
  }

  *random_val = (random_val_tmp % (kOSRandMax - kOSRandMin + 1)) + kOSRandMin;

  return 0;
}

}  // namespace osal
}  // namespace senscord
