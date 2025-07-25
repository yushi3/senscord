/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "config_manager.h"

#include <inttypes.h>
#include <map>
#include <utility>      // std::make_pair
#include <algorithm>    // std::find

#include "server_log.h"
#include "internal_types.h"

namespace senscord {
namespace server {

// server config element/attribute
static const char* kElementServer               = "server";
static const char* kElementStreams              = "streams";
static const char* kElementStream               = "stream";
static const char* kElementFrame                = "frame";
static const char* kElementDefaults             = "defaults";
static const char* kAttributeAddress            = "address";
static const char* kAttributeAddressPrimary     = "addressPrimary";
static const char* kAttributeAddressSecondary   = "addressSecondary";
static const char* kAttributeClient             = "client";
static const char* kAttributeKey                = "key";
static const char* kAttributeBuffering          = "buffering";
static const char* kAttributeNum                = "num";
static const char* kAttributeFormat             = "format";
static const char* kValueBufferingFormatDiscard   = "discard";
static const char* kValueBufferingFormatOverwrite = "overwrite";
/** @deprecated "queue" has been replaced by "discard". */
static const char* kValueBufferingFormatQueue     = "queue";
/** @deprecated "ring" has been replaced by "overwrite". */
static const char* kValueBufferingFormatRing      = "ring";

/**
 * @brief Convert numeric character to int type and return.
 * @param (source) The numeric character of the conversion source.
 * @param (result) To store the converted result
 * @return True is a success, False fail.
 */
bool StrToInt(const std::string& source, int32_t* result) {
  if (source.empty()) {
    return false;
  }

  char* endptr = NULL;
  int64_t num = INT64_MIN;
  if (osal::OSStrtoll(
      source.c_str(), &endptr, osal::kOSRadixAuto, &num) != 0) {
    return false;
  }

  if (endptr == NULL) {
    return false;
  }

  // Characters that can not be converted to numbers are errors
  if (*endptr != '\0') {
    return false;
  }

  // Numeric values that can not be converted to int type are errors
  if ((num > INT32_MAX) || (num < INT32_MIN)) {
    return false;
  }

  *result = static_cast<int32_t>(num);

  return true;
}

/**
 * @brief Constructor.
 */
ConfigManager::ConfigManager() {
  mutex_ = NULL;
  osal::OSCreateMutex(&mutex_);

  parser_ = new osal::OSXmlParser();

  ClearConfig();
}

/**
  * @brief Destructor.
  */
ConfigManager::~ConfigManager() {
  delete parser_;
  parser_ = NULL;

  if (mutex_ != NULL) {
    osal::OSDestroyMutex(mutex_);
    mutex_ = NULL;
  }
}

/**
 * @brief Read the specified Config file
 * @param[in] (config_path) Path of config file.
 * @return Status object.
 */
Status ConfigManager::ReadConfig(const std::string& config_path) {
  Status ret;
  osal::OSLockMutex(mutex_);

  do {
    if (current_config_path_ == config_path) {
      SENSCORD_SERVER_LOG_DEBUG("already read");
      break;
    }
    ClearConfig();

    ret = ParseConfig(config_path);
    if (!ret.ok()) {
      SENSCORD_STATUS_TRACE(ret);
      break;
    }

    current_config_path_ = config_path;
  } while (false);

  if (!ret.ok()) {
    ClearConfig();
  }

  osal::OSUnlockMutex(mutex_);
  return ret;
}

/**
 * @brief Set the server configuration.
 * @param[in] (server_config) Server configuration to be set.
 * @return Status object.
 */
Status ConfigManager::SetConfig(const ServerConfig& server_config) {
  osal::OSLockMutex(mutex_);
  ClearConfig();
  server_config_ = server_config;
  // In the case of an empty stream key, update the default setting.
  std::map<std::string, OpenStreamSetting>::iterator itr =
      server_config_.streams.find("");
  if (itr != server_config_.streams.end()) {
    default_stream_setting_ = itr->second;
    server_config_.streams.erase(itr);
  }
  osal::OSUnlockMutex(mutex_);
  return Status::OK();
}

/**
 * @brief Get the address required for Connection::Bind().
 * @param[out] (address) The address of the binding.
 * @return Status object.
 */
Status ConfigManager::GetBindAddress(std::string* address) const {
  if (address == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  osal::OSLockMutex(mutex_);
  *address = server_config_.bind_config;
  osal::OSUnlockMutex(mutex_);
  return Status::OK();
}

/**
 * @brief Get the secondary address used for Connection::Bind().
 * @param[out] (address) The secondary bind address.
 * @return Status object.
 */
Status ConfigManager::GetSecondaryBindAddress(std::string* address) const {
  if (address == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  Status status;
  osal::OSLockMutex(mutex_);
  if (!server_config_.bind_config2.empty()) {
    *address = server_config_.bind_config2;
  } else {
    status = SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseNotFound, "%s is empty", kAttributeAddressSecondary);
  }
  osal::OSUnlockMutex(mutex_);
  return status;
}

/**
 * @brief Get whether the client function is enabled or not.
 * @param[out] (enabled) If enabled returns true.
 * @return Status object.
 */
Status ConfigManager::GetClientEnabled(bool* enabled) const {
  if (enabled == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  osal::OSLockMutex(mutex_);
  *enabled = server_config_.is_enabled_client;
  osal::OSUnlockMutex(mutex_);
  return Status::OK();
}

/**
 * @brief Search by stream key and return stream config.
 * @param[in] (stream_key) Search stream key.
 * @param[out] (config) Return stream config.
 * @return Status object.
 */
Status ConfigManager::GetStreamConfigByStreamKey(
    const std::string& stream_key, OpenStreamSetting* config) const {
  if (config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  osal::OSLockMutex(mutex_);
  std::map<std::string, OpenStreamSetting>::const_iterator itr =
      server_config_.streams.find(stream_key);
  if (itr != server_config_.streams.end()) {
    *config = itr->second;
  } else {
    *config = default_stream_setting_;
  }
  osal::OSUnlockMutex(mutex_);
  return Status::OK();
}

/**
 * @brief Verify whether it is a supported stream.
 * @param[in] (supported_streams) Supported streams list.
 */
void ConfigManager::VerifySupportedStream(
    const std::vector<StreamTypeInfo>& supported_streams) const {
  osal::OSLockMutex(mutex_);
  std::map<std::string, OpenStreamSetting>::const_iterator itr =
      server_config_.streams.begin();
  std::map<std::string, OpenStreamSetting>::const_iterator end =
      server_config_.streams.end();
  for (; itr != end; ++itr) {
    bool found = false;
    std::vector<StreamTypeInfo>::const_iterator stream_itr =
        supported_streams.begin();
    std::vector<StreamTypeInfo>::const_iterator stream_end =
        supported_streams.end();
    for (; stream_itr != stream_end; ++stream_itr) {
      if (stream_itr->key == itr->first) {
        found = true;
        break;
      }
    }
    if (!found) {
      SENSCORD_SERVER_LOG_WARNING("unsupported stream key: %s",
          itr->first.c_str());
    }
  }
  osal::OSUnlockMutex(mutex_);
}

/**
 * @brief Clear the read Config information.
 */
void ConfigManager::ClearConfig() {
  current_config_path_.clear();
  server_config_.bind_config.clear();
  server_config_.bind_config2.clear();
  server_config_.is_enabled_client = false;
  server_config_.streams.clear();
  SetDefaultStreamConfig(&default_stream_setting_);
}

/**
 * @brief Set default config to Config.
 * @param[out] (config) Where to set the default config.
 */
void ConfigManager::SetDefaultStreamConfig(OpenStreamSetting* config) {
  config->frame_buffering.buffering = kBufferingOn;
  config->frame_buffering.num = 0;  // unlimited
  config->frame_buffering.format = kBufferingFormatDefault;
}

/**
 * @brief Analysis process of Config file
 * @param[in] (filename) Path of config file.
 * @return Status object.
 */
Status ConfigManager::ParseConfig(const std::string& filename) {
  if (parser_->Open(filename) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseInvalidArgument,
        "file open error : filename=%s", filename.c_str());
  }

  Status ret;
  bool success = false;

  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;
  // Perform analysis to the end of the file.
  while (parser_->Parse(&node_type) == 0) {
    std::string element;
    if (node_type == osal::kOSXmlElementNode) {
      parser_->GetElement(&element);
      if (element == kElementServer) {
        ret = ParseServer();
        SENSCORD_STATUS_TRACE(ret);
        if (!ret.ok()) {
          break;
        }
        success = true;
      } else {
        SENSCORD_SERVER_LOG_WARNING(
            "unknown element is ignored : element=%s", element.c_str());
      }
    }
  }
  parser_->Close();

  if (!success && ret.ok()) {
    ret = SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseAborted, "parse config failed");
  }

  return ret;
}

/**
 * @brief Parse server element and obtain it as Config.
 * @return Status object.
 */
Status ConfigManager::ParseServer() {
  Status ret;

  // attribute: address or addressPrimary (optional)
  ret = ParseAttributeAddress(&server_config_.bind_config);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    return ret;
  }

  // attribute: addressSecondary (optional)
  ParseAttributeAddressSecondary(&server_config_.bind_config2);

  // parse the client attribute.
  ParseAttributeClient(&server_config_.is_enabled_client);

  bool success = false;
  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;
  while (parser_->Parse(&node_type) == 0) {
    std::string element;
    if (node_type == osal::kOSXmlElementNode) {
      parser_->GetElement(&element);
      Status status = ParseServerElementNode(element);
      SENSCORD_STATUS_TRACE(status);
      if (!status.ok()) {
        ret = status;
        break;
      }
    } else if (node_type == osal::kOSXmlElementEnd) {
      parser_->GetElement(&element);
      if (element == kElementServer) {
        success = true;
        break;
      }
    }
  }

  if (!success && ret.ok()) {
    ret = SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseAborted, "parse server failed");
  }

  return ret;
}

/**
 * @brief Analyze the element nodes of the server.
 * @param[in] (element) Element name.
 * @return Status object.
 */
Status ConfigManager::ParseServerElementNode(const std::string& element) {
  Status ret;

  if (element == kElementStreams) {
    ret = ParseStreams();
    SENSCORD_STATUS_TRACE(ret);
  } else {
    SENSCORD_SERVER_LOG_WARNING(
        "unknown element is ignored : element=%s", element.c_str());
  }

  return ret;
}

/**
 * @brief Parse streams element and obtain it as Config.
 * @return Status object.
 */
Status ConfigManager::ParseStreams() {
  Status ret;
  bool success = false;

  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;
  while (parser_->Parse(&node_type) == 0) {
    std::string element;
    if (node_type == osal::kOSXmlElementNode) {
      parser_->GetElement(&element);
      Status status = ParseStreamsElementNode(element);
      SENSCORD_STATUS_TRACE(status);
      if (!status.ok()) {
        ret = status;
        break;
      }
    } else if (node_type == osal::kOSXmlElementEnd) {
      parser_->GetElement(&element);
      if (element == kElementStreams) {
        // If it is the end tag of streams, it exits the loop.
        success = true;
        break;
      }
    }
  }

  if (!success && ret.ok()) {
    ret = SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseAborted, "parse streams failed");
  }

  return ret;
}

/**
 * @brief Analyze the element nodes of the streams.
 * @param[in] (element) Element name.
 * @return Status object.
 */
Status ConfigManager::ParseStreamsElementNode(const std::string& element) {
  Status ret;

  if (element == kElementStream) {
    ret = ParseStream();
    SENSCORD_STATUS_TRACE(ret);
  } else if (element == kElementDefaults) {
    ret = ParseDefaults();
    SENSCORD_STATUS_TRACE(ret);
  } else {
    SENSCORD_SERVER_LOG_WARNING(
        "unknown element is ignored : element=%s", element.c_str());
  }

  return ret;
}

/**
 * @brief Parse stream element and obtain it as Config.
 * @return Status object.
 */
Status ConfigManager::ParseStream() {
  std::string key;
  Status ret = ParseAttributeKey(&key);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    return ret;
  }

  StreamSetting stream_config = {};
  stream_config.update = false;
  SetDefaultStreamConfig(&stream_config.open_setting);
  ret = ParseStreamChildNode(&stream_config);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    return ret;
  }

  if (stream_config.update) {
    server_config_.streams.insert(
        std::make_pair(key, stream_config.open_setting));
  }

  return Status::OK();
}

/**
 * @brief Analyze children in stream and reflect on Config
 * @param[out] (stream_config) Where to store the acquired config.
 * @return Status object.
 */
Status ConfigManager::ParseStreamChildNode(StreamSetting* stream_config) {
  Status ret;
  bool success = false;

  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;
  while (parser_->Parse(&node_type) == 0) {
    std::string element;
    if (node_type == osal::kOSXmlElementNode) {
      parser_->GetElement(&element);
      Status status = ParseStreamElementNode(element, stream_config);
      SENSCORD_STATUS_TRACE(status);
      if (!status.ok()) {
        ret = status;
        break;
      }
    } else if (node_type == osal::kOSXmlElementEnd) {
      parser_->GetElement(&element);
      if (element == kElementStream) {
        success = true;
        break;
      }
    }
  }

  if (!success && ret.ok()) {
    ret = SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseAborted, "parse stream child node failed");
  }

  return ret;
}

/**
 * @brief Analyze the element nodes of the stream.
 * @param[in] (element) Element name.
 * @param[out] (stream_config) Where to store the acquired config.
 * @return Status object.
 */
Status ConfigManager::ParseStreamElementNode(
    const std::string& element, StreamSetting* stream_config) {
  if (element == kElementFrame) {
    ParseFrame(&stream_config->open_setting.frame_buffering);
    stream_config->update = true;
  } else {
    SENSCORD_SERVER_LOG_WARNING(
        "unknown element is ignored : element=%s", element.c_str());
  }

  return Status::OK();
}

/**
 * @brief Parse frame element and obtain it as Config.
 * @param[out] (frame_buffering)  Where to store the acquired config.
 */
void ConfigManager::ParseFrame(FrameBuffering* frame_buffering) {
  ParseAttributeBuffering(&frame_buffering->buffering);
  ParseAttributeBufferingNum(&frame_buffering->num);
  ParseAttributeBufferingFormat(&frame_buffering->format);
}

/**
 * @brief Get the value of the Buffer attribute of Frame.
 * @param[out] (buffering) Where to store the acquired config.
 */
void ConfigManager::ParseAttributeBuffering(Buffering* buffering) {
  std::string value;
  if (parser_->GetAttribute(kAttributeBuffering, &value) == 0) {
    if (value == "on") {
      *buffering = kBufferingOn;
    } else if (value == "off") {
      *buffering = kBufferingOff;
    } else {
      SENSCORD_SERVER_LOG_WARNING(
          "unknown attribute value (%s=%s), use default value : %d",
          kAttributeBuffering, value.c_str(), *buffering);
    }
  } else {
    SENSCORD_SERVER_LOG_INFO(
        "%s attribute is not defined, use default value : %d",
        kAttributeBuffering, *buffering);
  }
}

/**
 * @brief Get the value of the Buffer attribute of Frame.
 * @param[out] (num) Where to store the acquired config.
 */
void ConfigManager::ParseAttributeBufferingNum(int32_t* num) {
  std::string value;
  if (parser_->GetAttribute(kAttributeNum, &value) == 0) {
    if (!StrToInt(value, num)) {
      SENSCORD_SERVER_LOG_WARNING(
          "can not be converted to a number. (%s=%s)",
          kAttributeNum, value.c_str());
      SENSCORD_SERVER_LOG_WARNING(" - use default value : %s=%" PRId32,
          kAttributeNum, *num);
    } else {
      // Negative values are not allowed
      if (*num < 0) {
        SENSCORD_SERVER_LOG_WARNING(
            "invalid value is used, use default value : %s=%" PRId32,
            kAttributeNum, *num);
        *num = kBufferNumDefault;
      }
    }
  } else {
    SENSCORD_SERVER_LOG_INFO(
        "%s attribute is not defined, use default value : %" PRId32,
        kAttributeNum, *num);
  }
}

/**
 * @brief Get the value of the num attribute of Frame.
 * @param[out] (format) Where to store the acquired config.
 */
void ConfigManager::ParseAttributeBufferingFormat(BufferingFormat* format) {
  std::string value;
  if (parser_->GetAttribute(kAttributeFormat, &value) == 0) {
    if (value == kValueBufferingFormatDiscard ||
        value == kValueBufferingFormatQueue) {
      *format = kBufferingFormatDiscard;
    } else if (value == kValueBufferingFormatOverwrite ||
        value == kValueBufferingFormatRing) {
      *format = kBufferingFormatOverwrite;
    } else {
      SENSCORD_SERVER_LOG_WARNING(
          "unknown attribute value (%s=%s), use default value : %d",
          kAttributeFormat, value.c_str(), *format);
    }
  } else {
    SENSCORD_SERVER_LOG_INFO(
        "%s attribute is not defined, use default value : %d",
        kAttributeFormat, *format);
  }
}

/**
 * @brief Parse default element and obtain it as Config.
 * @return Status object.
 */
Status ConfigManager::ParseDefaults() {
  Status ret;
  bool success = false;

  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;
  while (parser_->Parse(&node_type) == 0) {
    std::string element;
    if (node_type == osal::kOSXmlElementNode) {
      parser_->GetElement(&element);
      Status status = ParseDefaultElementNode(element);
      SENSCORD_STATUS_TRACE(status);
      if (!status.ok()) {
        ret = status;
        break;
      }
    } else if (node_type == osal::kOSXmlElementEnd) {
      parser_->GetElement(&element);
      if (element == kElementDefaults) {
        // If it is the end tag of defaults, it exits the loop.
        success = true;
        break;
      }
    }
  }

  if (!success && ret.ok()) {
    ret = SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseAborted, "parse defaults failed");
  }

  return ret;
}

/**
 * @brief Analyze the element nodes of the default.
 * @param[in] (element) Element name.
 * @return Status object.
 */
Status ConfigManager::ParseDefaultElementNode(const std::string& element) {
  if (element == kElementFrame) {
    OpenStreamSetting tmp_config = {};
    SetDefaultStreamConfig(&tmp_config);
    ParseFrame(&tmp_config.frame_buffering);
    default_stream_setting_ = tmp_config;
  }
  return Status::OK();
}

/**
 * @brief Parse address (or addressPrimary) attribute of config.
 * @param[out] (value) Address attribute value.
 * @return Status object.
 */
Status ConfigManager::ParseAttributeAddress(std::string* value) {
  if (parser_->GetAttribute(kAttributeAddress, value) != 0) {
    if (parser_->GetAttribute(kAttributeAddressPrimary, value) != 0) {
      // If 'address' and 'addressPrimary' are undefined.
      SENSCORD_SERVER_LOG_INFO(
          "%s and %s attributes are undefined.",
          kAttributeAddress, kAttributeAddressPrimary);
    }
  } else {
    if (parser_->GetAttribute(kAttributeAddressPrimary, value) == 0) {
      // If both 'address' and 'addressPrimary' are defined.
      return SENSCORD_STATUS_FAIL(kStatusBlockServer,
          Status::kCauseInvalidArgument,
          "Both %s and %s attributes are defined.",
          kAttributeAddress, kAttributeAddressPrimary);
    }
  }
  return Status::OK();
}

/**
 * @brief Parse addressSecondary attribute of config.
 * @param[out] (value) Address attribute value.
 */
void ConfigManager::ParseAttributeAddressSecondary(std::string* value) {
  if (parser_->GetAttribute(kAttributeAddressSecondary, value) != 0) {
    SENSCORD_SERVER_LOG_INFO(
        "%s attribute is not defined.",
        kAttributeAddressSecondary);
  }
}

/**
 * @brief Parse client attribute of config.
 * @param[out] (client_enabled) Whether the client is enabled.
 */
void ConfigManager::ParseAttributeClient(bool* client_enabled) {
  std::string value;
  if (parser_->GetAttribute(kAttributeClient, &value) == 0) {
    if (value == "on") {
      *client_enabled = true;
    } else if (value == "off") {
      *client_enabled = false;
    } else {
      SENSCORD_SERVER_LOG_WARNING(
          "unknown attribute value (%s=%s), use default value : %d",
          kAttributeClient, value.c_str(), *client_enabled);
    }
  } else {
    SENSCORD_SERVER_LOG_INFO(
        "%s attribute is not defined, use default value : %d",
        kAttributeClient, *client_enabled);
  }
}

/**
 * @brief Parse key attribute of config.
 * @param[out] (value) Key attribute value.
 * @return Status object.
 */
Status ConfigManager::ParseAttributeKey(std::string* value) {
  if (parser_->GetAttribute(kAttributeKey, value) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseNotFound,
        "parse attribute %s failed", kAttributeKey);
  }
  return Status::OK();
}

/**
 * @brief Print the contents of Config analyzed by ConfigManager.
 */
void ConfigManager::PrintConfig() const {
  osal::OSLockMutex(mutex_);
  SENSCORD_SERVER_LOG_DEBUG("*** server config ***");
  if (!current_config_path_.empty()) {
    SENSCORD_SERVER_LOG_DEBUG("- config_path       : %s",
        current_config_path_.c_str());
  }
  SENSCORD_SERVER_LOG_DEBUG("- address primary   : %s",
      server_config_.bind_config.c_str());
  SENSCORD_SERVER_LOG_DEBUG("- address secondary : %s",
      server_config_.bind_config2.c_str());
  SENSCORD_SERVER_LOG_DEBUG("- is_enabled_client : %d",
      server_config_.is_enabled_client);
  PrintStreamConfig();
  PrintDefaultConfig();
  osal::OSUnlockMutex(mutex_);
}

/**
 * @brief Print the definition of Stream.
 */
void ConfigManager::PrintStreamConfig() const {
  SENSCORD_SERVER_LOG_DEBUG(" [stream config]");
  if (!server_config_.streams.empty()) {
    std::map<std::string, OpenStreamSetting>::const_iterator it_stream;
    for (it_stream = server_config_.streams.begin();
        it_stream != server_config_.streams.end(); ++it_stream) {
      // Print stream definition name.
      SENSCORD_SERVER_LOG_DEBUG("  - stream : key=%s",
          it_stream->first.c_str());
      // Print frame config.
      PrintBuffering(it_stream->second.frame_buffering);
    }
  } else {
    SENSCORD_SERVER_LOG_DEBUG("    default setting is used.");
  }
}

/**
 * @brief Print the frame buffer config.
 * @param[in] (buffer_config) Frame buffer config.
 */
void ConfigManager::PrintBuffering(const FrameBuffering &buffer_config) const {
  SENSCORD_SERVER_LOG_DEBUG("    - frame : buffering=%" PRId32 ", "
      "num=%" PRId32 ", format=%" PRId32,
      buffer_config.buffering,
      buffer_config.num,
      buffer_config.format);
}

/**
 * @brief Print default parameter setting.
 */
void ConfigManager::PrintDefaultConfig() const {
  SENSCORD_SERVER_LOG_DEBUG(" [default config]");
  PrintBuffering(default_stream_setting_.frame_buffering);
}

}  // namespace server
}  // namespace senscord
