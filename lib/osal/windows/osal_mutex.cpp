/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <Windows.h>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"
#include "windows/osal_winerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Create a mutex object.
 * @param[out] mutex  Pointer to the variable that receives the mutex object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSCreateMutex(OSMutex** mutex) {
  static const OSFunctionId kFuncId = kIdOSCreateMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  HANDLE* handle = reinterpret_cast<HANDLE*>(malloc(sizeof(HANDLE)));
  if (handle == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorOutOfMemory);
  }

  *handle = CreateMutex(NULL, FALSE, NULL);
  if (*handle == NULL) {
    free(handle);
    SENSCORD_OSAL_LOG_ERROR("failed (CreateMutex err=%u)", GetLastError());
    return OSMakeErrorCode(kFuncId, kErrorUnknown);
  }

  *mutex = reinterpret_cast<OSMutex*>(handle);

  return 0;
}

/**
 * @brief Destroy a mutex object.
 * @param[in] mutex  Mutex object to destroy.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDestroyMutex(OSMutex* mutex) {
  static const OSFunctionId kFuncId = kIdOSDestroyMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  BOOL result = CloseHandle(*(reinterpret_cast<HANDLE*>(mutex)));
  if (!result) {
    SENSCORD_OSAL_LOG_ERROR("failed (CloseHandle err=%u)", GetLastError());
    return OSMakeErrorCode(kFuncId, kErrorUnknown);
  }

  free(mutex);

  return 0;
}

/**
 * @brief Lock a mutex.
 * @param[in] mutex  Mutex object to lock.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSLockMutex(OSMutex* mutex) {
  static const OSFunctionId kFuncId = kIdOSLockMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  HANDLE* handle = reinterpret_cast<HANDLE*>(mutex);
  DWORD result = WaitForSingleObject(*handle, INFINITE);
  if (result != WAIT_OBJECT_0) {
    SENSCORD_OSAL_LOG_ERROR(
        "failed (WaitForSingleObject err=%u)", GetLastError());
    return OSMakeErrorCode(kFuncId, kErrorUnknown);
  }

  return 0;
}

/**
 * @brief Lock a mutex. (specify absolute time)
 * @param[in] mutex         Mutex object to lock.
 * @param[in] nano_seconds  Timeout absolute time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSTimedLockMutex(OSMutex* mutex, uint64_t nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSTimedLockMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  uint64_t current_time = 0;
  int32_t result = OSGetTime(&current_time);
  if (result != 0) {
    SENSCORD_OSAL_LOG_ERROR("OSGetTime failed. ret=0x%" PRIx32 "", result);
    return OSMakeErrorCode(kFuncId, kErrorInternal);
  }

  if (nano_seconds <= current_time) {
    result = OSRelativeTimedLockMutex(mutex, 0);
  } else {
    result = OSRelativeTimedLockMutex(mutex, nano_seconds - current_time);
  }

  if (result != 0) {
    OSErrorCause cause = OSGetErrorCause(result);
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Lock a mutex. (specify relative time)
 * @param[in] mutex         Mutex object to lock.
 * @param[in] nano_seconds  Timeout relative time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSRelativeTimedLockMutex(OSMutex* mutex, uint64_t nano_seconds) {
  static const OSFunctionId kFuncId = kIdOSRelativeTimedLockMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  DWORD milli_seconds = static_cast<DWORD>((nano_seconds + 999999) / 1000000);

  HANDLE* handle = reinterpret_cast<HANDLE*>(mutex);
  DWORD result = WaitForSingleObject(*handle, milli_seconds);
  if (result == WAIT_TIMEOUT) {
    return OSMakeErrorCode(kFuncId, kErrorTimedOut);
  } else if (result != WAIT_OBJECT_0) {
    SENSCORD_OSAL_LOG_ERROR(
        "failed (WaitForSingleObject err=%u)", GetLastError());
    return OSMakeErrorCode(kFuncId, kErrorUnknown);
  }

  return 0;
}

/**
 * @brief Try to lock a mutex.
 * @param[in] mutex  Mutex object to lock.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSTryLockMutex(OSMutex* mutex) {
  static const OSFunctionId kFuncId = kIdOSTryLockMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  HANDLE* handle = reinterpret_cast<HANDLE*>(mutex);
  DWORD result = WaitForSingleObject(*handle, 0);
  if (result == WAIT_TIMEOUT) {
    return OSMakeErrorCode(kFuncId, kErrorBusy);
  } else if (result != WAIT_OBJECT_0) {
    SENSCORD_OSAL_LOG_ERROR(
        "failed (WaitForSingleObject err=%u)", GetLastError());
    return OSMakeErrorCode(kFuncId, kErrorUnknown);
  }

  return 0;
}

/**
 * @brief Unlock a mutex.
 * @param[in] mutex  Mutex object to unlock.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSUnlockMutex(OSMutex* mutex) {
  static const OSFunctionId kFuncId = kIdOSUnlockMutex;
  if (mutex == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  BOOL result = ReleaseMutex(*(reinterpret_cast<HANDLE*>(mutex)));
  if (!result) {
    SENSCORD_OSAL_LOG_ERROR("failed (ReleaseMutex err=%u)", GetLastError());
    return OSMakeErrorCode(kFuncId, kErrorUnknown);
  }

  return 0;
}

}  //  namespace osal
}  //  namespace senscord
