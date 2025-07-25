/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_UTIL_SENSCORD_UTILS_H_
#define LIB_CORE_UTIL_SENSCORD_UTILS_H_

#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include "senscord/status.h"

namespace senscord {
namespace util {

/**
 * @brief Convert numeric character to int64 type and return.
 * @param[in] (source) The numeric character of the conversion source.
 * @param[out] (result) To store the converted result
 * @return True is a success, False fail.
 */
extern bool StrToInt64(const std::string& source, int64_t* result);

/**
 * @brief Convert numeric character to int type and return.
 * @param (source) The numeric character of the conversion source.
 * @param (result) To store the converted result
 * @return True is a success, False fail.
 */
extern bool StrToInt(const std::string& source, int32_t* result);

/**
 * @brief Convert numeric character to uint64 type and return.
 * @param[in] (source) The numeric character of the conversion source.
 * @param[out] (result) To store the converted result
 * @return True is a success, False fail.
 */
extern bool StrToUint64(const std::string& source, uint64_t* result);

/**
 * @brief Convert numeric character to uint type and return.
 * @param (source) The numeric character of the conversion source.
 * @param (result) To store the converted result
 * @return True is a success, False fail.
 */
extern bool StrToUint(const std::string& source, uint32_t* result);

/**
 * @brief Get the argument value.
 * @param[in] (args) argument list.
 * @param[in] (name) argument name.
 * @param[out] (value) argument value.
 * @return Status object.
 */
Status GetArgument(
    const std::map<std::string, std::string>& args,
    const std::string& name, std::string* value);
Status GetArgumentInt64(
    const std::map<std::string, std::string>& args,
    const std::string& name, int64_t* value);
Status GetArgumentUint64(
    const std::map<std::string, std::string>& args,
    const std::string& name, uint64_t* value);

/**
 * @brief Retrieve directory path from environment variable and
 *        perform file search
 * @param (filename) Filename to be searched.
 * @param (filepath) Search result storage location.
 * @return True is a success, False fail.
 */
extern bool SearchFileFromEnv(const std::string& filename,
                                 std::string* filepath);

/**
 * @brief Convert multiple-paths to path list.
 * @param[in] (paths) multiple-paths.
 * @param[out] (path_list) list of paths.
 * @return Status object.
 */
Status ToPathList(
    const std::string& paths, std::vector<std::string>* path_list);

/**
 * @brief Get the path list from environment variable.
 * @param[in] (name) Name of the environment variable.
 * @param[out] (paths) Path list of the environment variable.
 * @return Status object.
 */
Status GetEnvironmentPaths(
    const std::string& name, std::vector<std::string>* paths);

}   // namespace util
}  //  namespace senscord
#endif  // LIB_CORE_UTIL_SENSCORD_UTILS_H_
