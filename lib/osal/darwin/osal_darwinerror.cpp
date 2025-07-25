/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <errno.h>

#include "osal_darwinerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Get error cause from errno.
 * @param[in] errno_val  Error number. (defined in <errno.h>)
 * @return OSAL error cause.
 *         If the errno_val is an undefined value, it returns kErrorUnknown.
 */
OSErrorCause GetErrorCauseFromErrno(int32_t errno_val) {
  struct ErrnoMap {
    int32_t errno_val;
    OSErrorCause cause;
  };
  const ErrnoMap kTable[] = {
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
      { ENODATA,         kErrorNoData },
      { EBADF,           kErrorBadStatus },
      { ENOTSOCK,        kErrorInvalidObject },
      { EDESTADDRREQ,    kErrorNotConnected },
      { EMSGSIZE,        kErrorTooLong },
      { EPROTONOSUPPORT, kErrorNotSupported },
      { ESOCKTNOSUPPORT, kErrorNotSupported },
      { EOPNOTSUPP,      kErrorNotSupported },
      { EPFNOSUPPORT,    kErrorNotSupported },
      { EAFNOSUPPORT,    kErrorNotSupported },
      { EADDRINUSE,      kErrorAddressInUse },
      { EADDRNOTAVAIL,   kErrorAddressNotAvailable },
      { ENETDOWN,        kErrorNetworkDown },
      { ENETUNREACH,     kErrorNetworkUnreachable },
      { ENETRESET,       kErrorNetworkReset },
      { ECONNABORTED,    kErrorConnectionAbort },
      { ECONNRESET,      kErrorConnectionReset },
      { ENOBUFS,         kErrorNoBufferSpace },
      { EISCONN,         kErrorIsConnected },
      { ENOTCONN,        kErrorNotConnected },
      { ESHUTDOWN,       kErrorShutdown },
      { ETIMEDOUT,       kErrorTimedOut },
      { ECONNREFUSED,    kErrorConnectionRefused },
      { EHOSTDOWN,       kErrorHostDown },
      { EHOSTUNREACH,    kErrorHostUnreachable },
      { EALREADY,        kErrorAlreadyProgress },
      { EINPROGRESS,     kErrorInProgress },
      { ECANCELED,       kErrorCancelled },
  };
  const uint32_t count = sizeof(kTable) / sizeof(kTable[0]);
  for (uint32_t index = 0; index < count; ++index) {
    if (kTable[index].errno_val == errno_val) {
      return kTable[index].cause;
    }
  }
  return kErrorUnknown;
}

}  // namespace osal
}  // namespace senscord
