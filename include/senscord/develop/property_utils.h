/*
 * SPDX-FileCopyrightText: 2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_PROPERTY_UTILS_H_
#define SENSCORD_DEVELOP_PROPERTY_UTILS_H_

#include <stdint.h>
#include <string>
#include "senscord/status.h"

namespace senscord {

class PropertyUtils {
 public:
  /**
   * @brief Set Channel ID to property key.
   * @param[in] (key) Key of property.
   * @param[in] (channel_id) Channel ID to be set to key.
   * @return Property key with Channel ID assigned.
   */
  static std::string SetChannelId(const std::string& key, uint32_t channel_id);

  /**
   * @brief Get Channel ID from property key.
   * @param[in] (key) Key of property.
   * @param[out] (channel_id) Channel ID retrieved from key.
   * @return Status object.
   */
  static Status GetChannelId(const std::string& key, uint32_t* channel_id);

  /**
   * @brief Get property key.
   * @param[in] (key) Key of property.
   * @return A value that excludes the Channel ID from the property key.
   */
  static std::string GetKey(const std::string& key);
};
}   // namespace senscord
#endif  // SENSCORD_DEVELOP_PROPERTY_UTILS_H_
