/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
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

namespace {

/**
 * @brief Returns true if the file is valid.
 */
bool IsValid(FILE* fp) {
  int32_t fd = fileno(fp);
  if (fd < 0) {
    return false;
  }
  return true;
}
}  // end of namespace

namespace senscord {
namespace osal {

/**
 * @brief Close a file.
 * @param[in] file  File object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFclose(OSFile* file) {
  static const OSFunctionId kFuncId = kIdOSFclose;
  if (file == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  FILE* fp = reinterpret_cast<FILE*>(file);
  if (!IsValid(fp)) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  int32_t ret = fclose(fp);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Remove a file.
 * @param[in] path_name  Path of the file to delete.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSRemove(const char* path_name) {
  static const OSFunctionId kFuncId = kIdOSRemove;
  if (path_name == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t fd = open(path_name, O_RDONLY);
  if (fd < 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  // Lock the file to match the behavior of Windows.
  // Exclusive lock, nonblocking.
  int32_t ret = flock(fd, LOCK_EX | LOCK_NB);
  if (ret != 0) {
    close(fd);
    return OSMakeErrorCode(kFuncId, kErrorPermissionDenied);
  }
  ret = unlink(path_name);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    close(fd);
    return OSMakeErrorCode(kFuncId, cause);
  }
  close(fd);
  return 0;
}

/**
 * @brief Output of the binary stream.
 * @param[in]  buffer      Pointer to the array of elements to be written.
 * @param[in]  member_size Size in bytes of each element to be written.
 * @param[in]  member_num  Number of elements, each one with size of
 *                         member_size bytes.
 * @param[in]  file        File object.
 * @param[out] written_num Total number of elements successfully written.
 *                         (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFwrite(const void* buffer, size_t member_size, size_t member_num,
                 OSFile* file, size_t* written_num) {
  static const OSFunctionId kFuncId = kIdOSFwrite;
  if (buffer == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (file == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  FILE* fp = reinterpret_cast<FILE*>(file);
  if (!IsValid(fp)) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  size_t num = fwrite(buffer, member_size, member_num, fp);
  if (written_num != NULL) {
    *written_num = num;
  }
  return 0;
}

/**
 * @brief Input of the binary stream.
 * @param[out] buffer      Pointer to the buffer that stores the read data.
 * @param[in]  member_size Size in bytes of each element to be read.
 * @param[in]  member_num  Number of elements, each one with size of
 *                         member_size bytes.
 * @param[in]  file        File object.
 * @param[out] read_num    Total number of elements successfully read.
 *                         (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFread(void* buffer, size_t member_size, size_t member_num,
                OSFile* file, size_t* read_num) {
  static const OSFunctionId kFuncId = kIdOSFread;
  if (buffer == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (file == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  FILE* fp = reinterpret_cast<FILE*>(file);
  if (!IsValid(fp)) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  size_t num = fread(buffer, member_size, member_num, fp);
  if (read_num != NULL) {
    *read_num = num;
  }
  return 0;
}

/**
 * @brief Sets the current position of the file.
 * @param[in] file    File object.
 * @param[in] offset  Binary files: Number of bytes to offset from seek_origin.
 *                    Text files: Either zero, or a value returned by OSFtell.
 * @param[in] seek_origin Position used as reference for the offset.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFseek(OSFile* file, int64_t offset, OSFileSeekOrigin seek_origin) {
  static const OSFunctionId kFuncId = kIdOSFseek;
  if (file == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t whence = 0;
  switch (seek_origin) {
    case kSeekSet: whence = SEEK_SET; break;
    case kSeekCur: whence = SEEK_CUR; break;
    case kSeekEnd: whence = SEEK_END; break;
    default:
      return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  FILE* fp = reinterpret_cast<FILE*>(file);
  if (!IsValid(fp)) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  int32_t ret = fseeko(fp, static_cast<off_t>(offset), whence);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Gets the current position of the file.
 * @param[in]  file    File object.
 * @param[out] offset  Pointer to the variable that receives the current
 *                     position.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFtell(OSFile* file, int64_t* offset) {
  static const OSFunctionId kFuncId = kIdOSFtell;
  if (file == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (offset == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  FILE* fp = reinterpret_cast<FILE*>(file);
  if (!IsValid(fp)) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  off_t off = ftello(fp);
  if (off < 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  *offset = static_cast<int64_t>(off);
  return 0;
}

/**
 * @brief Return error status of the stream.
 * @param[in] file  File object.
 * @retval >0  IO error.
 * @retval 0   No error.
 * @retval <0  Fail. (error code)
 */
int32_t OSFerror(OSFile* file) {
  static const OSFunctionId kFuncId = kIdOSFerror;
  if (file == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  FILE* fp = reinterpret_cast<FILE*>(file);
  if (!IsValid(fp)) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  int32_t ret = ferror(fp);
  return ret;
}

/**
 * @brief Return EOF status of the stream.
 * @param[in] file  File object.
 * @retval >0  EOF.
 * @retval 0   Not EOF.
 * @retval <0  Fail. (error code)
 */
int32_t OSFeof(OSFile* file) {
  static const OSFunctionId kFuncId = kIdOSFeof;
  if (file == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  FILE* fp = reinterpret_cast<FILE*>(file);
  if (!IsValid(fp)) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  int32_t ret = feof(fp);
  return ret;
}

/**
 * @brief Reset the status of the stream.
 * @param[in] file  File object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFclearError(OSFile* file) {
  static const OSFunctionId kFuncId = kIdOSFclearError;
  if (file == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  FILE* fp = reinterpret_cast<FILE*>(file);
  if (!IsValid(fp)) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  clearerr(fp);
  return 0;
}

/**
 * @brief file flush of the stream.
 * @param[in] file  File object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFflush(OSFile* file) {
  static const OSFunctionId kFuncId = kIdOSFflush;
  if (file == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  FILE* fp = reinterpret_cast<FILE*>(file);
  if (!IsValid(fp)) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }
  int32_t ret = fflush(fp);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief file flush of the stream.
 * @param[in]  file File object.
 * @param[out] size File size.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetBinaryFileSize(OSFile* file, size_t* size) {
  static const OSFunctionId kFuncId = kIdOSGetFileSize;
  if (file == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (size == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  FILE* fp = reinterpret_cast<FILE*>(file);
  if (!IsValid(fp)) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidObject);
  }

  int32_t fd = fileno(fp);

  struct stat stbuf = {};
  if (fstat(fd, &stbuf) == -1) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  *size = stbuf.st_size;

  return 0;
}

}  // namespace osal
}  // namespace senscord
