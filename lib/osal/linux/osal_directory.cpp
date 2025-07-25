/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <vector>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "linux/osal_linuxerror.h"

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

  // permission: 777
  int ret = mkdir(directory_path, S_IRWXU | S_IRWXG | S_IRWXO);
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

  int ret = rmdir(directory_path);
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

  DIR *directory = opendir(directory_path.c_str());
  if (directory == NULL) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  int32_t result = 0;
  file_list->clear();
  struct dirent *dent;
  while ((dent = readdir(directory)) != NULL) {
    std::string file_path = directory_path.c_str();
    file_path += "/";
    file_path += dent->d_name;

    struct stat st = {};
    result = lstat(file_path.c_str(), &st);
    if (result != 0) {
      OSErrorCause cause = GetErrorCauseFromErrno(errno);
      result = OSMakeErrorCode(kFuncId, cause);
      break;
    }
    if (S_ISREG(st.st_mode)) {
      file_list->push_back(dent->d_name);
    }
  }

  closedir(directory);

  return result;
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

  char* temporary_environment = getenv(name.c_str());
  if (temporary_environment == NULL ||
      (strlen(temporary_environment) + 1) > environment->max_size()) {
    *environment = "";
  } else {
    *environment = temporary_environment;
  }

  return 0;
}

}  //  namespace osal
}  //  namespace senscord
