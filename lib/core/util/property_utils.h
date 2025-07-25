/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_UTIL_PROPERTY_UTILS_H_
#define LIB_CORE_UTIL_PROPERTY_UTILS_H_

#include <string>
#include <map>

#include "senscord/status.h"

namespace senscord {
namespace util {

/**
 * @brief Property key parser.
 */
class PropertyKey {
 public:
  /**
   * @brief constructor.
   * @param[in] (key) Key of property.
   */
  explicit PropertyKey(const std::string& key)
    : key_end_(), append_begin_(), append_end_() {
    ParseKey(key);
    MakeFullKey();
  }

  /**
   * @brief operator <.
   * @param[in] (rhs) Key of property.
   */
  bool operator<(const PropertyKey& rhs) const {
    return (this->full_key_ < rhs.full_key_);
  }

  /**
   * @brief Get property key with append information.
   * @return Made property key + append information.
   */
  std::string GetFullKey() const;

  /**
   * @brief Get property key.
   * @return Excludes the append information from the property key.
   */
  std::string GetPropertyKey() const;

  /**
   * @brief Get append information string (sorted).
   * @return append information string.
   */
  std::string GetAppendInfo() const;

  /**
   * @brief Set Channel ID value.
   * @param[in] (id) Channel ID.
   */
  void SetChannelId(uint32_t id);

  /**
   * @brief Get Channel ID from property key.
   * @param[out] (id) Extract Channel ID.
   * @return Status object.
   */
  Status GetChannelId(uint32_t* id) const;

 private:
  /**
   * @brief Parse the property key.
   * @param[in] (key) Key of property.
   */
  void ParseKey(const std::string& key);

  /**
   * @brief Parse the append information from property key.
   * @param[in] (key) Key of property.
   */
  void ParseAppendInfo(const std::string& key);

  /**
   * @brief Make property key with append information.
   */
  void MakeFullKey();

  /**
   * @brief Make append information string (sorted).
   * @return append information string.
   */
  std::string MakeAppendInfoStr() const;

  // property key + append information(sorted)
  std::string full_key_;
  // end position of property key
  std::string::size_type key_end_;
  // position of sorted append information
  std::string::size_type append_begin_;
  std::string::size_type append_end_;
  // append information table
  std::map<std::string, std::string> append_info_table_;
};

}  //  namespace util
}  //  namespace senscord

#endif  // LIB_CORE_UTIL_PROPERTY_UTILS_H_
