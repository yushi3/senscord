/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "c_api/c_common.h"

#include <string>

#include "senscord/c_api/senscord_c_api.h"
#include "senscord/osal.h"

#ifdef _WIN32
#define THREAD_LOCAL    __declspec(thread)
#else
  #if defined(ANDROID) || defined(__APPLE__)
    #define THREAD_LOCAL    thread_local
  #else
    #define THREAD_LOCAL    __thread
  #endif
#endif

namespace senscord {
namespace c_api {

/**
 * @brief Get the last error per thread.
 */
Status* GetLastError() {
  static THREAD_LOCAL Status tls_status;
  return &tls_status;
}

/**
 * @brief Save the last error that occurred.
 */
void SetLastError(const Status& status) {
  Status* tls_status = GetLastError();
  *tls_status = status;
}

/**
 * @brief Copy string to char array.
 *
 * If "buffer == NULL" and "length != NULL",
 * the required buffer size is stored in "length".
 *
 * @param[in] (input) Input string.
 * @param[out] (buffer) Location to store the string.
 * @param[in,out] (length) [in] Buffer size.
 *                        [out] String length. (not including '\0')
 * @return Status object.
 */
Status StringToCharArray(
    const std::string& input, char* buffer, uint32_t* length) {
  SENSCORD_STATUS_ARGUMENT_CHECK(length == NULL);
  uint32_t buffer_size = *length;
  uint32_t input_size = static_cast<uint32_t>(input.size()) + 1;
  *length = input_size;
  SENSCORD_STATUS_ARGUMENT_CHECK(buffer == NULL);
  if (buffer_size < input_size) {
    if (buffer_size > 0) {
      osal::OSMemcpy(buffer, buffer_size, input.c_str(), buffer_size - 1);
      buffer[buffer_size - 1] = '\0';
    }
    return SENSCORD_STATUS_FAIL(
        senscord::kStatusBlockCore, senscord::Status::kCauseOutOfRange,
        "Insufficient buffer length.");
  }
  osal::OSMemcpy(buffer, buffer_size, input.c_str(), input_size);
  *length = input_size - 1;  // Subtract the size of '\0'.
  return Status::OK();
}

}  // namespace c_api
}  // namespace senscord
