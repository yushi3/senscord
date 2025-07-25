/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/record_utility.h"

#include <map>
#include <string>
#include <sstream>
#include <iomanip>    // setw
#include <utility>    // setfill
#include "senscord/osal.h"
#include "senscord/property_types.h"
#include "senscord/develop/recorder_common.h"
#include "senscord/develop/property_types_private.h"

namespace {

// Directory name rules
const char kDefaultDirectoryNameFormat[] =
    "${YYYY}${MM}${DD}_${hh}${mm}${ss}_${StreamKey}";
const char kNameReplaceKeywordYear[]      = "${YYYY}";
const char kNameReplaceKeywordMonth[]     = "${MM}";
const char kNameReplaceKeywordDay[]       = "${DD}";
const char kNameReplaceKeywordHour[]      = "${hh}";
const char kNameReplaceKeywordMinute[]    = "${mm}";
const char kNameReplaceKeywordSecond[]    = "${ss}";
const char kNameReplaceKeywordStreamKey[] = "${StreamKey}";

}  // namespace

namespace senscord {

/**
 * @brief Change to zero-filled string from value.
 * @param[in] (num) Source value.
 * @param[in] (digit) Digit number.
 * @return String from value.
 */
std::string ToZeroFilledString(uint64_t num, uint32_t digit) {
  std::stringstream ss;
  ss << std::setw(digit) << std::setfill('0') << num;
  return ss.str();
}

/**
 * @brief Get the path of recording info file.
 * @param[out] (filepath) File path.
 */
void RecordUtility::GetInfoFilePath(std::string* filepath) {
  *filepath = "info.xml";
}

/**
 * @brief Get the directory name of recording stream property.
 * @param[out] (directoryname) Directory name.
 */
void RecordUtility::GetStreamPropertyDirectoryName(
    std::string* directoryname) {
  *directoryname = "properties";
}

/**
 * @brief Get the path of recording stream property file.
 * @param[in] (property_key) Property key.
 * @param[out] (filepath) File path.
 */
void RecordUtility::GetStreamPropertyFilePath(
    const std::string& property_key, std::string* filepath) {
  GetStreamPropertyDirectoryName(filepath);
  *filepath += osal::kDirectoryDelimiter + property_key;
}

/**
 * @brief Get the directory name for channel recording.
 * @param[in] (channel_id) Channel ID.
 * @param[out] (directoryname) Directory name.
 */
void RecordUtility::GetChannelDirectoryName(
    uint32_t channel_id, std::string* directoryname) {
  std::stringstream ss;
  ss << "channel_0x";
  ss << std::setw(8) << std::setfill('0') << std::hex <<  channel_id;
  *directoryname = ss.str();
}

/**
 * @brief Get the path of recording raw index file.
 * @param[out] (filepath) File path.
 */
void RecordUtility::GetRawIndexFilePath(std::string* filepath) {
  *filepath = "raw_index.dat";
}

/**
 * @brief Get the name for raw data file.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[out] (filename) File name.
 */
void RecordUtility::GetRawDataFileName(
    uint64_t sequence_number, std::string* filename) {
  std::stringstream ss;
  ss << "rawdata_" << sequence_number;
  *filename = ss.str();
}

/**
 * @brief Get the path of the channel properties file.
 * @param[in] (channel_id) Channel ID.
 * @param[out] (filepath) File path.
 */
void RecordUtility::GetChannelPropertiesFilePath(
    uint32_t channel_id, std::string* filepath) {
  GetChannelDirectoryName(channel_id, filepath);
  *filepath += osal::kDirectoryDelimiter;
  std::string filename;
  GetChannelPropertiesFileName(&filename);
  *filepath += filename;
}

/**
 * @brief Get the name of the channel properties file.
 * @param[out] (filename) File name.
 */
void RecordUtility::GetChannelPropertiesFileName(std::string* filename) {
  *filename += "properties.dat";
}

/**
 * @brief Returns true for recordable stream properties.
 * @param[in] (key) Property key.
 * @return True for recordable stream properties.
 */
bool RecordUtility::IsRecordableProperty(const std::string& key) {
  const char* kExcludedKeys[] = {
    kStreamKeyPropertyKey,
    kStreamTypePropertyKey,
    kStreamStatePropertyKey,
    kFrameBufferingPropertyKey,
    kCurrentFrameNumPropertyKey,
    kUserDataPropertyKey,
    kSkipFramePropertyKey,
    kFrameRatePropertyKey,
    kRecordPropertyKey,
    kRecorderListPropertyKey,
    kChannelInfoPropertyKey,
    kChannelMaskPropertyKey,
    kRegisterAccess8PropertyKey,
    kRegisterAccess16PropertyKey,
    kRegisterAccess32PropertyKey,
    kRegisterAccess64PropertyKey,
    kPlayPropertyKey,
    kPlayModePropertyKey,
    kPlayFileInfoPropertyKey,
    kPlayPositionPropertyKey,
    kRegisterEventPropertyKey,
    kUnregisterEventPropertyKey,
    kFrameExtensionPropertyKey
  };
  const size_t count = sizeof(kExcludedKeys) / sizeof(kExcludedKeys[0]);
  for (size_t i = 0; i < count; ++i) {
    if (key == kExcludedKeys[i]) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Returns true for recordable channel properties.
 * @param[in] (key) Property key.
 * @return True for recordable channel properties.
 */
bool RecordUtility::IsRecordableChannelProperty(const std::string& key) {
  const char* kExcludedKeys[] = {
    kPlayPositionPropertyKey,
  };
  const size_t count = sizeof(kExcludedKeys) / sizeof(kExcludedKeys[0]);
  for (size_t i = 0; i < count; ++i) {
    if (key == kExcludedKeys[i]) {
      return false;
    }
  }
  return true;
}

#ifdef SENSCORD_RECORDER_SKV
/**
 * @brief Check whether property is the recording target.
 * @param[in] (key) Property key.
 * @return true means that property is the recording target.
 */
bool RecordUtility::IsRecordablePropertyForSkv(const std::string& key) {
  const char* kExcludedKeys[] = {
    kStreamKeyPropertyKey,
    kStreamStatePropertyKey,
    kStreamTypePropertyKey,
    kSkipFramePropertyKey,
    kFrameBufferingPropertyKey,
    kCurrentFrameNumPropertyKey,
    kUserDataPropertyKey,
    kRecordPropertyKey,
    kRecorderListPropertyKey,
    kRegisterAccess8PropertyKey,
    kRegisterAccess16PropertyKey,
    kRegisterAccess32PropertyKey,
    kRegisterAccess64PropertyKey,
    kPlayPropertyKey,
    kPlayModePropertyKey,
    kPlayFileInfoPropertyKey,
    kRegisterEventPropertyKey,
    kUnregisterEventPropertyKey,
    kFrameExtensionPropertyKey
  };

  for (uint32_t i = 0; i < sizeof(kExcludedKeys) / sizeof(char*); ++i) {
    if (key == kExcludedKeys[i]) {
      return false;
    }
  }
  return true;
}
#endif  // SENSCORD_RECORDER_SKV

/**
 * @brief Create the top directory.
 * @param[in,out] (property) Record property.
 * @param[in] (stream_key) Stream key.
 * @return Status object.
 */
Status RecordPropertyUtility::CreateTopDirectory(
    RecordProperty* property, const std::string& stream_key) {
  std::string dir_name;
  // apply default name rule
  if (property->name_rules[kRecordDirectoryTop].empty()) {
    property->name_rules[kRecordDirectoryTop] = kDefaultDirectoryNameFormat;
  }

  Status status = CreateTopDirectoryName(
      &dir_name, property->name_rules[kRecordDirectoryTop], stream_key);
  if (status.ok()) {
    property->path += osal::kDirectoryDelimiter;
    property->path += dir_name;
    // create directory
    status = CreateDirectory(&property->path);
  }

  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Create the directory name for stream.
 * @param[out] (dir_name) Directory name.
 * @param[in] (format) Directory name rule string.
 * @param[in] (stream_key) Stream key.
 */
Status RecordPropertyUtility::CreateTopDirectoryName(
    std::string* dir_name, const std::string& format,
    const std::string& stream_key) {
  // get current time
  osal::OSSystemTime time = {};
  osal::OSGetLocalTime(&time);

  // replace strings
  std::string tmp_name = format;
  ReplaceKeyWordString(&tmp_name,
      kNameReplaceKeywordYear, ToZeroFilledString(time.year, 4));
  ReplaceKeyWordString(&tmp_name,
      kNameReplaceKeywordMonth, ToZeroFilledString(time.month, 2));
  ReplaceKeyWordString(&tmp_name,
      kNameReplaceKeywordDay, ToZeroFilledString(time.day, 2));
  ReplaceKeyWordString(&tmp_name,
      kNameReplaceKeywordHour, ToZeroFilledString(time.hour, 2));
  ReplaceKeyWordString(&tmp_name,
      kNameReplaceKeywordMinute, ToZeroFilledString(time.minute, 2));
  ReplaceKeyWordString(&tmp_name,
      kNameReplaceKeywordSecond, ToZeroFilledString(time.second, 2));
  ReplaceKeyWordString(&tmp_name,
      kNameReplaceKeywordStreamKey, stream_key);

  // verify strings
  Status status = ValidateDirectoryName(tmp_name);
  if (status.ok()) {
    *dir_name = tmp_name;
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Verify that the directory name is correct.
 * @param[in] (format) Record property.
 * @return Status object.
 */
Status RecordPropertyUtility::ValidateDirectoryName(
    const std::string& dir_name) {
  // verify characters. (A-Z,a-z,0-9,'.','-','_')
  for (std::string::const_iterator itr = dir_name.begin(),
      end = dir_name.end(); itr != end; ++itr) {
    if (*itr >= 'A' && *itr <= 'Z') {
      continue;
    }
    if (*itr >= 'a' && *itr <= 'z') {
      continue;
    }
    if (*itr >= '0' && *itr <= '9') {
      continue;
    }
    if (itr != dir_name.begin() &&
        (*itr == '.' || *itr == '-' || *itr == '_')) {
      continue;
    }
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "directory name contains illegal characters. (%s)", dir_name.c_str());
  }
  return Status::OK();
}

/**
 * @brief Replace keywords with a string.
 * @param[in,out] (replace_str) String to be replaced.
 * @param[in] (keyword) Keywords to be replaced.
 * @param[in] (replace) A string to replace the keyword with.
 */
void RecordPropertyUtility::ReplaceKeyWordString(
    std::string* replaced_str, const std::string& keyword,
    const std::string& replace) {
  if (keyword.empty()) {
    return;
  }

  std::string::size_type pos = 0;
  size_t keyword_length = keyword.length();
  size_t replace_length = replace.length();

  while ((pos = replaced_str->find(keyword, pos)) != std::string::npos) {
    replaced_str->replace(pos, keyword_length, replace);
    pos += replace_length;
  }
}

/**
 * @brief Create the directory.
 * @param[in,out] (path) Directory path.
 * @return Status object.
 */
Status RecordPropertyUtility::CreateDirectory(std::string* path) {
  std::string tmp_path = *path;
  uint32_t suffix_number = 0;
  while (true) {
    int32_t ret = osal::OSMakeDirectory(tmp_path.c_str());
    if (ret == 0) {
      break;
    } else {
      if (osal::OSGetErrorCause(ret) == osal::kErrorAlreadyExists) {
        std::stringstream ss;
        ss << *path << "_" << suffix_number;
        tmp_path = ss.str();
        ++suffix_number;
      } else {
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseAborted,
            "failed to create directory: path=%s, ret=0x%" PRIx32,
            tmp_path.c_str(), ret);
      }
    }
  }
  *path = tmp_path;
  return Status::OK();
}

/**
 * @brief Gets append formats.
 * @param[in] (current_prop) The property of current property.
 * @param[in] (request_prop) The property of request property.
 * @param[out] (append_formats) Append formats.
 * @return Status object.
 */
Status RecordPropertyUtility::GetAppendFormat(
    const RecordProperty& current_prop, const RecordProperty& request_prop,
    std::map<uint32_t, std::string>* append_formats) {
  Status status = CheckSameRecordPath(current_prop.path, request_prop.path);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  status = CheckSameBufferNum(
      current_prop.buffer_num, request_prop.buffer_num);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  status = CheckSameRecordType(
      current_prop.formats, request_prop.formats, append_formats);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return Status::OK();
}

/**
 * @brief Check change record path.
 * @param[in] (current_path) The path of current property.
 * @param[in] (request_path) The path of request property.
 * @return Status object.
 */
Status RecordPropertyUtility::CheckSameRecordPath(
    const std::string& current_path, const std::string& request_path) {
  // Unspecified path.
  if (request_path.empty()) {
    return Status::OK();
  }
  // Same as path with record directory.
  if (current_path == request_path) {
    return Status::OK();
  }
  // Same as path without record directory.
  size_t length = current_path.find_last_of(osal::kDirectoryDelimiter);
  std::string without_record_path = current_path.substr(0, length);
  if (without_record_path == request_path) {
    return Status::OK();
  }
#ifdef SENSCORD_RECORDER_SKV
  // Same as path of set property (two directory paths up).
  length = without_record_path.find_last_of(osal::kDirectoryDelimiter);
  if (without_record_path.substr(0, length) == request_path) {
    return Status::OK();
  }
#endif  // SENSCORD_RECORDER_SKV
  return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseInvalidArgument,
      "Different path specified: cur=%s, req=%s",
      current_path.c_str(), request_path.c_str());
}

/**
 * @brief Check change buffer_num.
 * @param[in] (current_num) The buffer_num of current property.
 * @param[in] (request_num) The buffer_num of request property.
 * @return Status object.
 */
Status RecordPropertyUtility::CheckSameBufferNum(
    const uint32_t current_num, const uint32_t request_num) {
  // Unspecified buffer_num.
  if (request_num == 0) {
    return Status::OK();
  }
  // Same buffer_num.
  if (current_num == request_num) {
    return Status::OK();
  }
  return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseInvalidArgument,
      "Different buffer_num specified: cur=%" PRIu32 ", req=%" PRIu32,
      current_num, request_num);
}

/**
 * @brief Check change record-type.
 * @param[in] (current_formats) The formats of current property.
 * @param[in] (request_formats) The formats of request property.
 * @param[out] (append_formats) Append formats.
 * @return Status object.
 */
Status RecordPropertyUtility::CheckSameRecordType(
    const std::map<uint32_t, std::string>& current_formats,
    const std::map<uint32_t, std::string>& request_formats,
    std::map<uint32_t, std::string>* append_formats) {
  for (std::map<uint32_t, std::string>::const_iterator
      itr = request_formats.begin(), end = request_formats.end();
      itr != end; ++itr) {
    std::map<uint32_t, std::string>::const_iterator found =
        current_formats.find(itr->first);
    // Apply appended formats of the new channel.
    if (found == current_formats.end()) {
      (*append_formats)[itr->first] = itr->second;
      continue;
    }

    // Check the record-type change of the existing channel.
    if (found->second != itr->second) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument,
          "Different record-type specified: ch_id=%" PRIu32 ", cur=%s, req=%s",
          itr->first, found->second.c_str(), itr->second.c_str());
    }
  }
  return Status::OK();
}

}   // namespace senscord
