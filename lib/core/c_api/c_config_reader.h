/*
 * SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_C_API_C_CONFIG_READER_H_
#define LIB_CORE_C_API_C_CONFIG_READER_H_

#include <string>
#include <vector>
#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "c_api/c_config.h"

namespace senscord {
namespace c_api {

/**
 * @brief Config reader for C API.
 */
class ConfigReader : private senscord::util::Noncopyable {
 public:
  /**
   * @brief Read the converter information.
   * @param[in]  filename    Config file path.
   * @param[out] converters  Converter information read from the config file.
   * @return Status object.
   */
  static Status ReadConverterInfo(
      const std::string& filename, std::vector<ConverterConfig>* converters);

 private:
  ConfigReader();
  ~ConfigReader();
};

}  // namespace c_api
}  // namespace senscord

#endif  // LIB_CORE_C_API_C_CONFIG_READER_H_
