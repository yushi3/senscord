/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/osal_error.h"

namespace senscord {
namespace osal {

/**
 * @brief Get the error status.
 * @param[in] cause  OSAL error cause.
 * @return Error status (defined in "osal_error.h")
 *         If the error cause is kErrorNone, it returns success value (0).
 */
int32_t OSGetErrorStatus(OSErrorCause cause) {
  struct ErrorStatusMap {
    OSErrorCause cause;
    int32_t error_status;
  };
  const ErrorStatusMap kTable[] = {
      { kErrorNone,                 error::kOk },
      { kErrorInvalidObject,        error::kStatusParam },
      { kErrorBadAddress,           error::kStatusParam },
      { kErrorInvalidArgument,      error::kStatusParam },
      { kErrorTooLong,              error::kStatusParam },
      { kErrorNotSupported,         error::kStatusParam },
      { kErrorAddressNotAvailable,  error::kStatusParam },
      { kErrorTimedOut,             error::kStatusTimeout },
  };
  const uint32_t count = sizeof(kTable) / sizeof(kTable[0]);
  for (uint32_t index = 0; index < count; ++index) {
    if (kTable[index].cause == cause) {
      return kTable[index].error_status;
    }
  }
  return error::kStatusFail;
}

/**
 * @brief Make an error code.
 * @param[in] func_id  OSAL function identifier.
 * @param[in] cause    OSAL error cause.
 * @return Error code.
 *         If the error cause is kErrorNone, it returns success value (0).
 */
int32_t OSMakeErrorCode(OSFunctionId func_id, OSErrorCause cause) {
  int32_t error_code = OSGetErrorStatus(cause);
  if (error_code == error::kOk) {
    return error_code;  // Success.
  }
  error_code |= error::kBlockOsal;
  error_code |=
      static_cast<int32_t>(kFunctionIdMask & func_id) << kFunctionIdShiftBit;
  error_code |= static_cast<int32_t>(kErrorCauseMask & cause);
  return error_code;
}

/**
 * @brief Get the cause of the error.
 * @param[in] error_code  Error code returned by OSAL API.
 * @return OSAL error cause.
 *         If the error code is a positive value, it returns kErrorNone.
 */
OSErrorCause OSGetErrorCause(int32_t error_code) {
  if (error_code >= 0) {
    return kErrorNone;
  }
  if ((error_code & error::kBlockMask) != error::kBlockOsal) {
    return kErrorUnknown;
  }
  int32_t cause = error_code & kErrorCauseMask;
  if (cause > kErrorUnknown) {
    return kErrorUnknown;
  }
  return static_cast<OSErrorCause>(cause);
}

}  // namespace osal
}  // namespace senscord
