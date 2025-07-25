/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core/config_manager.h"

#include <inttypes.h>
#include <utility>      // std::make_pair
#include <algorithm>    // std::sort

#include "logger/logger.h"
#include "util/mutex.h"
#include "util/autolock.h"
#include "util/senscord_utils.h"
#include "allocator/memory_manager.h"
#include "senscord/develop/common_types.h"
#include "senscord/configuration.h"

#ifdef SENSCORD_SERVER_SEARCH_SSDP
#include "searcher/ssdp_module.h"
#endif  // SENSCORD_SERVER_SEARCH_SSDP

#ifdef SENSCORD_SERVER_SEARCH_UCOM
#include "searcher/ucom_module.h"
#endif  // SENSCORD_SERVER_SEARCH_UCOM

namespace senscord {

// senscord config element/attribute
static const char* kElementSdk                  = "sdk";
static const char* kElementStreams              = "streams";
static const char* kElementStream               = "stream";
static const char* kElementAddress              = "address";
static const char* kElementFrame                = "frame";
static const char* kElementDefaults             = "defaults";
static const char* kElementInstances            = "instances";
static const char* kElementInstance             = "instance";
static const char* kElementAllocators           = "allocators";
static const char* kElementAllocator            = "allocator";
static const char* kElementArguments            = "arguments";
static const char* kElementArgument             = "argument";
static const char* kElementCore                 = "core";
static const char* kElementExtension            = "extension";
static const char* kAttributeKey                = "key";
static const char* kAttributeInstanceName       = "instanceName";
static const char* kAttributeType               = "type";
static const char* kAttributePort               = "port";
static const char* kAttributeBuffering          = "buffering";
static const char* kAttributeNum                = "num";
static const char* kAttributeFormat             = "format";
static const char* kAttributeName               = "name";
static const char* kAttributeComponent          = "component";
static const char* kAttributeValue              = "value";
static const char* kAttributeLibrary            = "library";
static const char* kValueBufferingFormatDiscard   = "discard";
static const char* kValueBufferingFormatOverwrite = "overwrite";
/** @deprecated "queue" has been replaced by "discard". */
static const char* kValueBufferingFormatQueue     = "queue";
/** @deprecated "ring" has been replaced by "overwrite". */
static const char* kValueBufferingFormatRing      = "ring";

#ifdef SENSCORD_LOG_ENABLED
static const char* kElementLog                  = "log";
static const char* kAttributeSeverity           = "severity";
static const char* kAttributeTag                = "tag";
static const char* kAttributeLevel              = "level";
static const char* kLogSeverityOff              = "off";
static const char* kLogSeverityError            = "error";
static const char* kLogSeverityWarning          = "warning";
static const char* kLogSeverityInfo             = "info";
static const char* kLogSeverityDebug            = "debug";
#endif  // SENSCORD_LOG_ENABLED

#ifdef SENSCORD_STREAM_VERSION
static const char* kElementVersion              = "version";
static const char* kAttributeMajor              = "major";
static const char* kAttributeMinor              = "minor";
static const char* kAttributePatch              = "patch";
static const char* kAttributeDescripton         = "description";
#endif  // SENSCORD_STREAM_VERSION

#ifdef SENSCORD_SERVER
static const char* kElementClient               = "client";
static const char* kAttributeEnabled            = "enabled";
#endif  // SENSCORD_SERVER
#ifdef SENSCORD_SERVER_SETTING
static const char* kElementSearches             = "searches";
static const char* kElementSearch               = "search";
static const char* kElementServers              = "servers";
static const char* kElementServer               = "server";
#endif  // SENSCORD_SERVER_SETTING
#ifdef SENSCORD_SERVER_SEARCH_SSDP
static const char* kValueSsdp                   = kSearchTypeSsdp;
static const char* kValueTcp                    = "tcp";
#endif  // SENSCORD_SERVER_SEARCH_SSDP
#ifdef SENSCORD_SERVER_SEARCH_UCOM
static const char* kValueUcom                   = kSearchTypeUcom;
#endif  // SENSCORD_SERVER_SEARCH_UCOM
#if defined(SENSCORD_SERVER_SEARCH_SSDP) || defined(SENSCORD_SERVER_SEARCH_UCOM)
static const char* kValueConnection             = "connection";
#endif  // SENSCORD_SERVER_SEARCH_SSDP || SENSCORD_SERVER_SEARCH_UCOM

/**
 * @brief Constructor.
 */
ConfigManager::ConfigManager()
    : core_config_(), default_config_(), read_() {
  mutex_ = new util::Mutex();
  parser_ = new util::XmlParser();
  server_config_manager_ = new ServerConfigManager();

  ClearConfig();
}

/**
  * @brief Destructor.
  */
ConfigManager::~ConfigManager() {
  ClearConfig();

  delete parser_;
  parser_ = NULL;

  delete mutex_;
  mutex_ = NULL;

  delete server_config_manager_;
  server_config_manager_ = NULL;
}

/**
 * @brief Copy the specified status.
 */
ConfigManager::ConfigManager(const ConfigManager& rhs) {
  mutex_ = new util::Mutex();
  parser_ = new util::XmlParser();
  server_config_manager_ = new ServerConfigManager();

  util::AutoLock lock(rhs.mutex_);
  core_config_ = rhs.core_config_;
  default_config_ = rhs.default_config_;
  read_ = rhs.read_;
  identification_ = rhs.identification_;
}

/**
 * @brief Copy the specified status.
 */
ConfigManager& ConfigManager::operator =(const ConfigManager& rhs) {
  CoreConfig core_config = {};
  DefaultConfigs default_config = {};
  bool read = false;
  std::string identification = "";

  {
    util::AutoLock lock(rhs.mutex_);
    core_config = rhs.core_config_;
    default_config = rhs.default_config_;
    read = rhs.read_;
    identification = rhs.identification_;
  }

  util::AutoLock lock(mutex_);
  core_config_ = core_config;
  default_config_ = default_config;
  read_ = read;
  identification_ = identification;
  return *this;
}

/**
  * @brief Get the list of Config read by ReadConfig function.
  * @return List of Config.
  */
const CoreConfig* ConfigManager::GetConfig() const {
  return &core_config_;
}

/**
 * @brief Search by stream key and return stream config.
 * @param[in] (stream_key) Search stream key.
 * @return Stream config.
 */
const StreamSetting* ConfigManager::GetStreamConfigByStreamKey(
    const std::string& stream_key) const {
  return GetStreamConfigBackwardMatch(&core_config_.stream_list, stream_key);
}

/**
 * @brief Search by component instance name and return config.
 * @param[in] (instance_name) Search component instance name.
 * @return Component config.
 */
const ComponentInstanceConfig* ConfigManager::GetComponentConfigByInstanceName(
    const std::string& instance_name) const {
  return GetComponentConfig(&core_config_.instance_list, instance_name);
}

/**
 * @brief Clear the read Config information.
 */
void ConfigManager::ClearConfig() {
  util::AutoLock lock(mutex_);

  core_config_.stream_list.clear();
  core_config_.instance_list.clear();
#ifdef SENSCORD_SERVER_SETTING
  core_config_.search_list.clear();
  core_config_.server_list.clear();
#endif  // SENSCORD_SERVER_SETTING

  // system default configuration
  default_config_.frame_buffering.buffering = kBufferingOn;
  default_config_.frame_buffering.num = 4;
  default_config_.frame_buffering.format = kBufferingFormatOverwrite;
#ifdef SENSCORD_SERVER
  default_config_.client_instance_name.clear();
#endif  // SENSCORD_SERVER

  identification_.clear();

  read_ = false;
}


/**
 * @brief Returns whether or not it has been loaded.
 * @return True if loaded.
 */
bool ConfigManager::IsLoaded() const {
  util::AutoLock lock(mutex_);
  return read_;
}

/**
 * @brief Read the specified Config file
 * @param[in] (filename) Path of config file.
 * @param[in] (identification) SensCord identification.
 * @return Status object.
 */
Status ConfigManager::ReadConfig(const std::string& filename) {
  util::AutoLock lock(mutex_);

  if (read_) {
    SENSCORD_LOG_DEBUG("already opened");
    return Status::OK();
  }

  Status ret = ParseConfig(filename);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    ClearConfig();
    return ret;
  }

  return Status::OK();
}

Status ConfigManager::FinalizeConfig(const std::string& identification) {
  util::AutoLock lock(mutex_);

  if (read_) {
    SENSCORD_LOG_DEBUG("already finalized");
    return Status::OK();
  }

  identification_ = identification;
  Status ret = AddIdentification(identification_);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    ClearConfig();
    return ret;
  }

  ApplyDefaultConfig();

#ifdef SENSCORD_SERVER
  ret = UpdateClientInstances();
  if (!ret.ok()) {
    ClearConfig();
    return ret;
  }
#endif  // SENSCORD_SERVER

  ret = VerifyConfig();
  SENSCORD_STATUS_TRACE(ret);
  if (ret.ok()) {
    read_ = true;
  } else {
    ClearConfig();
    return ret;
  }

  return Status::OK();
}

/**
 * @brief Set configuration.
 * @param[in] (core_config) Configuration to set.
 */
void ConfigManager::SetConfig(const CoreConfig& core_config) {
  util::AutoLock lock(mutex_);
  core_config_ = core_config;
  read_ = true;
}

/**
 * @brief Analysis process of Config file
 * @param[in] (filename) Path of config file.
 * @return Status object.
 */
Status ConfigManager::ParseConfig(const std::string& filename) {
  Status ret = parser_->Open(filename);
  if (!ret.ok()) {
    return SENSCORD_STATUS_TRACE(ret);
  }

  int32_t count = 0;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    if (element.GetDepth() != 0) {
      continue;
    }
    std::string name = element.GetName();
    if (name == kElementSdk) {
      ++count;
      ret = ParseSdk();
      SENSCORD_STATUS_TRACE(ret);
    } else {
      SENSCORD_LOG_WARNING(
          "unknown element is ignored : element=%s", name.c_str());
    }
  }
  if (count == 0) {
    ret = SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseAborted, "Failed to parse config.");
  }
  parser_->Close();

  return ret;
}

/**
 * @brief Parse sdk element.
 * @return Status object.
 */
Status ConfigManager::ParseSdk() {
  Status ret;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/***" (depth=1)
    int32_t depth = element.GetDepth();
    if (depth != 1) {
      if (depth < 1) {
        parser_->UndoElement();
        break;
      }
      continue;
    }
    std::string name = element.GetName();
    if (name == kElementStreams) {
      ret = ParseStreams();
      SENSCORD_STATUS_TRACE(ret);
    } else if (name == kElementInstances) {
      ret = ParseInstances();
      SENSCORD_STATUS_TRACE(ret);
#ifdef SENSCORD_SERVER_SETTING
    } else if (name == kElementSearches) {
      ret = ParseSearches();
      SENSCORD_STATUS_TRACE(ret);
    } else if (name == kElementServers) {
      ret = ParseServers();
      SENSCORD_STATUS_TRACE(ret);
#endif  // SENSCORD_SERVER_SETTING
    } else if (name == kElementCore) {
      ret = ParseCore();
      SENSCORD_STATUS_TRACE(ret);
#ifdef SENSCORD_STREAM_VERSION
    } else if (name == kElementVersion) {
      ret = ParseVersion();
      SENSCORD_STATUS_TRACE(ret);
#endif  // SENSCORD_STREAM_VERSION
    }
  }
  return ret;
}

/**
 * @brief Parse streams element.
 * @return Status object.
 */
Status ConfigManager::ParseStreams() {
  Status ret;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/streams/***" (depth=2)
    int32_t depth = element.GetDepth();
    if (depth != 2) {
      if (depth < 2) {
        parser_->UndoElement();
        break;
      }
      continue;
    }
    std::string name = element.GetName();
    if (name == kElementStream) {
      ret = ParseStream();
      SENSCORD_STATUS_TRACE(ret);
    } else if (name == kElementDefaults) {
      ret = ParseStreamsDefaults();
      SENSCORD_STATUS_TRACE(ret);
    }
  }
  return ret;
}

/**
 * @brief Parse stream element.
 * @return Status object.
 */
Status ConfigManager::ParseStream() {
  Status ret;
  StreamSetting stream_config;

  stream_config.stream_key = parser_->GetAttributeString(kAttributeKey);
  if (stream_config.stream_key.empty()) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseNotFound,
        "Failed to parse config. stream/%s attribute", kAttributeKey);
  }

  int32_t count = 0;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/streams/stream/***" (depth=3)
    int32_t depth = element.GetDepth();
    if (depth != 3) {
      if (depth < 3) {
        parser_->UndoElement();
        break;
      }
      continue;
    }
    std::string name = element.GetName();
    if (name == kElementAddress) {
      ++count;
      ret = ParseAddress(&stream_config);
      SENSCORD_STATUS_TRACE(ret);
    } else if (name == kElementFrame) {
      ParseFrame(&stream_config);
    } else if (name == kElementArguments) {
      ret = ParseArguments(element.GetXPath(), &stream_config.arguments);
      SENSCORD_STATUS_TRACE(ret);
    } else if (name == kElementExtension) {
      ret = ParseExtension(&stream_config);
#ifdef SENSCORD_SERVER
    } else if (name == kElementClient) {
      ret = ParseClient(&stream_config);
      SENSCORD_STATUS_TRACE(ret);
#endif  // SENSCORD_SERVER
    }
  }

  if (count == 0) {
    ret = SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseAborted,
        "Failed to parse config. stream element");
  }
  if (ret.ok()) {
    core_config_.stream_list.push_back(stream_config);
  }

  return ret;
}

/**
 * @brief Parse address element.
 * @param[out] (config) Where to store the acquired config.
 * @return Status object.
 */
Status ConfigManager::ParseAddress(StreamSetting* config) {
  Status ret;
  {
    config->address.instance_name =
        parser_->GetAttributeString(kAttributeInstanceName);
    if (config->address.instance_name.empty()) {
      ret = SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseNotFound,
          "Failed to parse config. address/%s attribute",
          kAttributeInstanceName);
    }
  }
  if (ret.ok()) {
    config->address.port_type = parser_->GetAttributeString(kAttributeType);
    if (config->address.port_type.empty()) {
      ret = SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseNotFound,
          "Failed to parse config. address/%s attribute", kAttributeType);
    }
  }
  if (ret.ok()) {
    std::string port = parser_->GetAttributeString(kAttributePort);
    if (!util::StrToInt(port, &config->address.port_id) ||
        (config->address.port_id < 0)) {
      ret = SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidArgument,
          "Failed to parse config. address/%s attribute (%s)",
          kAttributePort, port.c_str());
    }
  }
  return ret;
}

/**
 * @brief Parse frame element.
 * @param[out] (config)  Where to store the acquired config.
 */
void ConfigManager::ParseFrame(StreamSetting* config) {
  ParseAttributeBuffering(config);
  ParseAttributeBufferingNum(config);
  ParseAttributeBufferingFormat(config);
}

/**
 * @brief Get the value of the Buffer attribute of Frame.
 * @param[out] (config) Where to store the acquired config.
 */
void ConfigManager::ParseAttributeBuffering(StreamSetting* config) {
  config->frame_buffering.buffering = kBufferingDefault;
  config->frame_buffering.num = kBufferNumDefault;
  config->frame_buffering.format = kBufferingFormatDefault;

  std::string buffering = parser_->GetAttributeString(kAttributeBuffering);
  if (!buffering.empty()) {
    if (buffering == "on") {
      config->frame_buffering.buffering = kBufferingOn;
    } else if (buffering == "off") {
      config->frame_buffering.buffering = kBufferingOff;
    } else {
      SENSCORD_LOG_WARNING(
          "unknown attribute value, use default value : %s=%s",
          kAttributeBuffering, buffering.c_str());
    }
  } else {
    SENSCORD_LOG_INFO(
        "%s attribute is not defined, use default", kAttributeBuffering);
  }
}

/**
 * @brief Get the value of the num attribute of Frame.
 * @param[out] (config) Where to store the acquired config.
 */
void ConfigManager::ParseAttributeBufferingNum(StreamSetting* config) {
  // Set default value.
  config->frame_buffering.num = kBufferNumDefault;

  std::string num = parser_->GetAttributeString(kAttributeNum);
  if (!num.empty()) {
    if (!util::StrToInt(num, &config->frame_buffering.num)) {
      SENSCORD_LOG_WARNING(
          "can not be converted to a number, use default value : %s=%s",
          kAttributeNum, num.c_str());

      SENSCORD_LOG_WARNING(" - use the default value");
    } else {
      // Negative values are not allowed
      if (config->frame_buffering.num < 0) {
        SENSCORD_LOG_WARNING(
            "invalid value is used, use default value : %s=%" PRId32,
            kAttributeNum, config->frame_buffering.num);
        config->frame_buffering.num = kBufferNumDefault;
      }
    }
  } else {
    SENSCORD_LOG_INFO(
        "%s attribute is not defined, use default value", kAttributeNum);
  }
}

/**
 * @brief Get the value of the format attribute of Frame.
 * @param[out] (config) Where to store the acquired config.
 */
void ConfigManager::ParseAttributeBufferingFormat(StreamSetting* config) {
  // Set default value.
  config->frame_buffering.format =  kBufferingFormatDefault;

  std::string format = parser_->GetAttributeString(kAttributeFormat);
  if (!format.empty()) {
    if (format == kValueBufferingFormatDiscard ||
        format == kValueBufferingFormatQueue) {
      config->frame_buffering.format = kBufferingFormatDiscard;
    } else if (format == kValueBufferingFormatOverwrite ||
        format == kValueBufferingFormatRing) {
      config->frame_buffering.format = kBufferingFormatOverwrite;
    } else {
      SENSCORD_LOG_WARNING(
          "unknown attribute value, use default value : %s=%s",
          kAttributeFormat, format.c_str());
    }
  } else {
    SENSCORD_LOG_INFO(
        "%s attribute is not defined, use default", kAttributeFormat);
  }
}

/**
 * @brief Parse extension element.
 * @param[out] (config) Where to store the acquired config.
 */
Status ConfigManager::ParseExtension(StreamSetting* config) {
  ExtensionSetting extension = {};
  extension.library_name = parser_->GetAttributeString(kAttributeLibrary);

  Status ret;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/streams/stream/extension/***" (depth=4)
    int32_t depth = element.GetDepth();
    if (depth != 4) {
      if (depth < 4) {
        parser_->UndoElement();
        break;
      }
      continue;
    }
    std::string name = element.GetName();
    if (name == kElementAllocators) {
      ret = ParseAllocators(element.GetXPath(), &extension.allocators);
      SENSCORD_STATUS_TRACE(ret);
    } else if (name == kElementArguments) {
      ret = ParseArguments(element.GetXPath(), &extension.arguments);
      SENSCORD_STATUS_TRACE(ret);
    }
  }
  if (ret.ok()) {
    config->extensions.push_back(extension);
  }
  return ret;
}

#ifdef SENSCORD_SERVER
/**
 * @brief Parse client element.
 * @param[out] (config) Where to store the acquired config.
 * @return Status object.
 */
Status ConfigManager::ParseClient(StreamSetting* config) {
  // parse enabled attribute.
  std::string enabled = parser_->GetAttributeString(kAttributeEnabled, "on");
  if (enabled == "off") {
    // ignore instanceName attribute.
    config->client_instance_name = "";
  } else {
    // parse instanceName attribute.
    config->client_instance_name =
        parser_->GetAttributeString(kAttributeInstanceName);
    if (config->client_instance_name.empty()) {
      if (enabled == "on") {
        return SENSCORD_STATUS_FAIL(
            kStatusBlockCore, Status::kCauseNotFound,
            "Failed to parse config. client/%s attribute",
            kAttributeInstanceName);
      } else {
        SENSCORD_LOG_WARNING(
            "Failed to parse config. client %s=`%s` is invalid."
            " behaves as `off`", kAttributeEnabled, enabled.c_str());
      }
    }
  }
  // this flag indicates that a client tag is specified.
  config->client_specified = true;
  return Status::OK();
}
#endif  // SENSCORD_SERVER

/**
 * @brief Parse instances element.
 * @return Status object.
 */
Status ConfigManager::ParseInstances() {
  Status ret;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/instances/***" (depth=2)
    int32_t depth = element.GetDepth();
    if (depth != 2) {
      if (depth < 2) {
        parser_->UndoElement();
        break;
      }
      continue;
    }
    std::string name = element.GetName();
    if (name == kElementInstance) {
      ret = ParseInstance();
      SENSCORD_STATUS_TRACE(ret);
    } else if (name == kElementDefaults) {
      ret = ParseInstancesDefaults();
      SENSCORD_STATUS_TRACE(ret);
    }
  }
  return ret;
}

/**
 * @brief Parse instance element.
 * @return Status object.
 */
Status ConfigManager::ParseInstance() {
  Status ret;
  ComponentInstanceConfig instance_config = {};

  instance_config.instance_name = parser_->GetAttributeString(kAttributeName);
  if (instance_config.instance_name.empty()) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseNotFound,
        "Failed to parse config. instance/%s attribute", kAttributeName);
  }

  instance_config.component_name =
      parser_->GetAttributeString(kAttributeComponent);
  if (instance_config.component_name.empty()) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseNotFound,
        "Failed to parse config. instance/%s attribute", kAttributeComponent);
  }

  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/instances/instance/***" (depth=3)
    int32_t depth = element.GetDepth();
    if (depth != 3) {
      if (depth < 3) {
        parser_->UndoElement();
        break;
      }
      continue;
    }
    std::string name = element.GetName();
    if (name == kElementArguments) {
      ret = ParseArguments(element.GetXPath(), &instance_config.arguments);
      SENSCORD_STATUS_TRACE(ret);
    } else if (name == kElementAllocators) {
      ret = ParseAllocators(
          element.GetXPath(), &instance_config.allocator_key_list);
      SENSCORD_STATUS_TRACE(ret);
    }
  }

  if (ret.ok()) {
    core_config_.instance_list.push_back(instance_config);
  }

  return ret;
}

/**
 * @brief Parse allocators element.
 * @param[in] (parent_xpath) Parent XPath.
 * @param[out] (instance_config) Where to store the acquired config.
 * @return Status object.
 */
Status ConfigManager::ParseAllocators(
    const std::string& parent_xpath, AllocatorMap* allocators) {
  Status ret;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/instances/instance/allocators/***" (depth=4)
    // xpath="/sdk/streams/stream/extension/allocators/***" (depth=5)
    std::string xpath = element.GetXPath();
    if (xpath.find(parent_xpath) == std::string::npos) {
      parser_->UndoElement();
      break;
    }
    std::string name = element.GetName();
    if (name == kElementAllocator) {
      ret = ParseAllocatorKey(allocators);
      SENSCORD_STATUS_TRACE(ret);
    }
  }
  return ret;
}

/**
 * @brief Parse allocator element.
 * @param[out] (instance_config) Where to store the acquired config.
 * @return Status object.
 */
Status ConfigManager::ParseAllocatorKey(
    AllocatorMap* allocators) {
  Status ret;
  std::string key;
  {
    ret = parser_->GetAttribute(kAttributeKey, &key);
    SENSCORD_STATUS_TRACE(ret);
  }
  if (ret.ok()) {
    if (key == senscord::kDefaultAllocatorKey) {
      key = "";  // default allocator
    }
    std::string name = parser_->GetAttributeString(
        kAttributeName, kAllocatorNameDefault);
    allocators->insert(std::make_pair(name, key));
  }
  return ret;
}

/**
 * @brief Parse arguments element.
 * @param[in] (parent_xpath) Parent XPath.
 * @param[out] (argument_map) Where to store the acquired config.
 * @return Status object.
 */
Status ConfigManager::ParseArguments(
    const std::string& parent_xpath, ArgumentMap* argument_map) {
  Status ret;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/streams/stream/arguments/***" (depth=4)
    // xpath="/sdk/streams/stream/extension/arguments/***" (depth=5)
    // xpath="/sdk/instances/instance/arguments/***" (depth=4)
    // xpath="/sdk/searches/search/arguments/***" (depth=4)
    // xpath="/sdk/servers/server/arguments/***" (depth=4)
    std::string xpath = element.GetXPath();
    if (xpath.find(parent_xpath) == std::string::npos) {
      parser_->UndoElement();
      break;
    }
    std::string name = element.GetName();
    if (name == kElementArgument) {
      ret = ParseArgument(argument_map);
      SENSCORD_STATUS_TRACE(ret);
    }
  }
  return ret;
}

/**
 * @brief Parse argument element.
 * @param[out] (argument_map) Where to store the acquired config.
 * @return Status object.
 */
Status ConfigManager::ParseArgument(ArgumentMap* argument_map) {
  Status ret;
  std::string name;
  std::string value;
  {
    ret = parser_->GetAttribute(kAttributeName, &name);
    SENSCORD_STATUS_TRACE(ret);
  }
  if (ret.ok()) {
    ret = parser_->GetAttribute(kAttributeValue, &value);
    SENSCORD_STATUS_TRACE(ret);
  }
  if (ret.ok()) {
    (*argument_map)[name] = value;
  }
  return ret;
}

/**
 * @brief Parse instances/defaults element.
 * @return Status object.
 */
Status ConfigManager::ParseInstancesDefaults() {
  Status ret;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/instances/defaults/***" (depth=3)
    int32_t depth = element.GetDepth();
    if (depth != 3) {
      if (depth < 3) {
        parser_->UndoElement();
        break;
      }
      continue;
    }
#ifdef SENSCORD_LOG_ENABLED
    std::string name = element.GetName();
    if (name == kElementLog) {
      if (core_config_.tag_logger_list.count(util::kLoggerTagDefault) == 0) {
        LogLevel instances_log_severity = kLogInfo;
        ParseLog(&instances_log_severity);
        core_config_.tag_logger_list.insert(std::make_pair(
            util::kLoggerTagDefault, instances_log_severity));
      }
    }
#endif  // SENSCORD_LOG_ENABLED
  }
  return ret;
}

/**
 * @brief Parse streams/default element.
 * @return Status object.
 */
Status ConfigManager::ParseStreamsDefaults() {
  Status ret;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/streams/defaults/***" (depth=3)
    int32_t depth = element.GetDepth();
    if (depth != 3) {
      if (depth < 3) {
        parser_->UndoElement();
        break;
      }
      continue;
    }
    std::string name = element.GetName();
    if (name == kElementFrame) {
      StreamSetting stream_config;
      ParseFrame(&stream_config);
      default_config_.frame_buffering = stream_config.frame_buffering;
#ifdef SENSCORD_SERVER
    } else if (name == kElementClient) {
      StreamSetting stream_config;
      ret = ParseClient(&stream_config);
      SENSCORD_STATUS_TRACE(ret);
      if (ret.ok()) {
        default_config_.client_instance_name =
            stream_config.client_instance_name;
      }
#endif  // SENSCORD_SERVER
    }
  }
  return ret;
}

#ifdef SENSCORD_SERVER_SETTING
/**
 * @brief Parse searches element.
 * @return Status object.
 */
Status ConfigManager::ParseSearches() {
  Status ret;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/searches/***" (depth=2)
    int32_t depth = element.GetDepth();
    if (depth != 2) {
      if (depth < 2) {
        parser_->UndoElement();
        break;
      }
      continue;
    }
    std::string name = element.GetName();
    if (name == kElementSearch) {
      ret = ParseSearch();
      SENSCORD_STATUS_TRACE(ret);
    }
  }
  return ret;
}

/**
 * @brief Parse search element.
 * @return Status object.
 */
Status ConfigManager::ParseSearch() {
  SearchSetting search_config = {};

  search_config.name = parser_->GetAttributeString(kAttributeName);
  if (search_config.name.empty()) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "Failed to parse config. search/%s attribute", kAttributeName);
  }

  std::string value = parser_->GetAttributeString(kAttributeValue);
  if (value == "true") {
    search_config.is_enabled = true;
  } else if (value == "false") {
    search_config.is_enabled = false;
  } else {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "Failed to parse config. search/%s attribute (%s)",
        kAttributeValue, value.c_str());
  }

  Status ret;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/searches/search/***" (depth=3)
    int32_t depth = element.GetDepth();
    if (depth != 3) {
      if (depth < 3) {
        parser_->UndoElement();
        break;
      }
      continue;
    }
    std::string name = element.GetName();
    if (name == kElementArguments) {
      ret = ParseArguments(element.GetXPath(), &search_config.arguments);
      SENSCORD_STATUS_TRACE(ret);
    }
  }

  if (ret.ok()) {
    core_config_.search_list.push_back(search_config);
  }

  return ret;
}

/**
 * @brief Parse servers element.
 * @return Status object.
 */
Status ConfigManager::ParseServers() {
  Status ret;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/servers/***" (depth=2)
    int32_t depth = element.GetDepth();
    if (depth != 2) {
      if (depth < 2) {
        parser_->UndoElement();
        break;
      }
      continue;
    }
    std::string name = element.GetName();
    if (name == kElementServer) {
      ret = ParseServer();
      SENSCORD_STATUS_TRACE(ret);
    }
  }
  return ret;
}

/**
 * @brief Parse server element.
 * @return Status object.
 */
Status ConfigManager::ParseServer() {
  ServerSetting server_config = {};

  Status ret;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/servers/server/***" (depth=3)
    int32_t depth = element.GetDepth();
    if (depth != 3) {
      if (depth < 3) {
        parser_->UndoElement();
        break;
      }
      continue;
    }
    std::string name = element.GetName();
    if (name == kElementArguments) {
      ret = ParseArguments(element.GetXPath(), &server_config.arguments);
      SENSCORD_STATUS_TRACE(ret);
    }
  }

  if (ret.ok()) {
    core_config_.server_list.push_back(server_config);
  }

  return ret;
}
#endif  // SENSCORD_SERVER_SETTING

/**
 * @brief Parse core element.
 * @return Status object.
 */
Status ConfigManager::ParseCore() {
  Status ret;
  util::XmlElement element = {};
  while (ret.ok() && parser_->NextElement(&element)) {
    // xpath="/sdk/core/***" (depth=2)
    int32_t depth = element.GetDepth();
    if (depth != 2) {
      if (depth < 2) {
        parser_->UndoElement();
        break;
      }
      continue;
    }
#ifdef SENSCORD_LOG_ENABLED
    std::string name = element.GetName();
    if (name == kElementLog) {
      ret = ParseCoreLog(&core_config_);
      SENSCORD_STATUS_TRACE(ret);
    }
#endif  // SENSCORD_LOG_ENABLED
  }
  return ret;
}

#ifdef SENSCORD_LOG_ENABLED
/**
 * @brief Check the log element attribute value.
 * @param[in] (attr_value) level or severity attribute value.
 * @param[out] (output) Log level.
 * @return true if successful.
 */
bool ConfigManager::CheckLogLevel(
    const std::string& attr_value, LogLevel* output) {
  if (attr_value == kLogSeverityOff) {
    *output = util::Logger::kLogOff;
  } else if (attr_value == kLogSeverityError) {
    *output = util::Logger::kLogError;
  } else if (attr_value == kLogSeverityWarning) {
    *output = util::Logger::kLogWarning;
  } else if (attr_value == kLogSeverityInfo) {
    *output = util::Logger::kLogInfo;
  } else if (attr_value == kLogSeverityDebug) {
    *output = util::Logger::kLogDebug;
  } else {
    return false;
  }
  return true;
}

/**
 * @brief Parse log element.
 * @param[out] (log_level) Log level.
 * @return Status object.
 */
Status ConfigManager::ParseLog(LogLevel* log_level) {
  std::string level = parser_->GetAttributeString(kAttributeLevel);
  if (!CheckLogLevel(level, log_level)) {
    std::string severity = parser_->GetAttributeString(kAttributeSeverity);
    if (!CheckLogLevel(severity, log_level)) {
      SENSCORD_LOG_WARNING(
          "Failed to parse config. log attribute (%s=`%s`, %s=`%s`),"
          " use default value",
          kAttributeLevel, level.c_str(),
          kAttributeSeverity, severity.c_str());
    }
  }
  return Status::OK();
}

/**
 * @brief Parse core/log element.
 * @param[in] (core_config) Core configuration.
 * @return Status object.
 */
Status ConfigManager::ParseCoreLog(CoreConfig* core_config) {
  std::string tag;
  Status ret = parser_->GetAttribute(kAttributeTag, &tag);
  if (ret.ok() && tag.empty()) {
    return Status::OK();
  }

  if (!ret.ok()) {
    tag = util::kLoggerTagCore;
  }

  LogLevel log_severity = kLogInfo;
  ParseLog(&log_severity);

  core_config->tag_logger_list[tag] = log_severity;
  return Status::OK();
}
#endif  // SENSCORD_LOG_ENABLED

#ifdef SENSCORD_STREAM_VERSION
/**
 * @brief Parse version element.
 * @return Status object.
 */
Status ConfigManager::ParseVersion() {
  Status ret;
  Version project_version = {};
  {
    ret = parser_->GetAttribute(kAttributeName, &project_version.name);
    SENSCORD_STATUS_TRACE(ret);
  }
  if (ret.ok()) {
    std::string tmp = parser_->GetAttributeString(kAttributeMajor);
    if (!util::StrToUint(tmp, &project_version.major)) {
      ret = SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidArgument,
          "Failed to parse config. version/%s attribute (%s)",
          kAttributeMajor, tmp.c_str());
    }
  }
  if (ret.ok()) {
    std::string tmp = parser_->GetAttributeString(kAttributeMinor);
    if (!util::StrToUint(tmp, &project_version.minor)) {
      ret = SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidArgument,
          "Failed to parse config. version/%s attribute (%s)",
          kAttributeMinor, tmp.c_str());
    }
  }
  if (ret.ok()) {
    std::string tmp = parser_->GetAttributeString(kAttributePatch);
    if (!util::StrToUint(tmp, &project_version.patch)) {
      SENSCORD_LOG_WARNING(
          "Failed to parse config. version/%s attribute (%s),"
          " use default value", kAttributePatch, tmp.c_str());
      project_version.patch = 0;
    }
  }
  if (ret.ok()) {
    project_version.description =
        parser_->GetAttributeString(kAttributeDescripton);
    core_config_.project_version = project_version;
  }
  return ret;
}
#endif  // SENSCORD_STREAM_VERSION

/**
 * @brief Apply default value to config.
 */
void ConfigManager::ApplyDefaultConfig() {
  // frame buffer
  std::vector<StreamSetting>::iterator itr;
  for (itr = core_config_.stream_list.begin();
       itr != core_config_.stream_list.end(); ++itr) {
    ApplyDefaultFrameBufferConfig(
        &(itr->frame_buffering), default_config_.frame_buffering);
  }
}

/**
 * @brief Apply default value to frame buffer config.
 * @param[in,out] (config) Applies to default config.
 * @param[in] (default_config) Default config.
 */
void ConfigManager::ApplyDefaultFrameBufferConfig(
    FrameBuffering* config, const FrameBuffering& default_config) {
  if (config->buffering == kBufferingDefault) {
    config->buffering = default_config.buffering;
  }
  if (config->buffering > kBufferingOff) {
    if (config->num == kBufferNumDefault) {
      config->num = default_config.num;
    }
    if (config->format == kBufferingFormatDefault) {
      config->format = default_config.format;
    }
  }
}

/**
 * @brief Set default config to Config.
 * @return Status object.
 */
Status ConfigManager::SetDefaultConfig() {
#ifdef SENSCORD_SERVER_SEARCH_SSDP
  SearchSetting search_config_ssdp = {};
  search_config_ssdp.name = "ssdp";
  search_config_ssdp.is_enabled = true;
  core_config_.search_list.push_back(search_config_ssdp);
#endif  // SENSCORD_SERVER_SEARCH_SSDP
#ifdef SENSCORD_SERVER_SEARCH_UCOM
  SearchSetting search_config_ucom = {};
  search_config_ucom.name = "ucom";
  search_config_ucom.is_enabled = true;
  core_config_.search_list.push_back(search_config_ucom);
#endif  // SENSCORD_SERVER_SEARCH_UCOM
  return Status::OK();
}

/**
 * @brief Verify stream config.
 * @param[in,out] (config) Verify target stream config.
 * @return Status object.
 */
Status ConfigManager::VerifyStreamConfig(StreamSetting* config) {
  if (config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  Status ret = VerifyFrameBufferConfig(config);
  return SENSCORD_STATUS_TRACE(ret);
}

/**
 * @brief Verify frame buffer config.
 * @param[in,out] (config) Verify target stream config.
 * @return Status object.
 */
Status ConfigManager::VerifyFrameBufferConfig(StreamSetting* config) {
  // check range and set correction value
  if ((config->frame_buffering.buffering < kBufferingUseConfig) ||
      (config->frame_buffering.buffering > kBufferingOn)) {
    SENSCORD_LOG_WARNING("unknown buffering value, use default value");
    config->frame_buffering.buffering = kBufferingDefault;
  }
  if (config->frame_buffering.num < kBufferNumUseConfig) {
    SENSCORD_LOG_WARNING("num is an invalid value, use default value");
    config->frame_buffering.num = kBufferNumDefault;
  }
  if ((config->frame_buffering.format < kBufferingFormatUseConfig) ||
      (config->frame_buffering.format > kBufferingFormatOverwrite)) {
    SENSCORD_LOG_WARNING("unknown format value, use default value");
    config->frame_buffering.format = kBufferingFormatDefault;
  }

  // apply basic setting from xml
  const StreamSetting* stream_config =
      GetStreamConfigByStreamKey(config->stream_key);
  if (stream_config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound,
        "unable to get config from Stream key : key=%s",
        config->stream_key.c_str());
  }
  if (config->frame_buffering.buffering == kBufferingUseConfig) {
    config->frame_buffering.buffering =
        stream_config->frame_buffering.buffering;
  }
  if (config->frame_buffering.num == kBufferNumUseConfig) {
    config->frame_buffering.num = stream_config->frame_buffering.num;
  }
  if (config->frame_buffering.format == kBufferingFormatUseConfig) {
    config->frame_buffering.format = stream_config->frame_buffering.format;
  }
  ApplyDefaultFrameBufferConfig(
      &(config->frame_buffering), stream_config->frame_buffering);

  // apply default setting from xml
  ApplyDefaultFrameBufferConfig(
      &(config->frame_buffering), default_config_.frame_buffering);
  if (config->frame_buffering.buffering < kBufferingOn) {
    config->frame_buffering.num = kBufferNumDefault;
    config->frame_buffering.format = kBufferingFormatDefault;
  }
  return Status::OK();
}

#ifdef SENSCORD_SERVER
/**
 * @brief Count the same client instance name.
 * @param[in] (client_instance_name) Client instance name.
 * @return Number of count.
 */
uint32_t ConfigManager::GetCountSameClientInstance(
  const std::string& client_instance_name) const {
  // check all streams
  uint32_t num_count = 0;
  for (std::vector<StreamSetting>::const_iterator
      itr = core_config_.stream_list.begin(),
      end = core_config_.stream_list.end(); itr != end; ++itr) {
    if (client_instance_name == itr->client_instance_name) {
      ++num_count;
    }
  }
  return num_count;
}

/**
 * @brief Update client informations.
 * @return Status object.
 */
Status ConfigManager::UpdateClientInstances() {
  // map for port number of each client instance.
  typedef std::map<std::string, int32_t> ClientPortMap;
  ClientPortMap client_port_map;

  // check all streams
  StreamVector::iterator itr_stream = core_config_.stream_list.begin();
  StreamVector::iterator end_steram = core_config_.stream_list.end();

  for (; itr_stream != end_steram; ++itr_stream) {
    StreamSetting* stream = &(*itr_stream);
    if (!stream->client_specified) {
      // uses the client specified by <defaults>.
      stream->client_instance_name = default_config_.client_instance_name;
    }
    if (stream->client_instance_name.empty()) {
      // client is not used.
      stream->radical_address = stream->address;
      continue;
    }

    // using client
    int32_t port_id = 0;
    const std::string& client_instance_name = stream->client_instance_name;

    // serach client port num
    ClientPortMap::iterator itr_client = client_port_map.find(
        client_instance_name);
    if (itr_client != client_port_map.end()) {
      // found
      port_id = (++itr_client->second);
    } else {
      // not found
      client_port_map.insert(
          std::make_pair(client_instance_name, port_id));
    }
    stream->radical_address = stream->address;
    stream->address.instance_name = client_instance_name;
    stream->address.port_type = kPortTypeClient;
    stream->address.port_id = port_id;

    // add allocator keys to client instance.
    Status status = AddAllocatorKey(
        client_instance_name, stream->radical_address);
    SENSCORD_STATUS_TRACE(status);
    if (!status.ok()) {
      return status;
    }
  }

  // set client port num
  InstanceVector::iterator itr_instance = core_config_.instance_list.begin();
  InstanceVector::iterator end_instance = core_config_.instance_list.end();
  for (; itr_instance != end_instance; ++itr_instance) {
    uint32_t port_num = 0;
    port_num = GetCountSameClientInstance(itr_instance->instance_name);
    if (port_num == 0) {
      // not match any client instance.
      continue;
    }
    std::ostringstream argument_value;
    argument_value << port_num;
    itr_instance->arguments[kArgumentNamePortNum] =
        argument_value.str();
  }
  return Status::OK();
}

/**
 * @brief Add allocator keys to client component instance.
 * @param[in] (dest_instance_name) Client instance name to insert.
 * @param[in] (src_address) Port of source.
 * @return Status object.
 */
Status ConfigManager::AddAllocatorKey(
    const std::string& dest_instance_name,
    const StreamAddress& src_address) {
  InstanceVector::const_iterator itr = core_config_.instance_list.begin();
  InstanceVector::const_iterator end = core_config_.instance_list.end();
  for (; itr != end; ++itr) {
    if (itr->instance_name == src_address.instance_name) {
      Status status = AddAllocatorKey(
          dest_instance_name, itr->allocator_key_list);
      return SENSCORD_STATUS_TRACE(status);
    }
  }
  return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
      "unknown instance name: %s", src_address.instance_name.c_str());
}

/**
 * @brief Add allocator keys to client component instance.
 * @param[in] (dest_instance_name) Client instance name to insert.
 * @param[in] (allocator_keys) List of allocator key.
 * @return Status object.
 */
Status ConfigManager::AddAllocatorKey(
    const std::string& dest_instance_name,
    const std::map<std::string, std::string>& allocator_keys) {
  InstanceVector::iterator itr = core_config_.instance_list.begin();
  InstanceVector::iterator end = core_config_.instance_list.end();
  for (; itr != end; ++itr) {
    if (itr->instance_name == dest_instance_name) {
      std::map<std::string, std::string>* list = &(itr->allocator_key_list);

      // insert to list
      if (allocator_keys.empty()) {
        // if the instance has no allocator, insert default key.
        list->insert(
            std::make_pair(kAllocatorNameDefault, kAllocatorDefaultKey));
      } else {
        // add all keys though it is duplicated.
        // name is as allocator key, because names are not used by client.
        std::map<std::string, std::string>::const_iterator keys_itr =
            allocator_keys.begin();
        std::map<std::string, std::string>::const_iterator keys_end =
            allocator_keys.end();
        for (; keys_itr != keys_end; ++keys_itr) {
          list->insert(
              std::make_pair(keys_itr->second, keys_itr->second));
        }
      }
      return Status::OK();
    }
  }
  return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
      "unknown client instance name: %s", dest_instance_name.c_str());
}
#endif  // SENSCORD_SERVER

#ifdef SENSCORD_STREAM_VERSION
/**
 * @brief Get the unique instance name list.
 * @param[out] (list) Return unique instance name list.
 * @return Status object.
 */
Status ConfigManager::GetInstanceNameList(std::vector<std::string>* list) {
  if (list == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  typedef std::vector<StreamSetting>::const_iterator stream_it;
  stream_it sitr = core_config_.stream_list.begin();
  stream_it send = core_config_.stream_list.end();
  for (; sitr != send; ++sitr) {
    typedef std::vector<ComponentInstanceConfig>::const_iterator instance_it;
    instance_it iitr = core_config_.instance_list.begin();
    instance_it iend = core_config_.instance_list.end();
    for (; iitr != iend; ++iitr) {
      if (sitr->address.instance_name == iitr->instance_name) {
        if (std::find(list->begin(), list->end(),
            iitr->instance_name) == list->end()) {
          list->push_back(iitr->instance_name);
          break;
        }
      }
    }
  }
  return Status::OK();
}
#endif  // SENSCORD_STREAM_VERSION

/**
 * @brief Check whether an instance corresponding to the instance name exists.
 * @param[in] (use_instance_name) Name of the interface to be used
 * @return Status object.
 */
Status ConfigManager::CheckExistInstance(
    const std::string& use_instance_name) {
  for (std::vector<ComponentInstanceConfig>::const_iterator
      itr = core_config_.instance_list.begin(),
      end = core_config_.instance_list.end(); itr != end; ++itr) {
    if (itr->instance_name == use_instance_name) {
      return Status::OK();
    }
  }

  /* can't find */
  return SENSCORD_STATUS_FAIL(kStatusBlockCore,
      Status::kCauseNotFound,
      "instance name not exist : %s", use_instance_name.c_str());
}

/**
 * @brief Check the validity of stream's config.
 * @return Status object.
 */
Status ConfigManager::VerifyStream() {
  StreamVector::iterator it_stream;
  for (it_stream = core_config_.stream_list.begin();
      it_stream != core_config_.stream_list.end(); ++it_stream) {
     Status ret = CheckExistInstance(it_stream->address.instance_name);
     SENSCORD_STATUS_TRACE(ret);
     if (!ret.ok()) {
       return ret;
     }
  }
  return Status::OK();
}

/**
 * @brief Check the validity of config.
 * @return Status object.
 */
Status ConfigManager::VerifyConfig() {
  Status ret = VerifyStream();
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    return ret;
  }

  return Status::OK();
}

/**
 * @brief Add identification.
 * @param[in] (identification) SensCord identification.
 * @return Status object.
 */
Status ConfigManager::AddIdentification(const std::string& identification) {
  if (identification.empty()) {
    return Status::OK();
  }

  const std::string id_and_delimiter =
      identification + kSensCordIdentificationDelimiter;

  for (std::vector<StreamSetting>::iterator
           itr = core_config_.stream_list.begin(),
           end = core_config_.stream_list.end();
       itr != end; ++itr) {
    ConcatenateIdString(&(itr->stream_key), id_and_delimiter);
    ConcatenateIdString(&(itr->address.instance_name), id_and_delimiter);
#ifdef SENSCORD_SERVER
    ConcatenateIdString(&(itr->radical_address.instance_name),
                        id_and_delimiter);
    ConcatenateIdString(&(itr->client_instance_name), id_and_delimiter);
#endif  // SENSCORD_SERVER
    itr->identification = identification;
  }

  for (std::vector<ComponentInstanceConfig>::iterator
           itr = core_config_.instance_list.begin(),
           end = core_config_.instance_list.end();
       itr != end; ++itr) {
    ConcatenateIdString(&(itr->instance_name), id_and_delimiter);
  }

#ifdef SENSCORD_SERVER
  ConcatenateIdString(&(default_config_.client_instance_name),
                      id_and_delimiter);
#endif  // SENSCORD_SERVER

  return Status::OK();
}

/**
 * @brief Concatenate Id string.
 * @param[out] (target) Target string.
 * @param[in] (identification) SensCord identification.
 * @return Status object.
 */
Status ConfigManager::ConcatenateIdString(std::string* target,
                                          const std::string& identification) {
  if (!target->empty()) {
    *target = identification + *target;
  }
  return Status::OK();
}

#ifdef SENSCORD_SERVER_SETTING
/**
 * @brief Get server information.
 * @param[in] (server_setting) Server setting.
 * @param[out] (type) Connection type.
 * @param[out] (address) Destination address.
 * @return Status object.
 */
Status ConfigManager::GetServerInfo(
    const ServerSetting& server_setting,
    std::string* type,
    std::string* address) {
#if defined(SENSCORD_SERVER_SEARCH_SSDP) || defined(SENSCORD_SERVER_SEARCH_UCOM)
  typedef std::map<std::string, std::string>::const_iterator Iter;

  Iter type_itr = server_setting.arguments.find(kValueConnection);
  Iter addr_itr = server_setting.arguments.find(kElementAddress);
  Iter end = server_setting.arguments.end();

  if ((type_itr == end) || (addr_itr == end)) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseInvalidArgument,
                                "invalid parameter");
  }

  *type = type_itr->second;
  *address = addr_itr->second;
#endif
  return Status::OK();
}

/**
 * @brief Check server config.
 * @param[in] (server_setting) Server setting.
 * @param[in] (server_list) Server setting list.
 * @return True if there is a server setting in the list
 */
bool ConfigManager::CheckServerConfig(
    const ServerSetting& server_setting,
    std::vector<ServerSetting>* server_list) {
  std::string target_type;
  std::string target_address;

  // check target setting
  if (!GetServerInfo(server_setting, &target_type, &target_address).ok()) {
    return false;
  }

  // check server setting in the list
  std::string type;
  std::string address;
  for (std::vector<ServerSetting>::iterator
           itr = server_list->begin(),
           end = server_list->end();
       itr != end; ++itr) {
    if (GetServerInfo(*itr, &type, &address).ok()) {
      if ((target_type == type) && (target_address == address)) {
        return true;
      }
    }
  }
  return false;
}

/**
 * @brief Read server config.
 * @return Status object.
 */
Status ConfigManager::ReadServerConfig() {
  if (server_config_manager_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
                                "server_config_manager_ is null");
  }

  std::vector<SearchSetting>::iterator search_itr =
      core_config_.search_list.begin();
  for (; search_itr != core_config_.search_list.end(); ++search_itr) {
#ifdef SENSCORD_SERVER_SEARCH_SSDP
    if ((search_itr->name == kValueSsdp) && search_itr->is_enabled) {
      senscord::SsdpModule ssdp;
      ssdp.Init(search_itr->arguments);
      const std::vector<senscord::DeviceAddress> deviceAddress = ssdp.Search();

      std::vector<ServerSetting> server_list;
      std::vector<senscord::DeviceAddress>::const_iterator addr_itr =
          deviceAddress.begin();
      for (; addr_itr != deviceAddress.end(); ++addr_itr) {
        ServerSetting serverSetting;
        serverSetting.arguments = addr_itr->GetMap();
        serverSetting.arguments.insert(search_itr->arguments.begin(),
                                       search_itr->arguments.end());
        server_list.push_back(serverSetting);
      }
      std::vector<senscord::ServerSetting>::iterator server_itr =
          core_config_.server_list.begin();
      while (server_itr != core_config_.server_list.end()) {
        if (server_itr->arguments[kValueConnection] == kValueTcp) {
          if (CheckServerConfig(*server_itr, &server_list)) {
            server_itr = core_config_.server_list.erase(server_itr);
          } else {
            ++server_itr;
          }
        } else {
          ++server_itr;
        }
      }
      core_config_.server_list.insert(core_config_.server_list.end(),
                                      server_list.begin(), server_list.end());
    }
#endif  // SENSCORD_SERVER_SEARCH_SSDP

#ifdef SENSCORD_SERVER_SEARCH_UCOM
    if ((search_itr->name == kValueUcom) && search_itr->is_enabled) {
      senscord::UcomModule ucom;
      ucom.Init(search_itr->arguments);
      const std::vector<senscord::DeviceAddress> deviceAddress = ucom.Search();

      std::vector<ServerSetting> server_list;
      std::vector<senscord::DeviceAddress>::const_iterator addr_itr =
          deviceAddress.begin();
      for (; addr_itr != deviceAddress.end(); ++addr_itr) {
        ServerSetting serverSetting;
        serverSetting.arguments = addr_itr->GetMap();
        serverSetting.arguments.insert(search_itr->arguments.begin(),
                                       search_itr->arguments.end());
        server_list.push_back(serverSetting);
      }

      std::vector<senscord::ServerSetting>::iterator server_itr =
          core_config_.server_list.begin();
      while (server_itr != core_config_.server_list.end()) {
        if (server_itr->arguments[kValueConnection] == kValueUcom) {
          if (CheckServerConfig(*server_itr, &server_list)) {
            server_itr = core_config_.server_list.erase(server_itr);
          } else {
            ++server_itr;
          }
        } else {
          ++server_itr;
        }
      }
      core_config_.server_list.insert(core_config_.server_list.end(),
                                      server_list.begin(), server_list.end());
    }
#endif  // SENSCORD_SERVER_SEARCH_UCOM
  }

  return server_config_manager_->GetServerConfig(&core_config_,
                                                 identification_);
}

/**
 * @brief Get server config.
 * @param[out] (server_config) Server configuration.
 * @return Status object.
 */
Status ConfigManager::GetServerConfig(ServerConfig* server_config) {
  if (server_config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
                                "server_config is null");
  }

  for (std::vector<StreamSetting>::const_iterator
           itr = core_config_.stream_list.begin(),
           end = core_config_.stream_list.end();
       itr != end; ++itr) {
    ServerStreamSetting server_stream_setting = {
        itr->stream_key,
        {itr->address.instance_name, itr->address.port_type,
         itr->address.port_id},
        {itr->radical_address.instance_name, itr->radical_address.port_type,
         itr->radical_address.port_id},
        itr->frame_buffering,
        itr->client_instance_name,
        itr->client_specified,
        itr->identification};
    server_config->stream_list.push_back(server_stream_setting);
  }

  for (std::vector<ComponentInstanceConfig>::const_iterator
           itr = core_config_.instance_list.begin(),
           end = core_config_.instance_list.end();
       itr != end; ++itr) {
    ServerComponentInstanceConfig server_component_instance_config = {
        itr->instance_name, itr->component_name, itr->allocator_key_list};
    server_config->instance_list.push_back(server_component_instance_config);
  }

  return Status::OK();
}
#endif  // SENSCORD_SERVER_SETTING

/**
 * @brief Print the contents of Config analyzed by ConfigManager.
 */
void ConfigManager::PrintConfig() {
#ifdef SENSCORD_LOG_ENABLED
  if (!read_) {
    SENSCORD_LOG_DEBUG("Config file has not been loaded yet");
    return;
  }
  SENSCORD_LOG_DEBUG("*** senscord config ***");

#ifdef SENSCORD_STREAM_VERSION
  PrintProjectVersion();
#endif  // SENSCORD_STREAM_VERSION
  PrintStreamConfig();
  PrintDefaultConfig();
  PrintInstanceConfig();
#ifdef SENSCORD_SERVER_SETTING
  PrintSearchConfig();
  PrintServerConfig();
#endif  // SENSCORD_SERVER_SETTING
  PrintLogSeverity();
#endif  // SENSCORD_LOG_ENABLED
}

/**
 * @brief Print the definition of Stream.
 */
void ConfigManager::PrintStreamConfig() {
  SENSCORD_LOG_DEBUG(" [stream config]");

  StreamVector::iterator it_stream;
  for (it_stream = core_config_.stream_list.begin();
      it_stream != core_config_.stream_list.end(); ++it_stream) {
    // Print stream definition name.
    SENSCORD_LOG_DEBUG("  - stream : key=%s",
        it_stream->stream_key.c_str());
    // Print definition of interface used by stream.
    SENSCORD_LOG_DEBUG("    - address : "
        "instanceName=%s, type=%s, port=%" PRId32,
        it_stream->address.instance_name.c_str(),
        it_stream->address.port_type.c_str(),
        it_stream->address.port_id);
#ifdef SENSCORD_SERVER
    SENSCORD_LOG_DEBUG("    - radical_address : "
        "instanceName=%s, type=%s, port=%" PRId32,
        it_stream->radical_address.instance_name.c_str(),
        it_stream->radical_address.port_type.c_str(),
        it_stream->radical_address.port_id);
    SENSCORD_LOG_DEBUG("    - client_specified : %s",
        it_stream->client_specified ? "true" : "false");
#endif  // SENSCORD_SERVER
    // Print frame config.
    PrintBuffering(it_stream->frame_buffering);
    // Printing arguments passed to the port.
    PrintPortArgument(it_stream->arguments);
  }
}

/**
 * @brief Print the frame buffer config.
 * @param[in] (buffer_config) Frame buffer config.
 */
void ConfigManager::PrintBuffering(const FrameBuffering &buffer_config) {
  SENSCORD_LOG_DEBUG("    - frame : buffering=%" PRId32 ", "
      "num=%" PRId32 ", format=%" PRId32,
      buffer_config.buffering,
      buffer_config.num,
      buffer_config.format);
}

/**
 * @brief Print port arguments setting.
 * @param[in] (arguments) Arguments to pass to the port.
 */
void ConfigManager::PrintPortArgument(
      const std::map<std::string, std::string>& arguments) {
  std::map<std::string, std::string>::const_iterator it_argument;

  for (it_argument = arguments.begin();
      it_argument != arguments.end(); ++it_argument) {
    SENSCORD_LOG_DEBUG("    - argument : key=%s, value=%s",
        it_argument->first.c_str(),
        it_argument->second.c_str());
  }
}
/**
 * @brief Print default parameter setting.
 */
void ConfigManager::PrintDefaultConfig() {
  SENSCORD_LOG_DEBUG(" [default config]");
  // Default parameters of frame print.
  PrintBuffering(default_config_.frame_buffering);

#ifdef SENSCORD_SERVER
  if (!default_config_.client_instance_name.empty()) {
    SENSCORD_LOG_DEBUG("    - client_instance_name : %s",
        default_config_.client_instance_name.c_str());
  }
#endif  // SENSCORD_SERVER
}

/**
 * @brief Print instance definition.
 */
void ConfigManager::PrintInstanceConfig() {
  SENSCORD_LOG_DEBUG(" [interface config]");
  // Print interface definition
  InstanceVector::iterator it_interface;
  for (it_interface = core_config_.instance_list.begin();
      it_interface != core_config_.instance_list.end(); ++it_interface) {
    SENSCORD_LOG_DEBUG("  - instance : name=%s, component=%s",
        it_interface->instance_name.c_str(),
        it_interface->component_name.c_str());
    // Print arguments passed to the interface.
    PrintComponentArguments(it_interface->arguments);
    // Print the key of the allocator to use.
    PrintAllocator(it_interface->allocator_key_list);
  }
}

/**
 * @brief Print component arguments setting.
 * @param[in] (arguments) Arguments to pass to the component.
 */
void ConfigManager::PrintComponentArguments(
    const std::map<std::string, std::string>& arguments) {
  std::map<std::string, std::string>::const_iterator it_argument;
  for (it_argument = arguments.begin();
      it_argument != arguments.end(); ++it_argument) {
    SENSCORD_LOG_DEBUG("    - argument : name=%s, value=%s",
        it_argument->first.c_str(),
        it_argument->second.c_str());
  }
}

/**
 * @brief Print allocator keys.
 * @param[in] (allocator_key_list) List of key of the allocator.
 */
void ConfigManager::PrintAllocator(
    const std::map<std::string, std::string>& allocator_key_list) {
  std::map<std::string, std::string>::const_iterator it_allocator;
  for (it_allocator = allocator_key_list.begin();
      it_allocator != allocator_key_list.end(); ++it_allocator) {
    SENSCORD_LOG_DEBUG("    - allocator : name=%s, key=%s",
        it_allocator->first.c_str(), it_allocator->second.c_str());
  }
}

#ifdef SENSCORD_SERVER_SETTING
/**
 * @brief Print search setting.
 */
void ConfigManager::PrintSearchConfig() {
  SENSCORD_LOG_DEBUG(" [search config]");

  SearchVector::iterator it_search;
  for (it_search = core_config_.search_list.begin();
       it_search != core_config_.search_list.end(); ++it_search) {
    SENSCORD_LOG_DEBUG("  - search : name=%s, value=%s",
                       it_search->name.c_str(),
                       it_search->is_enabled ? "true" : "false");

    std::map<std::string, std::string>::const_iterator it_argument;
    for (it_argument = it_search->arguments.begin();
         it_argument != it_search->arguments.end(); ++it_argument) {
      SENSCORD_LOG_DEBUG("    - argument : name=%s, value=%s",
                         it_argument->first.c_str(),
                         it_argument->second.c_str());
    }
  }
}

/**
 * @brief Print server setting.
 */
void ConfigManager::PrintServerConfig() {
  SENSCORD_LOG_DEBUG(" [server config]");

  ServerVector::iterator it_server;
  for (it_server = core_config_.server_list.begin();
       it_server != core_config_.server_list.end(); ++it_server) {
    SENSCORD_LOG_DEBUG("  - server");

    std::map<std::string, std::string>::const_iterator it_argument;
    for (it_argument = it_server->arguments.begin();
         it_argument != it_server->arguments.end(); ++it_argument) {
      SENSCORD_LOG_DEBUG("    - argument : name=%s, value=%s",
                         it_argument->first.c_str(),
                         it_argument->second.c_str());
    }
  }
}
#endif  // SENSCORD_SERVER_SETTING

#ifdef SENSCORD_STREAM_VERSION
/**
 * @brief Print component arguments setting.
 * @param[in] (arguments) Arguments to pass to the component.
 */
void ConfigManager::PrintProjectVersion() {
  SENSCORD_LOG_DEBUG(" [project version]");
    SENSCORD_LOG_DEBUG("    - name : %s",
        core_config_.project_version.name.c_str());
    SENSCORD_LOG_DEBUG("    - major : %" PRId32,
        core_config_.project_version.major);
    SENSCORD_LOG_DEBUG("    - minor : %" PRId32,
        core_config_.project_version.minor);
    SENSCORD_LOG_DEBUG("    - patch : %" PRId32,
        core_config_.project_version.patch);
    SENSCORD_LOG_DEBUG("    - description : %s",
        core_config_.project_version.description.c_str());
}
#endif  // SENSCORD_STREAM_VERSION

#ifdef SENSCORD_LOG_ENABLED
/**
 * @brief Print log severities
 */
void ConfigManager::PrintLogSeverity() {
  SENSCORD_LOG_DEBUG(" [log level]");

  for (std::map<std::string, util::Logger::LogSeverity>::const_iterator
      itr = core_config_.tag_logger_list.begin(),
      end = core_config_.tag_logger_list.end();
      itr != end; ++itr) {
    SENSCORD_LOG_DEBUG(" - %s : %s", itr->first.c_str(),
        GetLogSeverityLabel(itr->second).c_str());
  }
}

/**
 * @brief Get severity label from LogSeverity.
 * @param[in] (severity) Type of severity.
 * @return severity text
 */
std::string ConfigManager::GetLogSeverityLabel(
    util::Logger::LogSeverity severity) {
  switch (severity) {
    case util::Logger::kLogOff:
      return kLogSeverityOff;
      break;

    case util::Logger::kLogDebug:
      return kLogSeverityDebug;
      break;

    case util::Logger::kLogInfo:
      return kLogSeverityInfo;
      break;

    case util::Logger::kLogWarning:
      return kLogSeverityWarning;
      break;

    case util::Logger::kLogError:
      return kLogSeverityError;
      break;

    default:
      return "unknown";
      break;
  }
}
#endif  // SENSCORD_LOG_ENABLED

}    // namespace senscord
