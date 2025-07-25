/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_C_API_C_COMMON_H_
#define LIB_CORE_C_API_C_COMMON_H_

#include <string>

#include "senscord/status.h"
#include "senscord/c_api/senscord_c_types.h"

/**
 * @brief Macro for argument checking.
 * If the judgment expression is true, call return with "Invalid Argument".
 * @param (expr) judgment expression.
 */
#define SENSCORD_C_API_ARGUMENT_CHECK(expr) \
  do { \
    if (expr) { \
      senscord::c_api::SetLastError(SENSCORD_STATUS_FAIL( \
          senscord::kStatusBlockCore, \
          senscord::Status::kCauseInvalidArgument, \
          #expr)); \
      return -1; \
    } \
  } while (false)

namespace senscord {
namespace c_api {

/**
 * @brief Get the last error per thread.
 */
Status* GetLastError();

/**
 * @brief Save the last error that occurred.
 */
void SetLastError(const Status& status);

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
    const std::string& input, char* buffer, uint32_t* length);

/**
 * @brief Convert from pointer to handle.
 *
 * pointer -> uintprt_t -> senscord_handle_t(uint64_t)
 */
inline senscord_handle_t ToHandle(const void* ptr) {
  return static_cast<senscord_handle_t>(reinterpret_cast<uintptr_t>(ptr));
}

/**
 * @brief Convert from handle to pointer.
 *
 * senscord_handle_t(uint64_t) -> uintprt_t -> pointer
 */
template<typename T>
T ToPointer(senscord_handle_t handle) {
  return reinterpret_cast<T>(static_cast<uintptr_t>(handle));
}

}  // namespace c_api
}  // namespace senscord

#endif  // LIB_CORE_C_API_C_COMMON_H_
