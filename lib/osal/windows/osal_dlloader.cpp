/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <windows.h>

#include "senscord/osal.h"
#include "common/osal_error.h"

namespace senscord {
namespace osal {

/**
 * @brief Get the message corresponding to the error code.
 * @param[in]  error_code  Error code (GetLastError()).
 * @param[out] error_msg   Error message.
 */
void GetErrorMessage(DWORD error_code, std::string* error_msg) {
  if (error_msg != NULL) {
    LPTSTR msg_buffer = NULL;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        error_code,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        (LPTSTR)&msg_buffer,
        0,
        NULL);
    *error_msg = msg_buffer;
    LocalFree(msg_buffer);
  }
}

/**
 * @brief Load a dynamic library.
 * @param[in]  library_name  Library path name to load.
 * @param[out] dlhandle      Pointer to the variable that receives the handle
 *                           of dynamic library.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDlLoad(const char* library_name, OSDlHandle **handle) {
  return OSDlLoad(library_name, handle, NULL);
}

/**
 * @brief Load a dynamic library. (with error message)
 * @param[in]  library_name  Library path name to load.
 * @param[out] dlhandle      Pointer to the variable that receives the handle
 *                           of dynamic library.
 * @param[out] error_msg     Error message.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDlLoad(const char* library_name, OSDlHandle **handle,
    std::string* error_msg) {
  static const OSFunctionId kFuncId = kIdOSDlLoad;
  if (library_name == NULL) {
    if (error_msg != NULL) {
      *error_msg = "library name is null";
    }
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  if (handle == NULL) {
    if (error_msg != NULL) {
      *error_msg = "handle is null";
    }
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  HMODULE rtn_val = LoadLibrary(library_name);
  DWORD last_err = GetLastError();

  if (error_msg != NULL) {
    error_msg->clear();
  }
  if (rtn_val == NULL) {
    GetErrorMessage(last_err, error_msg);
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  *handle = reinterpret_cast<OSDlHandle *>(rtn_val);

  return 0;
}

/**
 * @brief Get a function pointer from a dynamic library.
 * @param[in]  handle         Handle of dynamic library.
 * @param[in]  function_name  Function name.
 * @param[out] func_ptr       Pointer to the variable that receives the
 *                            function pointer.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDlGetFuncPtr(OSDlHandle* handle,
                       const char* function_name,
                       void** func_ptr) {
  return OSDlGetFuncPtr(handle, function_name, func_ptr, NULL);
}

/**
 * @brief Get a function pointer from a dynamic library.
 *        (with error message)
 * @param[in]  handle         Handle of dynamic library.
 * @param[in]  function_name  Function name.
 * @param[out] func_ptr       Pointer to the variable that receives the
 *                            function pointer.
 * @param[out] error_msg      Error message.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDlGetFuncPtr(OSDlHandle* handle,
                       const char* function_name,
                       void** func_ptr,
                       std::string* error_msg) {
  static const OSFunctionId kFuncId = kIdOSDlGetFuncPtr;
  if (handle == NULL) {
    if (error_msg != NULL) {
      *error_msg = "handle is null";
    }
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  if (function_name == NULL) {
    if (error_msg != NULL) {
      *error_msg = "function name is null";
    }
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  if (func_ptr == NULL) {
    if (error_msg != NULL) {
      *error_msg = "func ptr is null";
    }
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  void *rtn_val = GetProcAddress(reinterpret_cast<HMODULE>(handle),
                                 function_name);
  DWORD last_err = GetLastError();

  if (error_msg != NULL) {
    error_msg->clear();
  }
  if (rtn_val == NULL) {
    GetErrorMessage(last_err, error_msg);
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  *func_ptr = rtn_val;
  return 0;
}

/**
 * @brief Unload a dynamic library.
 * @param[in] handle  Handle of dynamic library.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDlFree(OSDlHandle* handle) {
  return OSDlFree(handle, NULL);
}

/**
 * @brief Unload a dynamic library. (with error message)
 * @param[in] handle        Handle of dynamic library.
 * @param[out] error_msg    Error message.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDlFree(OSDlHandle* handle, std::string* error_msg) {
  static const OSFunctionId kFuncId = kIdOSDlFree;
  if (handle == NULL) {
    if (error_msg != NULL) {
      *error_msg = "handle is null";
    }
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  BOOL ret = FreeLibrary(reinterpret_cast<HMODULE>(handle));
  DWORD last_err = GetLastError();

  if (error_msg != NULL) {
    error_msg->clear();
  }
  if (ret == 0) {
    GetErrorMessage(last_err, error_msg);
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }

  return 0;
}

}  // namespace osal
}  // namespace senscord
