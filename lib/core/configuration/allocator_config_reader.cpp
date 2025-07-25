/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "configuration/allocator_config_reader.h"

#include "configuration/core_config.h"
#include "util/xml_parser.h"

namespace {

const char* kAttributeKey       = "key";
const char* kAttributeType      = "type";
const char* kAttributeCacheable = "cacheable";
const char* kAttributeName      = "name";
const char* kAttributeValue     = "value";

const char* kAttributeValueOn   = "on";

}  // namespace

namespace senscord {

/**
 * @brief Read allocator config.
 * @param[in] (file_path) Path of the allocator config file.
 * @param[out] (config) Allocator config.
 * @return Status object.
 */
Status AllocatorConfigReader::ReadConfig(
    const std::string& file_path,
    std::vector<AllocatorConfig>* config) {
  Status status;
  util::XmlParser parser;
  status = parser.Open(file_path);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  AllocatorConfig* current_config = NULL;

  util::XmlElement element = {};
  while (status.ok() && parser.NextElement(&element)) {
    int32_t depth = element.GetDepth();

    if ((depth == 1) &&
        (element.GetXPath() == "/allocators/allocator")) {
      AllocatorConfig tmp = {};
      // allocator key
      tmp.key = parser.GetAttributeString(kAttributeKey);
      if (tmp.key.empty()) {
        return SENSCORD_STATUS_FAIL(
            kStatusBlockCore, Status::kCauseInvalidArgument,
            "'key' not specified");
      }
      if (GetAllocatorConfig(config, tmp.key) != NULL) {
        return SENSCORD_STATUS_FAIL(
            kStatusBlockCore, Status::kCauseAlreadyExists,
            "key='%s' already exists", tmp.key.c_str());
      }
      // allocator type
      tmp.type = parser.GetAttributeString(kAttributeType);
      if (tmp.type.empty()) {
        return SENSCORD_STATUS_FAIL(
            kStatusBlockCore, Status::kCauseInvalidArgument,
            "'type' not specified");
      }
      // cacheable
      std::string cacheable = parser.GetAttributeString(kAttributeCacheable);
      if (cacheable == kAttributeValueOn) {
        tmp.cacheable = true;
      }
      config->push_back(tmp);
      current_config = &config->back();
    }

    if ((depth == 3) &&
        (element.GetXPath() == "/allocators/allocator/arguments/argument")) {
      // argument name
      std::string name;
      status = parser.GetAttribute(kAttributeName, &name);
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
      // argument value
      std::string value;
      status = parser.GetAttribute(kAttributeValue, &value);
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
      current_config->arguments[name] = value;
    }
  }

  parser.Close();
  return status;
}

}  // namespace senscord
