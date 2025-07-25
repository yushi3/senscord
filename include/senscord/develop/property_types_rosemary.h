/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_PROPERTY_TYPES_ROSEMARY_H_
#define SENSCORD_DEVELOP_PROPERTY_TYPES_ROSEMARY_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

#include "senscord/config.h"
#include "senscord/serialize_define.h"

namespace senscord {

/**
 * @brief Stream state definitions.
 */
enum SkvRecordType {
  kSkvRecordTypeCustomBuffer = 0,  /**< Record to CustomBuffer */
  kSkvRecordTypeCustomStream,      /**< Record to CustomStream */
};

}  // namespace senscord

SENSCORD_SERIALIZE_ADD_ENUM(senscord::SkvRecordType)

namespace senscord {

/**
 * SkvWriteProperty
 */
const char kSkvWritePropertyKey[] = "skv_write_property";

/**
 * @brief Skv write data.
 */
struct SkvWriteData {
  std::vector<uint8_t> data;
  SkvRecordType type;

  SENSCORD_SERIALIZE_DEFINE(data, type)
};

/**
 * @brief Property for record to skv file.
 */
struct SkvWriteProperty {
  std::map<std::string, SkvWriteData> write_list;

  SENSCORD_SERIALIZE_DEFINE(write_list)
};

}   // namespace senscord

#endif  // SENSCORD_DEVELOP_PROPERTY_TYPES_ROSEMARY_H_
