/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_RECORD_UTILITY_H_
#define LIB_CORE_RECORD_RECORD_UTILITY_H_

#include <stdint.h>
#include <map>
#include <string>
#include <sstream>
#include "senscord/senscord.h"
#include "senscord/property_types.h"

namespace senscord {

/**
 * @brief Change to string from value.
 * @param[in] (num) Source value.
 * @return String from value.
 */
template <typename T>
std::string ToString(T num) {
  std::ostringstream ss;
  ss << num;
  return ss.str();
}

/**
 * @brief Change to zero-filled string from value.
 * @param[in] (num) Source value.
 * @param[in] (digit) Digit number.
 * @return String from value.
 */
std::string ToZeroFilledString(uint64_t num, uint32_t digit);

/**
 * @brief Utility class for record property
 */
class RecordPropertyUtility {
 public:
  /**
   * @brief Create the top directory.
   * @param[in,out] (property) Record property.
   * @param[in] (stream_key) Stream key.
   * @return Status object.
   */
  static Status CreateTopDirectory(
      RecordProperty* property, const std::string& stream_key);

  /**
   * @brief Gets append formats.
   * @param[in] (current_prop) The property of current property.
   * @param[in] (request_prop) The property of request property.
   * @param[out] (append_formats) Append formats.
   * @return Status object.
   */
  static Status GetAppendFormat(
      const RecordProperty& current_prop, const RecordProperty& request_prop,
      std::map<uint32_t, std::string>* append_formats);

 private:
  RecordPropertyUtility();
  ~RecordPropertyUtility();

  /**
   * @brief Create the directory name for stream.
   * @param[out] (dir_name) Directory name.
   * @param[in] (format) Directory name rule string.
   * @param[in] (stream_key) Stream key.
   * @return Status object.
   */
  static Status CreateTopDirectoryName(
      std::string* dir_name, const std::string& format,
      const std::string& stream_key);

  /**
   * @brief Verify that the directory name is correct.
   * @param[in] (format) Record property.
   * @return Status object.
   */
  static Status ValidateDirectoryName(const std::string& format);

  /**
   * @brief Replace keywords with a string.
   * @param[in,out] (replace_str) String to be replaced.
   * @param[in] (keyword) Keywords to be replaced.
   * @param[in] (replace) A string to replace the keyword with.
   */
  static void ReplaceKeyWordString(
      std::string* replace_str, const std::string& keyword,
      const std::string& replace);

  /**
   * @brief Create the directory.
   * @param[in,out] (path) Directory path.
   * @return Status object.
   */
  static Status CreateDirectory(std::string* path);

  /**
   * @brief Check same record path.
   * @param[in] (current_path) The path of current property.
   * @param[in] (request_path) The path of request property.
   * @return Status object.
   */
  static Status CheckSameRecordPath(
      const std::string& current_path, const std::string& request_path);

  /**
   * @brief Check same buffer_num.
   * @param[in] (current_num) The buffer_num of current property.
   * @param[in] (request_num) The buffer_num of request property.
   * @return Status object.
   */
  static Status CheckSameBufferNum(
      const uint32_t current_num, const uint32_t request_num);

  /**
   * @brief Check change record-type.
   * @param[in] (current_formats) The formats of current property.
   * @param[in] (request_formats) The formats of request property.
   * @param[out] (append_formats) Append formats.
   * @return Status object.
   */
  static Status CheckSameRecordType(
      const std::map<uint32_t, std::string>& current_formats,
      const std::map<uint32_t, std::string>& request_formats,
      std::map<uint32_t, std::string>* append_formats);
};

}  // namespace senscord

#endif  // LIB_CORE_RECORD_RECORD_UTILITY_H_
