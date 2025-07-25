/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "windows/osal_winerror.h"

#include <inttypes.h>
#include <stdio.h>
#include <errno.h>
#include <Winsock2.h>

#include "senscord/osal.h"
#include "common/osal_logger.h"

namespace senscord {
namespace osal {

/**
 * @brief Get error cause from errno.
 * @param[in] errno_val  Error number. (defined in <errno.h>)
 * @return OSAL error cause.
 *         If the errno_val is an undefined value, it returns kErrorUnknown.
 */
OSErrorCause GetErrorCauseFromErrno(int32_t errno_val) {
  struct ErrorMap {
    int32_t errno_val;
    OSErrorCause cause;
  };
  const ErrorMap kTable[] = {
      { 0,               kErrorNone },
      { EPERM,           kErrorNotPermitted },
      { ENOENT,          kErrorNotFound },
      { ESRCH,           kErrorNotFound },
      { EINTR,           kErrorInterrupted },
      { EIO,             kErrorIO },
      { EBADF,           kErrorInvalidObject },
      { EAGAIN,          kErrorUnavailable },
      { ENOMEM,          kErrorOutOfMemory },
      { EACCES,          kErrorPermissionDenied },
      { EFAULT,          kErrorBadAddress },
      { EBUSY,           kErrorBusy },
      { EEXIST,          kErrorAlreadyExists },
      { ENODEV,          kErrorNotFound },
      { ENOTDIR,         kErrorNotDirectory },
      { EISDIR,          kErrorIsDirectory },
      { EINVAL,          kErrorInvalidArgument },
      { ENFILE,          kErrorResourceExhausted },
      { EMFILE,          kErrorResourceExhausted },
      { ENOSPC,          kErrorNoSpaceLeft },
      { EPIPE,           kErrorBrokenPipe },
      { ERANGE,          kErrorOutOfRange },
      { EDEADLK,         kErrorDeadLock },
      { ENOLCK,          kErrorNoLock },
      { ENAMETOOLONG,    kErrorTooLong },
      { ENOSYS,          kErrorUnimplemented },
      { ENOTEMPTY,       kErrorNotEmpty },
  };
  const uint32_t count = sizeof(kTable) / sizeof(kTable[0]);
  for (uint32_t index = 0; index < count; ++index) {
    if (kTable[index].errno_val == errno_val) {
      return kTable[index].cause;
    }
  }
  SENSCORD_OSAL_LOG_WARNING(
      "Return value is kErrorUnknown. errno=%" PRId32, errno_val);
  return kErrorUnknown;
}

/**
* @brief Get error cause from Winsock error.
* @param[in] wsa_err  Error code. (defined in <Winsock2.h>)
* @return OSAL error cause.
*         If the wsa_err is an undefined value, it returns kErrorUnknown.
*/
OSErrorCause GetErrorCauseFromWinsock(int32_t wsa_err) {
  struct ErrorMap {
    int32_t wsa_err;
    OSErrorCause cause;
  };
  const ErrorMap kTable[] = {
      { 0,                     kErrorNone },
      { WSA_INVALID_HANDLE,    kErrorInvalidObject },
      { WSA_NOT_ENOUGH_MEMORY, kErrorOutOfMemory },
      { WSA_INVALID_PARAMETER, kErrorInvalidArgument },
      { WSAEINTR,              kErrorInterrupted },
      { WSAEBADF,              kErrorInvalidObject },
      { WSAEACCES,             kErrorPermissionDenied },
      { WSAEFAULT,             kErrorBadAddress },
      { WSAEINVAL,             kErrorInvalidArgument },
      { WSAEMFILE,             kErrorResourceExhausted },
      { WSAEWOULDBLOCK,        kErrorUnavailable },
      { WSAEINPROGRESS,        kErrorInProgress },
      { WSAEALREADY,           kErrorAlreadyProgress },
      { WSAENOTSOCK,           kErrorInvalidObject },
      { WSAEDESTADDRREQ,       kErrorNotConnected },
      { WSAEMSGSIZE,           kErrorTooLong },
      { WSAEPROTONOSUPPORT,    kErrorNotSupported },
      { WSAESOCKTNOSUPPORT,    kErrorNotSupported },
      { WSAEOPNOTSUPP,         kErrorNotSupported },
      { WSAEPFNOSUPPORT,       kErrorNotSupported },
      { WSAEAFNOSUPPORT,       kErrorNotSupported },
      { WSAEADDRINUSE,         kErrorAddressInUse },
      { WSAEADDRNOTAVAIL,      kErrorAddressNotAvailable },
      { WSAENETDOWN,           kErrorNetworkDown },
      { WSAENETUNREACH,        kErrorNetworkUnreachable },
      { WSAENETRESET,          kErrorNetworkReset },
      { WSAECONNABORTED,       kErrorConnectionAbort },
      { WSAECONNRESET,         kErrorConnectionReset },
      { WSAENOBUFS,            kErrorNoBufferSpace },
      { WSAEISCONN,            kErrorIsConnected },
      { WSAENOTCONN,           kErrorNotConnected },
      { WSAESHUTDOWN,          kErrorShutdown },
      { WSAETIMEDOUT,          kErrorTimedOut },
      { WSAECONNREFUSED,       kErrorConnectionRefused },
      { WSAENAMETOOLONG,       kErrorTooLong },
      { WSAEHOSTDOWN,          kErrorHostDown },
      { WSAEHOSTUNREACH,       kErrorHostUnreachable },
      { WSAENOTEMPTY,          kErrorNotEmpty },
      { WSAVERNOTSUPPORTED,    kErrorNotSupported },
      { WSANOTINITIALISED,     kErrorInvalidOperation },
      { WSAECANCELLED,         kErrorCancelled },
      { WSASERVICE_NOT_FOUND,  kErrorNotFound },
      { WSATYPE_NOT_FOUND,     kErrorNotFound },
      { WSA_E_CANCELLED,       kErrorCancelled },
      { WSAHOST_NOT_FOUND,     kErrorHostUnreachable },
      { WSATRY_AGAIN,          kErrorHostUnreachable },
      { WSANO_DATA,            kErrorNoData },
  };
  const uint32_t count = sizeof(kTable) / sizeof(kTable[0]);
  for (uint32_t index = 0; index < count; ++index) {
    if (kTable[index].wsa_err == wsa_err) {
      return kTable[index].cause;
    }
  }
  SENSCORD_OSAL_LOG_WARNING(
      "Return value is kErrorUnknown. wsa_err=%" PRId32, wsa_err);
  return kErrorUnknown;
}

}  // namespace osal
}  // namespace senscord
