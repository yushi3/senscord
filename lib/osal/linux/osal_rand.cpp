/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <inttypes.h>
#include "senscord/osal.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"

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

  uint64_t nano_seconds = 0;
  int32_t result = OSGetTime(&nano_seconds);
  if (result != 0) {
    SENSCORD_OSAL_LOG_ERROR("OSGetTime failed. ret=0x%" PRIx32, result);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }

  srandom(static_cast<unsigned>(nano_seconds));
  *random_val = (static_cast<uint16_t>(random()) %
      (kOSRandMax - kOSRandMin + 1)) + kOSRandMin;
  return 0;
}

}  // namespace osal
}  // namespace senscord
