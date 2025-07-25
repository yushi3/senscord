/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "linux/osal_linuxerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Open a file.
 * @param[in]  file_path Path of the file to be opened.
 * @param[in]  mode      String containing the file access mode.
 * @param[out] file      Pointer to the variable that receives the file object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFopen(const char* file_path, const char* mode, OSFile** file) {
  static const OSFunctionId kFuncId = kIdOSFopen;
  if (file_path == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (mode == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (file == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int32_t ret = 0;
  FILE* fp = fopen(file_path, mode);
  if (fp == NULL) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }

  // Lock the file to match the behavior of Windows.
  // Exclusive lock, nonblocking.
  std::string tmp(mode);
  if ((tmp[0] == 'w') || (tmp[0] == 'a') ||
      (tmp.find('+') != std::string::npos)) {
    ret = flock(fileno(fp), LOCK_EX | LOCK_NB);
  } else if (tmp[0] == 'r') {
    ret = flock(fileno(fp), LOCK_SH | LOCK_NB);
  } else {
    fclose(fp);
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (ret < 0) {
    fclose(fp);
    return OSMakeErrorCode(kFuncId, kErrorPermissionDenied);
  }

  // seek position is set to the beginning to
  // match the operation with other OSs.
  if (tmp[0] == 'a' && (tmp.find('+') != std::string::npos)) {
    ret = fseek(fp, 0, SEEK_SET);
    if (ret < 0) {
      OSErrorCause cause = GetErrorCauseFromErrno(errno);
      fclose(fp);
      return OSMakeErrorCode(kFuncId, cause);
    }
  }
  *file = reinterpret_cast<OSFile*>(fp);

  return 0;
}

}  // namespace osal
}  // namespace senscord
