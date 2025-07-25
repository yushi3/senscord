/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
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
static const char* kElementListeners            = "listeners";
static const char* kElementListener             = "listener";
static const char* kAttributeConnection         = "connection";
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
static const char* kConnectionDefaultKey        = "";
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

    ret = VerifyConfig(server_config_);
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
 * @brief Get the listener setting list.
 * @param[out] (listeners) Listener setting list.
 * @return Status object.
 */
Status ConfigManager::GetListenerList(
    std::vector<ListenerSetting>* listeners) const {
  if (listeners == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  osal::OSLockMutex(mutex_);
  *listeners = server_config_.listeners;
  osal::OSUnlockMutex(mutex_);
  return Status::OK();
}

/**
 * @brief Search by stream key and return stream config.
 * @param[in] (stream_key) Search stream key.
 * @param[in] (connection_key) Search connection key.
 * @param[out] (config) Return stream config.
 * @return Status object.
 */
Status ConfigManager::GetStreamConfigByStreamKey(
    const std::string& stream_key, const std::string& connection_key,
    OpenStreamSetting* config) const {
  if (config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  SetDefaultStreamConfig(&config->frame_buffering);
  osal::OSLockMutex(mutex_);
  const StreamSetting* stream_setting = GetStreamSetting(stream_key);
  ConnectionBuffering::const_iterator buffering_itr;
  do {
    if (stream_setting != NULL) {
      buffering_itr = stream_setting->buffering.find(connection_key);
      if (buffering_itr != stream_setting->buffering.end()) {
        // user setting(stream, connection)
        config->frame_buffering = buffering_itr->second;
        break;
      }
      buffering_itr = stream_setting->buffering.find(kConnectionDefaultKey);
      if (buffering_itr != stream_setting->buffering.end()) {
        // user setting(stream)
        config->frame_buffering = buffering_itr->second;
        break;
      }
    }
    buffering_itr = default_stream_setting_.buffering.find(connection_key);
    if (buffering_itr != default_stream_setting_.buffering.end()) {
      // user seting(connection)
      config->frame_buffering = buffering_itr->second;
      break;
    }
    buffering_itr =
        default_stream_setting_.buffering.find(kConnectionDefaultKey);
    if (buffering_itr != default_stream_setting_.buffering.end()) {
      // default of system setting
      config->frame_buffering = buffering_itr->second;
    }
  } while (false);
  osal::OSUnlockMutex(mutex_);
  return Status::OK();
}

/**
 * @brief Search by stream key and return stream setting.
 * @param[in] (stream_key) Search stream key.
 * @return Return stream setting.
 */
const StreamSetting* ConfigManager::GetStreamSetting(
    const std::string& stream_key) const {
  std::vector<StreamSetting>::const_iterator itr =
      server_config_.streams.begin();
  std::vector<StreamSetting>::const_iterator end = server_config_.streams.end();
  const StreamSetting* result = NULL;
  for (; itr != end; ++itr) {
    const std::string& target = itr->stream_key;
    // Exact match.
    if (target == stream_key) {
      result = &(*itr);
      break;
    }
    // Backward match.
    if (result == NULL && IsBackwardMatch(target, stream_key)) {
      result = &(*itr);
    }
  }
  return result;
}

/**
 * @brief Backward matching or not.
 * @param[in] (target) Search target string.
 * @param[in] (suffix) The string of backward matches to search for.
 * @return True is matched backwards.
 */
bool ConfigManager::IsBackwardMatch(
    const std::string& target, const std::string& suffix) const {
  size_t target_size = target.size();
  size_t suffix_size = suffix.size();
  if (target_size >= suffix_size && target.find(
      suffix, target_size - suffix_size) != std::string::npos) {
    return true;
  }
  return false;
}

/**
 * @brief Verify whether it is a supported stream.
 * @param[in] (supported_streams) Supported streams list.
 */
void ConfigManager::VerifySupportedStream(
    const std::vector<StreamTypeInfo>& supported_streams) const {
  osal::OSLockMutex(mutex_);
  std::vector<StreamSetting>::const_iterator itr =
      server_config_.streams.begin();
  std::vector<StreamSetting>::const_iterator end =
      server_config_.streams.end();
  for (; itr != end; ++itr) {
    bool found = false;
    std::vector<StreamTypeInfo>::const_iterator stream_itr =
        supported_streams.begin();
    std::vector<StreamTypeInfo>::const_iterator stream_end =
        supported_streams.end();
    for (; stream_itr != stream_end; ++stream_itr) {
      if (IsBackwardMatch(stream_itr->key, itr->stream_key)) {
        found = true;
        break;
      }
    }
    if (!found) {
      SENSCORD_SERVER_LOG_WARNING("unsupported stream key: %s",
          itr->stream_key.c_str());
    }
  }
  osal::OSUnlockMutex(mutex_);
}

/**
 * @brief Perform parameter check of Config.
 * @param[in] (server_config) Server configuration.
 * @return Status object.
 */
Status ConfigManager::VerifyConfig(const ServerConfig& server_config) const {
  if (server_config.listeners.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        senscord::Status::kCauseAborted,
        "listener setting is empty.");
  }
  return Status::OK();
}

/**
 * @brief Clear the read Config information.
 */
void ConfigManager::ClearConfig() {
  current_config_path_.clear();
  server_config_.is_enabled_client = false;
  server_config_.listeners.clear();
  server_config_.streams.clear();
  FrameBuffering buffering = {};
  SetDefaultStreamConfig(&buffering);
  default_stream_setting_.buffering[kConnectionDefaultKey] = buffering;
}

/**
 * @brief Set default config to Config.
 * @param[out] (config) Where to set the default config.
 */
void ConfigManager::SetDefaultStreamConfig(FrameBuffering* buffering) const {
  buffering->buffering = kBufferingOn;
  buffering->num = 0;  // unlimited
  buffering->format = kBufferingFormatDefault;
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
  } else if (element == kElementListeners) {
    ret = ParseListeners();
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
  StreamSetting stream_setting = {};
  Status ret = ParseAttributeKey(&stream_setting.stream_key);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    return ret;
  }

  ret = ParseStreamChildNode(&stream_setting);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    return ret;
  }

  bool found = false;
  std::vector<StreamSetting>::iterator itr = server_config_.streams.begin();
  std::vector<StreamSetting>::iterator end = server_config_.streams.end();
  for (; itr != end; ++itr) {
    if (itr->stream_key == stream_setting.stream_key) {
      *itr = stream_setting;  // overwrite
      found = true;
      break;
    }
  }
  if (!found) {
    server_config_.streams.push_back(stream_setting);
  }

  return Status::OK();
}

/**
 * @brief Analyze children in stream and reflect on Config
 * @param[out] (stream_setting) Where to store the acquired config.
 * @return Status object.
 */
Status ConfigManager::ParseStreamChildNode(StreamSetting* stream_setting) {
  Status ret;
  bool success = false;

  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;
  while (parser_->Parse(&node_type) == 0) {
    std::string element;
    if (node_type == osal::kOSXmlElementNode) {
      parser_->GetElement(&element);
      Status status = ParseStreamElementNode(element, stream_setting);
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
 * @param[out] (stream_setting) Where to store the acquired config.
 * @return Status object.
 */
Status ConfigManager::ParseStreamElementNode(
    const std::string& element, StreamSetting* stream_setting) {
  if (element == kElementFrame) {
    ParseFrame(&stream_setting->buffering);
  } else {
    SENSCORD_SERVER_LOG_WARNING(
        "unknown element is ignored : element=%s", element.c_str());
  }

  return Status::OK();
}

/**
 * @brief Is the target included in listeners.
 * @param[in] (connection) target connection.
 * @return true is contains.
 */
bool ConfigManager::ContainsListener(const std::string& connection) {
  typedef std::vector<ListenerSetting> Listeners;
  Listeners::const_iterator itr = server_config_.listeners.begin();
  Listeners::const_iterator end = server_config_.listeners.end();
  for (; itr != end; ++itr) {
    if (itr->connection == connection) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Parse frame element and obtain it as Config.
 * @param[out] (connection_buffering)  Where to store the acquired config.
 */
void ConfigManager::ParseFrame(
    ConnectionBuffering* connection_buffering) {
  FrameBuffering frame_buffering = {};
  std::string connection;
  SetDefaultStreamConfig(&frame_buffering);
  ParseAttributeBuffering(&frame_buffering.buffering);
  ParseAttributeBufferingNum(&frame_buffering.num);
  ParseAttributeBufferingFormat(&frame_buffering.format);
  Status status = ParseAttributeConnection(&connection);
  if (!status.ok() || ContainsListener(connection)) {
    (*connection_buffering)[connection] = frame_buffering;   // Overwrite
  } else {
    SENSCORD_SERVER_LOG_WARNING(
        "unknown connection is ignored(%s)", connection.c_str());
  }
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
    ParseFrame(&default_stream_setting_.buffering);
  }
  return Status::OK();
}

/**
 * @brief Parse listeners element and obtain it as Config.
 * @return Status object.
 */
Status ConfigManager::ParseListeners() {
  Status ret;
  bool success = false;

  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;
  while (parser_->Parse(&node_type) == 0) {
    std::string element;
    if (node_type == osal::kOSXmlElementNode) {
      parser_->GetElement(&element);
      Status status = ParseListenersElementNode(element);
      SENSCORD_STATUS_TRACE(status);
      if (!status.ok()) {
        ret = status;
        break;
      }
    } else if (node_type == osal::kOSXmlElementEnd) {
      parser_->GetElement(&element);
      if (element == kElementListeners) {
        // If it is the end tag of listeners, it exits the loop.
        success = true;
        break;
      }
    }
  }

  if (!success && ret.ok()) {
    ret = SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseAborted, "parse listeners failed");
  }

  return ret;
}

/**
 * @brief Analyze the element nodes of the listeners.
 * @param[in] (element) Element name.
 * @return Status object.
 */
Status ConfigManager::ParseListenersElementNode(const std::string& element) {
  Status ret;

  if (element == kElementListener) {
    ret = ParseListener();
    SENSCORD_STATUS_TRACE(ret);
  } else {
    SENSCORD_SERVER_LOG_WARNING(
        "unknown element is ignored : element=%s", element.c_str());
  }

  return ret;
}

/**
 * @brief Parse listener element and obtain it as Config.
 * @return Status object.
 */
Status ConfigManager::ParseListener() {
  Status status;
  ListenerSetting listener_config = {};

  // attribute: connection (required)
  status = ParseAttributeConnection(&listener_config.connection);
  SENSCORD_STATUS_TRACE(status);
  if (!status.ok()) {
    return status;
  }

  // attribute: address or addressPrimary (required)
  status = ParseAttributeAddress(&listener_config.address_primary);
  SENSCORD_STATUS_TRACE(status);
  if (!status.ok()) {
    return status;
  }

  // attribute: addressSecondary (optional)
  ParseAttributeAddressSecondary(&listener_config.address_secondary);

  server_config_.listeners.push_back(listener_config);

  return Status::OK();
}

/**
 * @brief Parse connection attribute of config.
 * @param[out] (value) Connection attribute value.
 * @return Status object.
 */
Status ConfigManager::ParseAttributeConnection(std::string* value) {
  if (parser_->GetAttribute(kAttributeConnection, value) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        Status::kCauseNotFound,
        "%s attribute is not defined.", kAttributeConnection);
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
      return SENSCORD_STATUS_FAIL(kStatusBlockServer,
          Status::kCauseNotFound,
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
    SENSCORD_SERVER_LOG_DEBUG("- config_path : %s",
        current_config_path_.c_str());
  }
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
    std::vector<StreamSetting>::const_iterator it_stream;
    for (it_stream = server_config_.streams.begin();
        it_stream != server_config_.streams.end(); ++it_stream) {
      // Print stream definition name.
      SENSCORD_SERVER_LOG_DEBUG("  - stream : key=%s",
          it_stream->stream_key.c_str());
      ConnectionBuffering::const_iterator it_connection;
      for (it_connection = it_stream->buffering.begin();
           it_connection != it_stream->buffering.end();
           ++it_connection) {
        // Print frame config.
        PrintBuffering(it_connection->second, it_connection->first);
      }
    }
  } else {
    SENSCORD_SERVER_LOG_DEBUG("    default setting is used.");
  }
}

/**
 * @brief Print the frame buffer config.
 * @param[in] (buffer_config) Frame buffer config.
 * @param[in] (connection) Apply to connection.
 */
void ConfigManager::PrintBuffering(
    const FrameBuffering &buffer_config, const std::string& connection) const {
  SENSCORD_SERVER_LOG_DEBUG("    - frame : buffering=%" PRId32 ", "
      "num=%" PRId32 ", format=%" PRId32 ", connection=%s",
      buffer_config.buffering,
      buffer_config.num,
      buffer_config.format,
      connection.c_str());
}

/**
 * @brief Print default parameter setting.
 */
void ConfigManager::PrintDefaultConfig() const {
  SENSCORD_SERVER_LOG_DEBUG(" [default config]");
  for (ConnectionBuffering::const_iterator
      itr = default_stream_setting_.buffering.begin(),
      end = default_stream_setting_.buffering.end(); itr != end; ++itr) {
    PrintBuffering(itr->second, itr->first);
  }
}

}  // namespace server
}  // namespace senscord
