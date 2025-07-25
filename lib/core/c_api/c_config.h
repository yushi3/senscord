/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_C_API_C_CONFIG_H_
#define LIB_CORE_C_API_C_CONFIG_H_

#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/configuration.h"

namespace senscord {
namespace c_api {

/**
 * @brief Converter configuration.
 */
struct ConverterConfig {
  std::string library_name;
  bool enable_property;
  bool enable_rawdata;
};

/**
 * @brief Data of config handle.
 */
struct ConfigHandle {
  /** Configuration object pointer. */
  Configuration* config;
  /** converter configuration. */
  std::vector<ConverterConfig> converters;
};

}  // namespace c_api
}  // namespace senscord

#endif  // LIB_CORE_C_API_C_CONFIG_H_
