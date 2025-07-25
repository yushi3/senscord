/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <string>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "windows/osal_winerror.h"

namespace {

namespace osal = senscord::osal;

/**
 * @brief Returns true if the file is valid.
 */
bool IsValid(FILE* fp) {
  int32_t fd = _fileno(fp);
  if (fd < 0) {
    return false;
  }
  return true;
}

/**
 * @brief Verify the file access mode.
 * @param[in]  mode   String containing the file access mode.
 * @param[out] oflag  File access mode. (defined in <fcntl.h>)
 * @return OSAL error cause. On success, it returns kErrorNone.
 */
osal::OSErrorCause VerifyAccessMode(const char* mode, int32_t* oflag) {
  std::string temp = mode;
  std::string basic_mode = temp.substr(0, temp.find(','));
  if (basic_mode.empty()) {
    return osal::kErrorInvalidArgument;
  }
  int32_t temp_oflag = 0;
  bool update = (basic_mode.find('+') != std::string::npos);
  switch (basic_mode[0]) {
    case 'r':  // Read.
      if (update) {
        temp_oflag |= _O_RDWR;
      } else {
        temp_oflag |= _O_RDONLY;
      }
      break;
    case 'w':  // Write.
      temp_oflag |= _O_CREAT | _O_TRUNC;
      if (update) {
        temp_oflag |= _O_RDWR;
      } else {
        temp_oflag |= _O_WRONLY;
      }
      break;
    case 'a':  // Append.
      temp_oflag |= _O_CREAT | _O_APPEND;
      if (update) {
        temp_oflag |= _O_RDWR;
      } else {
        temp_oflag |= _O_WRONLY;
      }
      break;
    default:
      return osal::kErrorInvalidArgument;
  }
  if (basic_mode.find('b') != std::string::npos) {
    temp_oflag |= _O_BINARY;
  } else {
    temp_oflag |= _O_TEXT;
  }
  if (oflag != NULL) {
    *oflag = temp_oflag;
  }
  return osal::kErrorNone;
}
}  // end of namespace

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

  OSErrorCause cause = VerifyAccessMode(mode, NULL);
  if (cause != kErrorNone) {
    return OSMakeErrorCode(kFuncId, cause);
  }

  FILE* fp = NULL;
  errno_t ret = fopen_s(&fp, file_path, mode);
  if (ret != 0) {
    cause = GetErrorCauseFromErrno(ret);
    return OSMakeErrorCode(kFuncId, cause);
  }

  *file = reinterpret_cast<OSFile*>(fp);

  return 0;
}

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
  int32_t ret = remove(path_name);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
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
  int32_t ret = _fseeki64(fp, static_cast<__int64>(offset), whence);
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
  int64_t off = _ftelli64(fp);
  if (off < 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  *offset = off;
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
  errno_t ret = clearerr_s(fp);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(ret);
    return OSMakeErrorCode(kFuncId, cause);
  }
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

  int32_t fd = _fileno(fp);

#ifndef _WIN64
  struct stat stbuf = {};
  if (fstat(fd, &stbuf) == -1) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
#else
  struct _stat64 stbuf = {};
  if (_fstat64(fd, &stbuf) == -1) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
#endif
  *size = stbuf.st_size;

  return 0;
}

}  // namespace osal
}  // namespace senscord
