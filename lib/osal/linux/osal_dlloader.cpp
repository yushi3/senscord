/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <dlfcn.h>

#include "senscord/osal.h"
#include "common/osal_error.h"

namespace senscord {
namespace osal {

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
 * @brief Load a dynamic library.
 * @param[in]  library_name  Library path name to load. (with error message)
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

  dlerror();
  void* rtn_val = dlopen(library_name, RTLD_NOW | RTLD_LOCAL);
  char* err_str = dlerror();

  if (rtn_val == NULL) {
    if (error_msg != NULL && err_str != NULL) {
      *error_msg = err_str;
    }
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  *handle = reinterpret_cast<OSDlHandle *>(rtn_val);
  if (error_msg != NULL) {
    error_msg->clear();
  }
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

  dlerror();
  void* rtn_val = dlsym(handle, function_name);
  char* err_str = dlerror();

  if (err_str != NULL) {
    if (error_msg != NULL) {
      *error_msg = err_str;
    }
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  *func_ptr = rtn_val;
  if (error_msg != NULL) {
    error_msg->clear();
  }
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
 * @param[in]  handle       Handle of dynamic library.
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

  dlerror();
  int ret = dlclose(handle);
  char* err_str = dlerror();

  if (ret != 0) {
    if (error_msg != NULL && err_str != NULL) {
      *error_msg = err_str;
    }
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }

  if (error_msg != NULL) {
    error_msg->clear();
  }
  return 0;
}

}  // namespace osal
}  // namespace senscord
