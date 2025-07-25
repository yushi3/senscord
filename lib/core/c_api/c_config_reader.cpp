/*
 * SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "c_api/c_config_reader.h"
#include <sstream>
#include <string>
#include <vector>
#include "logger/logger.h"
#include "util/xml_parser.h"

namespace {

// config element/attribute
const char* kAttributeName      = "name";
const char* kAttributeType      = "type";

const char* kTypeProperty       = "property";
const char* kTypeRawData        = "rawdata";
const char kTypeDelimiter       = '|';

/**
 * @brief Trim spaces and ltabs from strings.
 */
std::string TrimString(const std::string& str) {
  std::string result;
  const char* trimPattern = " \t\r\n";
  std::string::size_type left = str.find_first_not_of(trimPattern);
  if (left != std::string::npos) {
    std::string::size_type right = str.find_last_not_of(trimPattern);
    result = str.substr(left, right - left + 1);
  }
  return result;
}

}  // namespace

namespace senscord {
namespace c_api {

/**
 * @brief Read the converter information.
 * @param[in]  filename    Config file path.
 * @param[out] converters  Converter information read from the config file.
 * @return Status object.
 */
Status ConfigReader::ReadConverterInfo(
    const std::string& filename, std::vector<ConverterConfig>* converters) {
  Status status;
  util::XmlParser parser;
  status = parser.Open(filename);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  util::XmlElement element = {};
  while (status.ok() && parser.NextElement(&element)) {
    if ((element.GetDepth() == 3) &&
        (element.GetName() == "converter") &&
        (element.GetXPath() == "/sdk/core/converters/converter")) {
      ConverterConfig info = {};
      // attribute: name
      info.library_name = parser.GetAttributeString(kAttributeName);
      if (info.library_name.empty()) {
        continue;  // skip
      }
      // attribute: type
      std::string types = parser.GetAttributeString(kAttributeType);
      std::istringstream input(types);
      std::string type;
      while (std::getline(input, type, kTypeDelimiter)) {
        type = TrimString(type);
        if (type == kTypeProperty) {
          info.enable_property = true;
        } else if (type == kTypeRawData) {
          info.enable_rawdata = true;
        } else {
          SENSCORD_LOG_WARNING(
              "ConfigReader: name=%s, type=%s (invalid type: '%s')",
              info.library_name.c_str(), types.c_str(), type.c_str());
        }
      }
      if (!(info.enable_property || info.enable_rawdata)) {
        info.enable_property = true;
      }
      // add information.
      converters->push_back(info);
      SENSCORD_LOG_INFO(
          "ConfigReader: name=%s, property=%d, rawdata=%d",
          info.library_name.c_str(),
          info.enable_property, info.enable_rawdata);
    }
  }

  parser.Close();
  return Status::OK();
}

}  // namespace c_api
}  // namespace senscord
