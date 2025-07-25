/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CORE_INTERNAL_TYPES_H_
#define LIB_CORE_CORE_INTERNAL_TYPES_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <utility>  // pair

#include "senscord/senscord_types.h"
#include "senscord/develop/common_types.h"
#include "configuration/core_config.h"

namespace senscord {

/**
 * @brief Callback strategy types when multiple frames arrive simultaneously.
 */
enum CallbackStrategy {
  kCallbackStrategyOrder,     /**< Called in keeping sequential order. */
  kCallbackStrategySimple,    /**< Called as soon as frame arrives. */
};

/**
 * @brief A structure that stores the result of parsing the
 *        component config file.
 */
struct ComponentConfig {
  std::string name;
  uint32_t major_version;
  uint32_t minor_version;
  uint32_t patch_version;
  std::string description;
  std::vector<Version> linkage_versions;
};

/**
 * @brief Event occured information.
 */
struct EventInfo {
  std::string type;
  EventArgument argument;
};

// Port type: undefined
const char kPortTypeUndefined[] = "undefined";

// Senscord config file name
const char kSenscordConfigFile[] = "senscord.xml";

// Allocator config file name
const char kAllocatorConfigFile[] = "senscord_allocators.xml";

// Recorder config file name
const char kRecorderConfigFile[] = "senscord_recorders.xml";

// Connection config file name
const char kConnectionConfigFile[] = "senscord_connections.xml";

// TODO(kamata): will be deleted in the future.
// file path
const char kSensCordFilePathEnvStr[] = "SENSCORD_FILE_PATH";

// SensCord identification
const char kSensCordIdentification[] = "SENSCORD_IDENTIFICATION";

// SensCord identification delimiter
const char kSensCordIdentificationDelimiter[] = "_";

#ifdef _WIN32
const char kEnvDelimiter = ';';
#else
const char kEnvDelimiter = ':';
#endif

// Component name (for Publisher)
const char kComponentNamePublisher[] = "publisher";

}   // namespace senscord
#endif  // LIB_CORE_CORE_INTERNAL_TYPES_H_
