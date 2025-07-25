/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_ENVIRONMENT_H_
#define SENSCORD_ENVIRONMENT_H_

#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/noncopyable.h"
#include "senscord/status.h"

namespace senscord {

/**
 * @brief Environment variable utility class.
 */
class Environment : private util::Noncopyable {
 public:
  /**
   * @brief Get the path of senscord config file.
   * @param[out] (path) File paths.
   * @return Status object.
   */
  static Status GetSensCordFilePath(std::vector<std::string>* path);

  /**
   * @brief Set the path of senscord config file.
   * @param[in] (path) File paths.
   * @return Status object.
   */
  static Status SetSensCordFilePath(const std::vector<std::string>& path);

 private:
  /**
   * @brief The path of senscord file.
   */
  static std::vector<std::string> senscord_file_paths_;

  /**
   * @brief Private constructor
   */
  Environment() {}

  /**
   * @brief Private destructor
   */
  ~Environment() {}
};

}  // namespace senscord
#endif  // SENSCORD_ENVIRONMENT_H_
