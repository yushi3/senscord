/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_OSAL_ERROR_H_
#define SENSCORD_OSAL_ERROR_H_

#include <stdint.h>

#include "senscord/config.h"

namespace senscord {
namespace osal {

/*
 * OSAL error style.
 * - 31-28 bit : Status codes.
 * - 27-24 bit : Blocks.
 * - 23- 8 bit : OSAL function identifier.
 * -  7- 0 bit : OSAL error cause.
 */
namespace error {

// Success.
const int32_t kOk = 0;

// Status codes.
#define STATUS_CODE_SHIFT_BIT   (28)
const int32_t kStatusMask     = 0xF << STATUS_CODE_SHIFT_BIT;
const int32_t kStatusFatal    = 0x8 << STATUS_CODE_SHIFT_BIT;
const int32_t kStatusFail     = 0x9 << STATUS_CODE_SHIFT_BIT;
const int32_t kStatusWarning  = 0xA << STATUS_CODE_SHIFT_BIT;
const int32_t kStatusParam    = 0xB << STATUS_CODE_SHIFT_BIT;
const int32_t kStatusTimeout  = 0xC << STATUS_CODE_SHIFT_BIT;
#undef STATUS_CODE_SHIFT_BIT

// Blocks.
#define BLOCK_CODE_SHIFT_BIT    (24)
const int32_t kBlockMask      = 0xF << BLOCK_CODE_SHIFT_BIT;
const int32_t kBlockOsal      = 0x1 << BLOCK_CODE_SHIFT_BIT;
#undef BLOCK_CODE_SHIFT_BIT

/**
 * @brief Check whether the return code means error.
 * @param[in] Return code from funtion.
 */
inline bool IsError(int32_t return_code) {
  return (return_code < 0);
}

/**
 * @brief Check whether the return code means timeout.
 * @param[in] Return code from funtion.
 */
inline bool IsTimeout(int32_t return_code) {
  return ((return_code & kStatusMask) == kStatusTimeout);
}

}  // namespace error

/**
 * @brief OSAL error cause.
 */
enum OSErrorCause {
  kErrorNone = 0,               // 0x00
  kErrorInternal,               // 0x01
  kErrorNotPermitted,           // 0x02
  kErrorNotFound,               // 0x03
  kErrorInterrupted,            // 0x04
  kErrorIO,                     // 0x05
  kErrorInvalidObject,          // 0x06
  kErrorUnavailable,            // 0x07
  kErrorOutOfMemory,            // 0x08
  kErrorPermissionDenied,       // 0x09
  kErrorBadAddress,             // 0x0A
  kErrorBusy,                   // 0x0B
  kErrorAlreadyExists,          // 0x0C
  kErrorNotDirectory,           // 0x0D
  kErrorIsDirectory,            // 0x0E
  kErrorInvalidArgument,        // 0x0F
  kErrorResourceExhausted,      // 0x10
  kErrorNoSpaceLeft,            // 0x11
  kErrorBrokenPipe,             // 0x12
  kErrorOutOfRange,             // 0x13
  kErrorDeadLock,               // 0x14
  kErrorNoLock,                 // 0x15
  kErrorTooLong,                // 0x16
  kErrorUnimplemented,          // 0x17
  kErrorNotEmpty,               // 0x18
  kErrorNoData,                 // 0x19
  kErrorBadStatus,              // 0x1A
  kErrorNotSupported,           // 0x1B
  kErrorAddressInUse,           // 0x1C
  kErrorAddressNotAvailable,    // 0x1D
  kErrorNetworkDown,            // 0x1E
  kErrorNetworkUnreachable,     // 0x1F
  kErrorNetworkReset,           // 0x20
  kErrorConnectionAbort,        // 0x21
  kErrorConnectionReset,        // 0x22
  kErrorNoBufferSpace,          // 0x23
  kErrorIsConnected,            // 0x24
  kErrorNotConnected,           // 0x25
  kErrorShutdown,               // 0x26
  kErrorTimedOut,               // 0x27
  kErrorConnectionRefused,      // 0x28
  kErrorHostDown,               // 0x29
  kErrorHostUnreachable,        // 0x2A
  kErrorAlreadyProgress,        // 0x2B
  kErrorInProgress,             // 0x2C
  kErrorCancelled,              // 0x2D
  kErrorInvalidOperation,       // 0x2E

  // New definitions should be added above this.
  kErrorUnknown
};

/**
 * @brief Get the cause of the error.
 * @param[in] error_code  Error code returned by OSAL API.
 * @return OSAL error cause.
 *         If the error code is a positive value, it returns kErrorNone.
 */
OSErrorCause OSGetErrorCause(int32_t error_code);

}  //  namespace osal
}  //  namespace senscord

#endif  // SENSCORD_OSAL_ERROR_H_
