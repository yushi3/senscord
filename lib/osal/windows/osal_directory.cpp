/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <Windows.h>
#include <direct.h>

#include <string>
#include <vector>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"
#include "windows/osal_winerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Make the directory
 * @param[in] (directory_path) Directory path.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSMakeDirectory(const char* directory_path) {
  static const OSFunctionId kFuncId = kIdOSMakeDirectory;
  if (directory_path == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int ret = _mkdir(directory_path);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Remove the directory
 * @param[in] (directory_path) Directory path.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSRemoveDirectory(const char* directory_path) {
  static const OSFunctionId kFuncId = kIdOSRemoveDirectory;
  if (directory_path == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int ret = _rmdir(directory_path);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Get a list of regular files in the specified directory.
 * @param[in]  directory_path Path of the directory to scan.
 * @param[out] file_list      Pointer to the variable length array that receive
 *                            a list of file paths.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetRegularFileList(const std::string& directory_path,
                             std::vector<std::string>* file_list) {
  static const OSFunctionId kFuncId = kIdOSGetRegularFileList;
  if (file_list == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  std::string target_path = directory_path;
  size_t path_size = target_path.size() - 1;
  if ((target_path[path_size] != '\\') &&
      (target_path[path_size] != '/')) {
    target_path += "\\";
  }
  target_path += "*";

  HANDLE handle;
  WIN32_FIND_DATA fd;

  handle = FindFirstFile(target_path.c_str(), &fd);
  if (handle == INVALID_HANDLE_VALUE) {
    SENSCORD_OSAL_LOG_ERROR("failed (FindFirstFile err=%u)", GetLastError());
    return OSMakeErrorCode(kFuncId, kErrorNotFound);
  }

  file_list->clear();
  do {
    if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
      file_list->push_back(fd.cFileName);
    }
  } while (FindNextFile(handle, &fd));

  BOOL ret = FindClose(handle);
  if (ret == 0) {
    SENSCORD_OSAL_LOG_ERROR("failed (FindClose err=%u)", GetLastError());
    return OSMakeErrorCode(kFuncId, kErrorUnknown);
  }

  return 0;
}

/**
 * @brief Get the value of the specified environment variable.
 * @param[in]  name        Name of the environment variable.
 * @param[out] environment Pointer to the variable that receives the value of
 *                         the environment variable.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetEnvironment(const std::string& name, std::string* environment) {
  static const OSFunctionId kFuncId = kIdOSGetEnvironment;
  if (environment == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  char* temporary_environment;
  size_t environment_size;

  int32_t result = 0;
  errno_t get_result = getenv_s(&environment_size, NULL, 0, name.c_str());
  if (get_result != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(get_result);
    return OSMakeErrorCode(kFuncId, cause);
  } else if (environment_size == 0) {
    *environment = "";
  } else {
    temporary_environment = reinterpret_cast<char*>(malloc(environment_size *
        sizeof(char)));
    if (temporary_environment == NULL) {
      result = OSMakeErrorCode(kFuncId, kErrorOutOfMemory);
    } else {
      get_result = getenv_s(&environment_size, temporary_environment,
          environment_size, name.c_str());
      if (get_result != 0) {
        OSErrorCause cause = GetErrorCauseFromErrno(get_result);
        result = OSMakeErrorCode(kFuncId, cause);
      } else {
        *environment = temporary_environment;
      }

      free(temporary_environment);
    }
  }

  return result;
}

/**
 * @brief Get the file name of the dynamic library.
 * @param[in] base  Base file name.
 * @param[out] name  Dynamic library file name.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetDynamicLibraryFileName(
    const std::string& base, std::string* name) {
  static const OSFunctionId kFuncId = kIdOSGetDynamicLibraryFileName;
  if (name == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  *name = base + ".dll";
  return 0;
}

}  //  namespace osal
}  //  namespace senscord
