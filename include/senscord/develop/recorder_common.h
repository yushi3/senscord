/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_RECORDER_COMMON_H_
#define SENSCORD_DEVELOP_RECORDER_COMMON_H_

#include "senscord/config.h"

#if defined(SENSCORD_RECORDER) || defined(SENSCORD_PLAYER)

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

#include "senscord/serialize_define.h"
#include "senscord/property_types.h"

namespace senscord {

// type for writing raw_index.dat
enum RecordDataType {
  kRecordDataTypeRaw = 0,         /**< Single raw data */
  kRecordDataTypeCompositeRaw     /**< Composite raw data */
};

}   // namespace senscord

SENSCORD_SERIALIZE_ADD_ENUM(senscord::RecordDataType)

namespace senscord {

/**
 * @brief Raw data storage of one channel for raw_index.dat file.
 */
struct ChannelRawDataForRawIndex {
  uint64_t sequence_number;         /**< Sequence number */
  uint32_t channel_id;              /**< Channel ID */
  uint64_t caputured_timestamp;     /**< Caputured timestamp */
  uint64_t sent_time;               /**< Sent time */
  RecordDataType record_type;       /**< Record type */
  std::vector<uint8_t> rawdata;     /**< Raw data payload */

  SENSCORD_SERIALIZE_DEFINE(sequence_number, channel_id, caputured_timestamp,
      sent_time, record_type, rawdata)
};

/**
 * @brief Properties storage of one channel for record.
 */
struct ChannelPropertiesForRecord {
  /** Sequence number */
  uint64_t sequence_number;

  /** Pairs of property key and serialized channel property data */
  std::map<std::string, BinaryProperty> properties;

  SENSCORD_SERIALIZE_DEFINE(sequence_number, properties)
};

/**
 * @brief Properties storage of one stream for record.
 */
struct StreamPropertiesForRecord {
  /** Pairs of property key and serialized stream property data */
  std::map<std::string, BinaryProperty> properties;

  SENSCORD_SERIALIZE_DEFINE(properties)
};

/**
 * @brief Utility class for recorder and player.
 */
class RecordUtility {
 public:
  /**
   * @brief Get the path of recording info file.
   * @param[out] (filepath) File path.
   */
  static void GetInfoFilePath(std::string* filepath);

  /**
   * @brief Get the directory name of recording stream property.
   * @param[out] (directoryname) Directory name.
   */
  static void GetStreamPropertyDirectoryName(std::string* directoryname);

  /**
   * @brief Get the path of recording stream property file.
   * @param[in] (property_key) Property key.
   * @param[out] (filepath) File path.
   */
  static void GetStreamPropertyFilePath(
    const std::string& property_key, std::string* filepath);

  /**
   * @brief Get the directory name for channel recording.
   * @param[in] (channel_id) Channel ID.
   * @param[out] (directoryname) Directory name.
   */
  static void GetChannelDirectoryName(
    uint32_t channel_id, std::string* directoryname);

  /**
   * @brief Get the path of recording raw index file.
   * @param[out] (filepath) File path.
   */
  static void GetRawIndexFilePath(std::string* filepath);

  /**
   * @brief Get the name for raw data file.
   * @param[in] (sequence_number) Frame sequence number.
   * @param[out] (filename) File name.
   */
  static void GetRawDataFileName(
    uint64_t sequence_number, std::string* filename);

  /**
   * @brief Get the path of the channel properties file.
   * @param[in] (channel_id) Channel ID.
   * @param[out] (filepath) File path.
   */
  static void GetChannelPropertiesFilePath(
    uint32_t channel_id, std::string* filepath);

  /**
   * @brief Get the name of the channel properties file.
   * @param[out] (filename) File name.
   */
  static void GetChannelPropertiesFileName(std::string* filename);

  /**
   * @brief Returns true for recordable stream properties.
   * @param[in] (key) Property key.
   * @return True for recordable stream properties.
   */
  static bool IsRecordableProperty(const std::string& key);

  /**
   * @brief Returns true for recordable channel properties.
   * @param[in] (key) Property key.
   * @return True for recordable channel properties.
   */
  static bool IsRecordableChannelProperty(const std::string& key);

  /**
   * @brief Check whether property is the recording target.
   * @param[in] (key) Property key.
   * @return true means that property is the recording taget.
   */
  static bool IsRecordablePropertyForSkv(const std::string& key);

 private:
  /**
   * @brief Private constructor
   */
  RecordUtility() {}

  /**
   * @brief Private destructor
   */
  ~RecordUtility() {}
};

}   // namespace senscord

#endif  // defined(SENSCORD_RECORDER) || defined(SENSCORD_PLAYER)
#endif  // SENSCORD_DEVELOP_RECORDER_COMMON_H_
