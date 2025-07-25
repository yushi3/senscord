/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string>

#include "senscord/c_api/senscord_c_api.h"
#include "c_api/c_common.h"

namespace c_api = senscord::c_api;

/**
 * @brief Gets the level of the last error that occurred.
 * @return Error level.
 */
enum senscord_error_level_t senscord_get_last_error_level(void) {
  senscord::Status* tls_status = c_api::GetLastError();
  return static_cast<senscord_error_level_t>(tls_status->level());
}

/**
 * @brief Gets the cause of the last error that occurred.
 * @return Error cause.
 */
enum senscord_error_cause_t senscord_get_last_error_cause(void) {
  senscord::Status* tls_status = c_api::GetLastError();
  return static_cast<senscord_error_cause_t>(tls_status->cause());
}

/**
 * @brief Gets the parameters of the last error that occurred.
 * @param[in] param  The type of parameter to get.
 * @param[out] buffer  Location to store the parameter string.
 * @param[in,out] length  [in] Buffer size.
 *                        [out] String length. (not including '\0')
 * @return 0 is success or minus is failed.
 */
int32_t senscord_get_last_error_string(
    enum senscord_status_param_t param,
    char* buffer,
    uint32_t* length) {
  senscord::Status* tls_status = c_api::GetLastError();
  std::string input;
  switch (param) {
    case SENSCORD_STATUS_PARAM_MESSAGE:
      input = tls_status->message();
      break;
    case SENSCORD_STATUS_PARAM_BLOCK:
      input = tls_status->block();
      break;
    case SENSCORD_STATUS_PARAM_TRACE:
      input = tls_status->trace();
      break;
    default:
      return -1;
  }
  senscord::Status status = c_api::StringToCharArray(
      input, buffer, length);
  if (!status.ok()) {
    return -1;
  }
  return 0;
}

/**
 * @brief Get information on the last error that occurred.
 * @return Error status.
 */
struct senscord_status_t senscord_get_last_error(void) {
  senscord::Status* tls_status = c_api::GetLastError();
  senscord_status_t ret = {};
  ret.level = static_cast<senscord_error_level_t>(tls_status->level());
  ret.cause = static_cast<senscord_error_cause_t>(tls_status->cause());
  ret.message = tls_status->message().c_str();
  ret.block = tls_status->block().c_str();
  ret.trace = tls_status->trace().c_str();
  return ret;
}
