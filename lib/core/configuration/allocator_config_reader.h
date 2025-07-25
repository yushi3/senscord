/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CONFIGURATION_ALLOCATOR_CONFIG_READER_H_
#define LIB_CORE_CONFIGURATION_ALLOCATOR_CONFIG_READER_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/develop/memory_allocator_types.h"

namespace senscord {

/**
 * @brief Allocator config reader.
 */
class AllocatorConfigReader : private util::Noncopyable {
 public:
  /**
   * @brief Read allocator config.
   * @param[in] (filename) Path of the allocator config file.
   * @param[out] (config) Allocator config.
   * @return Status object.
   */
  static Status ReadConfig(
      const std::string& file_path,
      std::vector<AllocatorConfig>* config);

 private:
  AllocatorConfigReader();
  ~AllocatorConfigReader();
};

}  // namespace senscord

#endif  // LIB_CORE_CONFIGURATION_ALLOCATOR_CONFIG_READER_H_
