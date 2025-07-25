/*
 * SPDX-FileCopyrightText: 2020-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "player_stream_file_manager.h"

#include <inttypes.h>
#include <stdint.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "./player_component_util.h"
#include "senscord/logger.h"

static const char* kModuleName = "player_stream_file_manager";

namespace {
// info.xml element/attribute
const char* kElementRecord         = "record";
const char* kElementStream         = "stream";
const char* kElementFrameRate      = "framerate";
const char* kElementSkipFrame      = "skipframe";
const char* kElementProperties     = "properties";
const char* kElementProperty       = "property";
const char* kElementChannels       = "channels";
const char* kElementChannel        = "channel";
const char* kAttributeDate         = "date";
const char* kAttributeKey          = "key";
const char* kAttributeType         = "type";
const char* kAttributeNum          = "num";
const char* kAttributeDenom        = "denom";
const char* kAttributeRate         = "rate";
const char* kAttributeId           = "id";
const char* kAttributeDescription  = "description";
const char* kAttributeMask         = "mask";
const char* kAttributeValueTrue    = "true";
}  // namespace

/**
 * @brief Constructor.
 */
PlayerStreamFileManager::PlayerStreamFileManager()
    : info_xml_(), parser_(NULL) {}

/**
 * @brief Destructor.
 */
PlayerStreamFileManager::~PlayerStreamFileManager() {
  ClearStreamProperty();
}

/**
 * @brief Read the xml info from record file.
 * @param[in] (target_path) The target path of play property.
 * @return Status object.
 */
senscord::Status PlayerStreamFileManager::ReadStreamFile(
    const std::string& target_path) {
  InfoXmlParameter info_xml = {};
  senscord::Status status = ReadXmlInfo(target_path, &info_xml);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  // apply
  info_xml_ = info_xml;

  status = ReadStreamProperty(target_path, info_xml.stream.property_keys);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}

/**
 * @brief Read the xml info from record file.
 * @param[in] (target_path) The target path of play property.
 * @param[out] (info_xml) The analysis result of info.xml.
 * @return Status object.
 */
senscord::Status PlayerStreamFileManager::ReadXmlInfo(
    const std::string& target_path, InfoXmlParameter* info_xml) {
  std::string file_path;
  {
    std::string file_name;
    senscord::RecordUtility::GetInfoFilePath(&file_name);
    file_path = target_path + senscord::osal::kDirectoryDelimiter + file_name;
  }

  parser_ = new senscord::osal::OSXmlParser();
  if (parser_->Open(file_path) != 0) {
    delete parser_;
    parser_ = NULL;
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseInvalidArgument,
        "fail to open xml:%s", file_path.c_str());
  }

  senscord::osal::OSXmlNodeType type = senscord::osal::kOSXmlUnsupportedNode;
  std::string element;
  senscord::Status status;
  while (parser_->Parse(&type) == 0) {
    if (type == senscord::osal::kOSXmlElementNode) {
      parser_->GetElement(&element);
      if (element == kElementRecord) {
        status = ParseRecord(info_xml);
        if (!status.ok()) {
          SENSCORD_STATUS_TRACE(status);
          break;
        }
      } else {
        SENSCORD_LOG_WARNING(
            "unknown element : element=%s", element.c_str());
      }
    }
  }
  parser_->Close();
  delete parser_;
  parser_ = NULL;

  if (status.ok()) {
    if ((info_xml->stream.frame_num == 0) ||
        (info_xml->stream.frame_denom == 0)) {
      status = SENSCORD_STATUS_FAIL(
          kModuleName, senscord::Status::kCauseOutOfRange,
          "Invalid frame rate: num=%" PRIu32 ", denom=%" PRIu32,
          info_xml->stream.frame_num, info_xml->stream.frame_denom);
    }
  }

  return status;
}

/**
 * @brief Parse record element and obtain it as info.xml.
 * @param[out] (info_xml) The analysis result of info.xml.
 * @return Status object.
 */
senscord::Status PlayerStreamFileManager::ParseRecord(
    InfoXmlParameter* info_xml) {
  senscord::Status status = player::GetAttributeString(
    parser_, kAttributeDate, &(info_xml->record_date));
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  status = SENSCORD_STATUS_FAIL(kModuleName,
      senscord::Status::kCauseAborted, "parse stream failed");
  senscord::osal::OSXmlNodeType type = senscord::osal::kOSXmlUnsupportedNode;
  std::string element;
  while (parser_->Parse(&type) == 0) {
    if (type == senscord::osal::kOSXmlElementNode) {
      parser_->GetElement(&element);
      if (element == kElementStream) {
        status = ParseStream(&(info_xml->stream));
        if (!status.ok()) {
          SENSCORD_STATUS_TRACE(status);
          break;
        }
      } else if (element == kElementChannels) {
        status = ParseChannels(&(info_xml->channels));
        if (!status.ok()) {
          SENSCORD_STATUS_TRACE(status);
          break;
        }
      } else {
        SENSCORD_LOG_WARNING(
            "unknown element : element=%s", element.c_str());
      }
    }
  }

  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Parse stream element and obtain it as info.xml.
 * @param[out] (stream) The stream information of info.xml
 * @return Status object.
 */
senscord::Status PlayerStreamFileManager::ParseStream(
    InfoXmlStreamInfo* stream) {
  senscord::Status status = player::GetAttributeString(
      parser_, kAttributeKey, &(stream->key));
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  status = player::GetAttributeString(
      parser_, kAttributeType, &(stream->type));
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  senscord::Status result = SENSCORD_STATUS_FAIL(kModuleName,
      senscord::Status::kCauseAborted, "parse stream failed");
  senscord::osal::OSXmlNodeType type = senscord::osal::kOSXmlUnsupportedNode;
  while (parser_->Parse(&type) == 0) {
    std::string element;
    if (type == senscord::osal::kOSXmlElementNode) {
      parser_->GetElement(&element);

      if (element == kElementFrameRate) {
        // element framerate
        status = player::GetAttributeUInt32(
            parser_, kAttributeNum, &stream->frame_num);
        if (!status.ok()) {
          return SENSCORD_STATUS_TRACE(status);
        }

        status = player::GetAttributeUInt32(
            parser_, kAttributeDenom, &stream->frame_denom);
        if (!status.ok()) {
          return SENSCORD_STATUS_TRACE(status);
        }
      } else if (element == kElementSkipFrame) {
        // element skipframe
        status = player::GetAttributeUInt32(
            parser_, kAttributeRate, &stream->skip_frame);
        if (!status.ok()) {
          return SENSCORD_STATUS_TRACE(status);
        }
      } else if (element == kElementProperties) {
        // element properties
        status = ParseProperties(&(stream->property_keys));
        if (!status.ok()) {
          return SENSCORD_STATUS_TRACE(status);
        }
      }
    } else if (type == senscord::osal::kOSXmlElementEnd) {
      parser_->GetElement(&element);
      if (element == kElementStream) {
        // If it is the end tag of stream, it exits the loop.
        result = senscord::Status::OK();
        break;
      }
    }
  }

  return SENSCORD_STATUS_TRACE(result);
}

/**
 * @brief Parse properties element and obtain it as info xml.
 * @param[out] (property_keys) The list of stream property key of info.xml
 * @return Status object.
 */
senscord::Status PlayerStreamFileManager::ParseProperties(
    std::vector<std::string>* property_keys) {
  senscord::Status result = SENSCORD_STATUS_FAIL(kModuleName,
      senscord::Status::kCauseAborted, "parse properties failed");

  senscord::osal::OSXmlNodeType type = senscord::osal::kOSXmlUnsupportedNode;
  while (parser_->Parse(&type) == 0) {
    std::string element;
    if (type == senscord::osal::kOSXmlElementNode) {
      parser_->GetElement(&element);
      if (element == kElementProperty) {
        std::string key;
        senscord::Status status =
            player::GetAttributeString(parser_, kAttributeKey, &key);
        if (!status.ok()) {
          return SENSCORD_STATUS_TRACE(status);
        }
        property_keys->push_back(key);
      }
    } else if (type == senscord::osal::kOSXmlElementEnd) {
      parser_->GetElement(&element);
      if (element == kElementProperties) {
        // If it is the end tag of properties, it exits the loop.
        result = senscord::Status::OK();
        break;
      }
    }
  }

  return SENSCORD_STATUS_TRACE(result);
}

/**
 * @brief Parse stream element and obtain it as Config.
 * @param[out] (channel) The channel information of info.xml
 * @return Status object.
 */
senscord::Status PlayerStreamFileManager::ParseChannels(
    InfoXmlChannelList* channels) {
  senscord::Status result = SENSCORD_STATUS_FAIL(kModuleName,
      senscord::Status::kCauseAborted, "parse channels failed");

  senscord::osal::OSXmlNodeType type =
      senscord::osal::kOSXmlUnsupportedNode;
  while (parser_->Parse(&type) == 0) {
    std::string element;
    if (type == senscord::osal::kOSXmlElementNode) {
      parser_->GetElement(&element);
      if (element == kElementChannel) {
        uint32_t channel_id = 0;
        senscord::Status status =
            player::GetAttributeUInt32(parser_, kAttributeId, &channel_id);
        if (!status.ok()) {
          return SENSCORD_STATUS_TRACE(status);
        }

        InfoXmlChannelParameter channel = {};
        status = player::GetAttributeString(
            parser_, kAttributeType, &(channel.rawdata_type));
        if (!status.ok()) {
          return SENSCORD_STATUS_TRACE(status);
        }
        status = player::GetAttributeString(
            parser_, kAttributeDescription, &(channel.description));
        if (!status.ok()) {
          return SENSCORD_STATUS_TRACE(status);
        }

        std::string mask;
        status = player::GetAttributeString(parser_, kAttributeMask, &mask);
        if (mask == kAttributeValueTrue) {
          channel.mask = true;
          SENSCORD_LOG_DEBUG("channel_id=%" PRIu32 " mask=true", channel_id);
        }

        // append channel
        channels->insert(std::make_pair(channel_id, channel));
      }
    } else if (type == senscord::osal::kOSXmlElementEnd) {
      parser_->GetElement(&element);
      if (element == kElementChannels) {
        // If it is the end tag of channels, it exits the loop.
        result = senscord::Status::OK();
        break;
      }
    }
  }

  return SENSCORD_STATUS_TRACE(result);
}

/**
 * @brief Get the channel parameters of info.xml.
 * @param[out] (channels) The channel parameters of info.xml.
 */
void PlayerStreamFileManager::GetInfoXmlChannels(
    InfoXmlChannelList* channels) {
  *channels = info_xml_.channels;
}

/**
 * @brief Get the properties of xml info.
 * @param[out] (prop) Frame rate property.
 */
void PlayerStreamFileManager::GetFrameRate(senscord::FrameRateProperty* prop) {
  if (prop != NULL) {
    prop->num = info_xml_.stream.frame_num;
    prop->denom = info_xml_.stream.frame_denom;
  }
}

/**
 * @brief Read file for stream property.
 * @param[in] (target_path) The target path of play property.
 * @param[in] (key_list) The list of stream property key..
 * @return Status object.
 */
senscord::Status PlayerStreamFileManager::ReadStreamProperty(
    const std::string& target_path, const std::vector<std::string>& key_list) {
  senscord::Status status;
  for (size_t i = 0; i < key_list.size(); i++) {
    std::string file_name;
    senscord::RecordUtility::GetStreamPropertyFilePath(
        key_list[i], &file_name);

    std::string path =
        target_path + senscord::osal::kDirectoryDelimiter + file_name;
    std::vector<uint8_t> property_data;
    status = player::FileReadAllData(
        (const char*)path.c_str(), &property_data);
    if (status.ok()) {
      property_list_.insert(std::make_pair(key_list[i], property_data));
    } else {
      if (status.cause() == senscord::Status::kCauseResourceExhausted ||
          status.cause() == senscord::Status::kCauseNotFound) {
        SENSCORD_LOG_WARNING("can't read %s", path.c_str());
        // regard this case as OK, do not return status
      } else {
        return SENSCORD_STATUS_TRACE(status);
      }
    }
  }
  return senscord::Status::OK();
}

/**
 * @brief Clear for stream property.
 */
void PlayerStreamFileManager::ClearStreamProperty() {
  property_list_.clear();
}

/**
 * @brief Set stream property
 * @param[in] (key) The key of property
 * @param[in] (serialized_property) The serialized property
 * @param[in] (serialized_size) The size of serialized property
 * @return status object
 */
senscord::Status PlayerStreamFileManager::SetStreamProperty(
    const std::string& key, const void* serialized_property,
    size_t serialized_size) {
  StreamPropertyList::iterator itr = property_list_.find(key);
  if (itr == property_list_.end()) {
    SENSCORD_LOG_WARNING("can't found property: key=%s", key.c_str());
    // In this case, it' s OK to do nothing.
    return senscord::Status::OK();
  }

  // erase old property
  property_list_.erase(itr);

  // append new property
  std::vector<uint8_t> property_data(serialized_size);
  senscord::osal::OSMemcpy(
      property_data.data(), property_data.size(),
      serialized_property, serialized_size);
  property_list_.insert(std::make_pair(key, property_data));

  return senscord::Status::OK();
}

/**
 * @brief Get stream property
 * @param[in] (key) The key of property
 * @return The pointer of stream property data.
 *         If NULL, the data is not find
 */
const std::vector<uint8_t>* PlayerStreamFileManager::GetStreamProperty(
    const std::string& key) {
  StreamPropertyList::const_iterator itr = property_list_.find(key);
  if (itr != property_list_.end()) {
    return &(itr->second);
  }
  return NULL;
}

/**
 * @brief Get key list of stream property.
 * @param[out] (key_list) The list of stream properties.
 * @return Status object.
 */
senscord::Status PlayerStreamFileManager::GetStreamPropertyList(
    std::vector<std::string>* key_list) {
  if (key_list == NULL) {
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseInvalidArgument, "list is NULL");
  }
  *key_list = info_xml_.stream.property_keys;

  return senscord::Status::OK();
}

/**
 * @brief Get stream type
 * @return The type of stream
 */
const std::string& PlayerStreamFileManager::GetStreamType() {
  return info_xml_.stream.type;
}

/**
 * @brief Get the properties of xml info.
 * @param[out] (property) PlayFileInfo property.
 */
void PlayerStreamFileManager::GetPlayFileInfo(
    senscord::PlayFileInfoProperty* property) {
  if (property != NULL) {
    property->record_date = info_xml_.record_date;
    property->stream_key = info_xml_.stream.key;
    property->stream_type = info_xml_.stream.type;
  }
}
