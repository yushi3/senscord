/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util/senscord_utils.h"

#include <algorithm>
#include <sstream>

#include "core/internal_types.h"
#include "senscord/osal.h"
#include "senscord/environment.h"

namespace senscord {
namespace util {

/**
 * @brief Convert numeric character to int64 type and return.
 * @param[in] (source) The numeric character of the conversion source.
 * @param[out] (result) To store the converted result
 * @return True is a success, False fail.
 */
bool StrToInt64(const std::string& source, int64_t* result) {
  if (result == NULL || source.empty()) {
    return false;
  }

  char* endptr = NULL;
  int64_t num = 0;
  if (osal::OSStrtoll(
      source.c_str(), &endptr, osal::kOSRadixAuto, &num) != 0) {
    return false;
  }

  // Characters that can not be converted to numbers are errors
  if (endptr == NULL || *endptr != '\0') {
    return false;
  }

  *result = num;

  return true;
}

/**
 * @brief Convert numeric character to int type and return.
 * @param (source) The numeric character of the conversion source.
 * @param (result) To store the converted result
 * @return True is a success, False fail.
 */
bool StrToInt(const std::string& source, int32_t* result) {
  if (result == NULL || source.empty()) {
    return false;
  }

  int64_t num = INT64_MIN;
  if (!StrToInt64(source, &num)) {
    return false;
  }

  // Numeric values that can not be converted to int type are errors
  if ((num > INT32_MAX) || (num < INT32_MIN)) {
    return false;
  }

  *result = static_cast<int32_t>(num);

  return true;
}

/**
 * @brief Convert numeric character to uint64 type and return.
 * @param[in] (source) The numeric character of the conversion source.
 * @param[out] (result) To store the converted result
 * @return True is a success, False fail.
 */
bool StrToUint64(const std::string& source, uint64_t* result) {
  if (result == NULL || source.empty()) {
    return false;
  }

  char* endptr = NULL;
  uint64_t num = 0;
  if (osal::OSStrtoull(
      source.c_str(), &endptr, osal::kOSRadixAuto, &num) != 0) {
    return false;
  }

  // Characters that can not be converted to numbers are errors
  if (endptr == NULL || *endptr != '\0') {
    return false;
  }

  *result = num;

  return true;
}

/**
 * @brief Convert numeric character to uint type and return.
 * @param (source) The numeric character of the conversion source.
 * @param (result) To store the converted result
 * @return True is a success, False fail.
 */
bool StrToUint(const std::string& source, uint32_t* result) {
  if (result == NULL || source.empty()) {
    return false;
  }

  uint64_t num = 0;
  if (!StrToUint64(source, &num)) {
    return false;
  }

  // Numeric values that can not be converted to uint type are errors
  if (num > UINT32_MAX) {
    return false;
  }

  *result = static_cast<uint32_t>(num);

  return true;
}

/**
 * @brief Get the argument value.
 * @param[in] (args) argument list.
 * @param[in] (name) argument name.
 * @param[out] (value) argument value.
 * @return Status object.
 */
Status GetArgument(
    const std::map<std::string, std::string>& args,
    const std::string& name, std::string* value) {
  if (name.empty()) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument, "empty name");
  }
  if (value == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument, "value is NULL");
  }
  std::map<std::string, std::string>::const_iterator itr = args.find(name);
  if (itr == args.end()) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseNotFound,
        "no value is existed by %s", name.c_str());
  }
  *value = itr->second;
  return Status::OK();
}

/**
 * @brief Get the argument value.
 * @param[in] (args) argument list.
 * @param[in] (name) argument name.
 * @param[out] (value) argument value.
 * @return Status object.
 */
Status GetArgumentInt64(
    const std::map<std::string, std::string>& args,
    const std::string& name, int64_t* value) {
  if (value == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument, "value is NULL");
  }
  // get value
  std::string str;
  Status status = GetArgument(args, name, &str);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  // parse
  if (!StrToInt64(str, value)) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "parse error: name=%s, %s", name.c_str(), str.c_str());
  }
  return Status::OK();
}

/**
 * @brief Get the argument value.
 * @param[in] (args) argument list.
 * @param[in] (name) argument name.
 * @param[out] (value) argument value.
 * @return Status object.
 */
Status GetArgumentUint64(
    const std::map<std::string, std::string>& args,
    const std::string& name, uint64_t* value) {
  if (value == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument, "value is NULL");
  }
  // get value
  std::string str;
  Status status = GetArgument(args, name, &str);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  // parse
  if (!StrToUint64(str, value)) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "parse error: name=%s, %s", name.c_str(), str.c_str());
  }
  return Status::OK();
}

/**
 * @brief Retrieve directory path from environment variable and
 *        perform file search
 * @param (filename) Filename to be searched.
 * @param (filepath) Search result storage location.
 * @return True is a success, False fail.
 */
bool SearchFileFromEnv(const std::string& filename, std::string* filepath) {
  if (filepath == NULL) {
    return false;
  }

  std::vector<std::string> env_paths;
  Status status = Environment::GetSensCordFilePath(&env_paths);
  if (!status.ok() || env_paths.empty()) {
    return false;
  }

  std::vector<std::string>::const_iterator itr = env_paths.begin();
  std::vector<std::string>::const_iterator end = env_paths.end();
  for (; itr != end; ++itr) {
    if (!itr->empty()) {
      std::vector<std::string> file_list;
      if (osal::OSGetRegularFileList(*itr, &file_list) != 0) {
        continue;
      }
      std::vector<std::string>::iterator result = std::find(
          file_list.begin(), file_list.end(), filename);
      if (result != file_list.end()) {
        *filepath = *itr + osal::kDirectoryDelimiter + filename;
        return true;
      }
    }
  }

  return false;
}

/**
 * @brief Convert multiple-paths to path list.
 * @param[in] (paths) multiple-paths.
 * @param[out] (path_list) list of paths.
 * @return Status object.
 */
Status ToPathList(
    const std::string& paths, std::vector<std::string>* path_list) {
  if (path_list == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "path_list is null");
  }
  std::istringstream input(paths);
  std::string path;
  while (std::getline(input, path, kEnvDelimiter)) {
    if (!path.empty()) {
      path_list->push_back(path);
    }
  }
  return Status::OK();
}

/**
 * @brief Get the path list from environment variable.
 * @param[in] (name) Name of the environment variable.
 * @param[out] (paths) Path list of the environment variable.
 * @return Status object.
 */
Status GetEnvironmentPaths(
    const std::string& name, std::vector<std::string>* paths) {
  // Get environment variable.
  std::string env;
  if (osal::OSGetEnvironment(name, &env) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "not found environment variable in \"%s\"", name.c_str());
  }

  // Convert multiple-paths to path list.
  Status status = ToPathList(env, paths);
  return SENSCORD_STATUS_TRACE(status);
}

}   // namespace util
}  //  namespace senscord
