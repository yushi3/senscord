/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string>
#include <vector>
#include <sstream>

#include "senscord/c_api/senscord_c_api.h"
#include "senscord/environment.h"
#include "util/senscord_utils.h"
#include "core/internal_types.h"
#include "c_api/c_common.h"

namespace c_api = senscord::c_api;

/**
 * @brief Set the file search paths.
 *
 * Use instead of SENSCORD_FILE_PATH.
 *
 * @param[in] paths  The same format as SENSCORD_FILE_PATH.
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_set_file_search_path(
    const char* paths) {
  SENSCORD_C_API_ARGUMENT_CHECK(paths == NULL);

  // Convert multiple-paths to path list.
  std::vector<std::string> path_list;
  senscord::Status status = senscord::util::ToPathList(paths, &path_list);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }

  status = senscord::Environment::SetSensCordFilePath(path_list);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }

  return 0;
}

/**
 * @brief Get the file search paths.
 *
 * If "buffer == NULL" and "length != NULL",
 * the required buffer size is stored in "length".
 *
 * @param[out] buffer  Location to store the path string.
 * @param[in,out] length  [in] Buffer size.
 *                        [out] String length. (not including '\0')
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_get_file_search_path(
    char* buffer, uint32_t* length) {
  SENSCORD_C_API_ARGUMENT_CHECK(length == NULL);

  std::vector<std::string> path_list;
  senscord::Status status = senscord::Environment::GetSensCordFilePath(
      &path_list);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }

  std::ostringstream ostream;
  for (std::vector<std::string>::const_iterator itr = path_list.begin(),
      end = path_list.end(); itr != end; ++itr) {
    std::ostringstream::pos_type pos = ostream.tellp();
    if (pos > 0) {
      ostream << senscord::kEnvDelimiter;
    }
    ostream << *itr;
  }

  status = c_api::StringToCharArray(ostream.str(), buffer, length);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}
