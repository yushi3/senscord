/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_MEMORY_ALLOCATOR_TYPES_H_
#define SENSCORD_DEVELOP_MEMORY_ALLOCATOR_TYPES_H_

#include <string>
#include <map>

#include "senscord/config.h"

namespace senscord {

/**
 * @brief Allocator config.
 */
struct AllocatorConfig {
  std::string key;  /**< Allocator key. */
  std::string type; /**< Allocator type. */
  bool cacheable;   /**< Cacheable. */
  std::map<std::string, std::string> arguments;  /**< Arguments. */
};

}   // namespace senscord
#endif  // SENSCORD_DEVELOP_MEMORY_ALLOCATOR_TYPES_H_
