/*
 * SPDX-FileCopyrightText: 2022-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util/property_utils.h"

#include <string>
#include <sstream>
#include <utility>  // for make_pair

#include "senscord/osal.h"
#include "senscord/senscord_types.h"

namespace senscord {
namespace util {

const char kAppendInfoChannel[] = "ch=";

/**
 * @brief Get property key with append information.
 * @return Made property key + append information.
 */
std::string PropertyKey::GetFullKey() const {
  return full_key_;
}

/**
 * @brief Get property key.
 * @return Excludes the append information from the property key.
 */
std::string PropertyKey::GetPropertyKey() const {
  return full_key_.substr(0, key_end_);
}

/**
 * @brief Get append information string (sorted).
 * @return append information string.
 */
std::string PropertyKey::GetAppendInfo() const {
  return full_key_.substr(append_begin_, append_end_);
}

/**
 * @brief Parse the property key.
 */
void PropertyKey::ParseKey(const std::string& key) {
  std::string::size_type spos = key.find("[");
  std::string::size_type epos = key.rfind("]");

  if (spos == std::string::npos) {
    if (epos == std::string::npos) {
      full_key_ = key;  // not exist '[]'.
      key_end_ = spos;
    } else {
      full_key_ = "";   // only ']'.
    }
  } else {
    if (epos != (key.size() - 1)) {
      full_key_ = "";   // only '[' or ']' not last position.
    } else if (spos == 0) {
      full_key_ = "";   // not exist property key.
    } else {
      full_key_ = key;  // normal format.
      key_end_ = spos;
      ParseAppendInfo(key);
    }
  }
}

/**
 * @brief Parse the append information from property key.
 * @param[in] (key) Key of property.
 */
void PropertyKey::ParseAppendInfo(const std::string& key) {
  std::string::size_type last_spos = key.rfind("[");
  std::string::size_type first_epos = key.find("]");
  std::stringstream info(key.substr(last_spos + 1, first_epos - last_spos - 1));

  std::string element;
  while (std::getline(info, element, ',')) {
    std::string::size_type pos = element.find("=");
    if (pos != std::string::npos) {
      std::string tag = element.substr(0, pos + 1);
      std::string val = element.substr(pos + 1);
      append_info_table_.insert(make_pair(tag, val));
    }
  }
}

/**
 * @brief Set Channel ID value.
 * @param[in] (channel_id) Channel ID.
 */
void PropertyKey::SetChannelId(uint32_t channel_id) {
  std::ostringstream buf;
  buf << channel_id;
  append_info_table_[kAppendInfoChannel] = buf.str();
  MakeFullKey();
}

/**
 * @brief Extract Channel ID from property key.
 * @param[out] (id) Extract Channel ID.
 * @return Status object.
 */
Status PropertyKey::GetChannelId(uint32_t* id) const {
  std::map<std::string, std::string>::const_iterator itr =
      append_info_table_.find(kAppendInfoChannel);
  if (itr == append_info_table_.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "append information is not found.");
  }

  uint64_t num = 0;
  int result = senscord::osal::OSStrtoull(itr->second.c_str(), NULL, 0, &num);
  if ((result == 0) && (num <= UINT32_MAX)) {
    *id = static_cast<uint32_t>(num);
    return Status::OK();
  }

  return SENSCORD_STATUS_FAIL(kStatusBlockCore,
      Status::kCauseNotFound, "append information(Channel ID) is over size.");
}

/**
 * @brief Make property key with append information.
 */
void PropertyKey::MakeFullKey() {
  if (!full_key_.empty() && !append_info_table_.empty()) {
    std::ostringstream make_info;
    make_info << GetPropertyKey() << "[";
    make_info << MakeAppendInfoStr();
    make_info << "]";
    full_key_ = make_info.str();
    std::string::size_type last_spos = full_key_.rfind("[");
    std::string::size_type first_epos = full_key_.find("]");
    append_begin_ = last_spos + 1;
    append_end_ = first_epos - last_spos - 1;
  }
}

/**
 * @brief Get append information string (sorted).
 * @return append information string.
 */
std::string PropertyKey::MakeAppendInfoStr() const {
  std::map<std::string, std::string>::const_iterator itr =
      append_info_table_.begin();
  std::map<std::string, std::string>::const_iterator end =
      append_info_table_.end();
  std::ostringstream info;
  for (; itr != end;) {
    info << itr->first << itr->second;
    ++itr;
    if (itr != end) {
      info << ",";
    }
  }
  return info.str();
}

}  // namespace util

/**
 * @brief Set Channel ID to property key.
 * @param[in] (key) Key of property.
 * @param[in] (channel_id) Channel ID to be set to key.
 * @return Property key with Channel ID assigned.
 */
std::string PropertyUtils::SetChannelId(
    const std::string& key, uint32_t channel_id) {
  util::PropertyKey param(key);
  param.SetChannelId(channel_id);
  return param.GetFullKey();
}

/**
 * @brief Get Channel ID from property key.
 * @param[in] (key) Key of property.
 * @param[out] (channel_id) Channel ID retrieved from key.
 * @return Status.
 */
Status senscord::PropertyUtils::GetChannelId(
    const std::string& key, uint32_t* channel_id) {
  if (channel_id == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "channel_id is NULL");
  }

  util::PropertyKey param(key);
  return param.GetChannelId(channel_id);
}

/**
 * @brief Get property key.
 * @param[in] (key) Key of property.
 * @return A value that excludes the Channel ID from the property key.
 */
std::string PropertyUtils::GetKey(const std::string& key) {
  util::PropertyKey param(key);
  std::string get_key = param.GetPropertyKey();
  if (get_key.empty()) {
    return key;
  }

  return get_key;
}

}  //  namespace senscord
