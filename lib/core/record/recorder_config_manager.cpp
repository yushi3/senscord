/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/recorder_config_manager.h"
#include <utility>    // make_pair
#include "core/internal_types.h"

namespace senscord {

// recorder config element/attribute
static const char* kElementRecorders  = "recorders";
static const char* kElementRecorder   = "recorder";
static const char* kAttributeType     = "type";
static const char* kAttributeName     = "format";

/**
 * @brief Read recorders config.
 * @param[in] (filename) Recorder config file path.
 * @return Status object.
 */
Status RecorderConfigManager::ReadConfig(const std::string& filename) {
  if (is_read_) {
    return Status::OK();
  }

  // open config file.
  if (parser_.Open(filename) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "open error(%s)", filename.c_str());
  }

  Status status;
  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;

  while (parser_.Parse(&node_type) == 0) {
    std::string element;
    if (node_type == osal::kOSXmlElementNode) {
      if ((parser_.GetElement(&element)) != 0) {
        status = SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
            "xml parse failed in get element");
        break;
      }

      if (element == kElementRecorders) {
        // recorder tag
        if (!(status = ParseRecorders()).ok()) {
          break;
        }
      }
    }
  }

  // close
  parser_.Close();
  if (status.ok()) {
    is_read_ = true;
  } else {
    // clear
    format_list_.clear();
  }
  return status;
}

/**
 * @brief Get the recorder type name by format name.
 * @param[in] (format_name) Format name.
 * @param[out] (type_name) Type name (recorder name).
 * @return Status object.
 */
Status RecorderConfigManager::GetRecorderType(
    const std::string& format_name, std::string* type_name) const {
  if (type_name == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid argument");
  }

  FormatList::const_iterator itr = format_list_.find(format_name);
  if (itr == format_list_.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotSupported, "unknown format name");
  }
  *type_name = itr->second;
  return Status::OK();
}

/**
 * @brief Get the recordable format list.
 * @param[out] (formats) List of formats.
 * @return Status object.
 */
Status RecorderConfigManager::GetRecordableFormats(
    std::vector<std::string>* formats) const {
  if (formats == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid argument");
  }
  FormatList::const_iterator itr = format_list_.begin();
  FormatList::const_iterator end = format_list_.end();
  for (; itr != end; ++itr) {
    formats->push_back(itr->first);
  }
  return Status::OK();
}

/**
 * @brief Parse recorders element of config.
 * @return Status object.
 */
Status RecorderConfigManager::ParseRecorders() {
  Status status;

  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;
  while (parser_.Parse(&node_type) == 0) {
    std::string element;
    if (node_type == osal::kOSXmlElementNode) {
      if ((parser_.GetElement(&element)) != 0) {
        return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
            "xml parse failed in get element");
      }
      if (element == kElementRecorder) {
        // recorder tag
        if (!(status = ParseRecorder()).ok()) {
          break;
        }
      } else {
        SENSCORD_LOG_WARNING("unknown \"%s\" element, ignored",
            element.c_str());
      }
    } else if (node_type == osal::kOSXmlElementEnd) {
      // node end
      if ((parser_.GetElement(&element)) != 0) {
        return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
            "xml parse failed in get element");
      }
      if (element == kElementRecorders) {
        break;
      } else {
        SENSCORD_LOG_WARNING("unknown \"/%s\" element, ignored",
            element.c_str());
      }
    }
  }
  return status;
}

/**
 * @brief Parse recorder element of config.
 * @return Status object.
 */
Status RecorderConfigManager::ParseRecorder() {
  Status status;
  std::string format_name;
  std::string type_name;

  if (!(status = ParseAttribute(kAttributeName, &format_name)).ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  if (!(status = ParseAttribute(kAttributeType, &type_name)).ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  format_list_.insert(std::make_pair(format_name, type_name));
  return Status::OK();
}

/**
 * @brief Parse attribute of config.
 * @param[in] (attr_name) attribute name.
 * @param[out] (value) type attribute value.
 * @return Status object.
 */
Status RecorderConfigManager::ParseAttribute(
    const char* attr_name, std::string* value) {
  if ((parser_.GetAttribute(attr_name, value)) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "parse attribute \"%s\" failed", attr_name);
  }
  return Status::OK();
}

/**
 * @brief Constructor
 */
RecorderConfigManager::RecorderConfigManager() : is_read_(false) {}

/**
 * @brief Destructor
 */
RecorderConfigManager::~RecorderConfigManager() {}

}    // namespace senscord
