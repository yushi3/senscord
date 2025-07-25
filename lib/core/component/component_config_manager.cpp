/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "component/component_config_manager.h"

#include <inttypes.h>
#include <string>
#include <utility>

#include "util/senscord_utils.h"
#include "logger/logger.h"
#include "senscord/osal.h"
#include "senscord/environment.h"
#include "util/mutex.h"
#include "util/autolock.h"

namespace senscord {

// component config element/attribute
static const char* kElementComponent         = "component";
static const char* kAttributeName            = "name";
static const char* kAttributeMajor           = "major";
static const char* kAttributeMinor           = "minor";
static const char* kAttributePatch           = "patch";
static const char* kAttributeDescription     = "description";
static const char* kElementLinkageVersions   = "linkage_versions";
static const char* kElementVersion           = "version";
static const char* kExtensionXml             = ".xml";

/**
 * @brief Constructor.
 */
ComponentConfigManager::ComponentConfigManager() {
  mutex_ = new util::Mutex();
  parser_ = new osal::OSXmlParser();
}

/**
 * @brief Destructor.
 */
ComponentConfigManager::~ComponentConfigManager() {
  {
    util::AutoLock lock(mutex_);
    std::map<std::string, ComponentConfig*>::iterator it
        = component_configs_.begin();
    while (it != component_configs_.end()) {
      delete it->second;
      ++it;
    }
    component_configs_.clear();
  }
  delete parser_;
  parser_ = NULL;
  delete mutex_;
  mutex_ = NULL;
}

/**
 * @brief Read information on the component configuration file.
 * @param[in] (name) Name of component to use.
 * @return Status object.
 */
Status ComponentConfigManager::ReadConfig(const std::string& name) {
  util::AutoLock lock(mutex_);
  std::pair<std::map<std::string, ComponentConfig*>::iterator, bool> ret =
      component_configs_.insert(
          std::map<std::string, ComponentConfig*>::value_type(name, NULL));
  if (!ret.second) {
    SENSCORD_LOG_DEBUG("already read");
    return Status::OK();
  }
  ComponentConfig* config = new ComponentConfig;
  InitComponentConfig(config);
  Status status = Read(name, config);
  if (!status.ok()) {
    SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
    delete config;
    component_configs_.erase(ret.first);
    config = NULL;
  } else {
    ret.first->second = config;
  }
  return Status::OK();  // always return success
}

/**
 * @brief Acquire component config with component name as key.
 * @param[in] (name) Component name to get config.
 * @param[out] (config) Component Config storage pointer.
 * @return Status object.
 */
Status ComponentConfigManager::GetConfig(
    const std::string& name, ComponentConfig **config) {
  if (config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  util::AutoLock lock(mutex_);
  std::map<std::string, ComponentConfig*>::const_iterator itr =
      component_configs_.find(name);
  if (itr == component_configs_.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "component config not found : name=%s", name.c_str());
  }
  *config = itr->second;
  return Status::OK();
}

/**
 * @brief Read information on the compontnt`s configuration file.
 * @param[in] (name) Name of component to use.
 * @param[out] (config) The destination to store the analysis result of xml.
 * @return Status object.
 */
Status ComponentConfigManager::Read(
    const std::string& name, ComponentConfig* config) {
  if (config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  if (name.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "name is empty");
  }
  if (name == kComponentNamePublisher) {
    return Status::OK();  // publisher is skip
  }
  std::vector<std::string> env_paths;
  Status status = Environment::GetSensCordFilePath(&env_paths);
  if (!status.ok() || env_paths.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "failed to acquire environment : env=%s", kSensCordFilePathEnvStr);
  }

  std::string xml_name = name + kExtensionXml;
  std::vector<std::string>::const_iterator path_itr = env_paths.begin();
  std::vector<std::string>::const_iterator path_end = env_paths.end();
  for (; path_itr != path_end; ++path_itr) {
    if (!ConfigExistsAtPath(xml_name, *path_itr)) {
      continue;   // next path
    }
    std::string file_path = *path_itr + "/" + xml_name;
    status = ParseXml(file_path, config);
    // verify check
    if (status.ok() && (config->name != name)) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
          "component name does not match : file_path=%s, name=%s",
          file_path.c_str(), name.c_str());
    }
    return SENSCORD_STATUS_TRACE(status);
  }

  return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
      "component config acquisition failure : name=%s", name.c_str());
}

/**
 * @brief Check if the target component config exists in the target path.
 * @param[in] (name) Component name to parse config.
 * @param[in] (path) Path to the directory where xml is located.
 * @return Check result (true: exists. / false: not found.)
 */
bool ComponentConfigManager::ConfigExistsAtPath(
    const std::string& name, const std::string& path) {
  Status status;
  if (path.empty()) {
    return false;
  }
  std::vector<std::string> file_list;
  if (osal::OSGetRegularFileList(path, &file_list) != 0) {
    return false;
  }
  if (file_list.empty()) {
    return false;
  }
  std::vector<std::string>::const_iterator it;
  for (it = file_list.begin(); it != file_list.end(); ++it) {
    if (name == *it) {
      return true;  // found
    }
  }
  return false;   // not found
}

/**
 * @brief Parse xml file.
 * @param[in] (name) Component name to parse config.
 * @param[out] (config) Where to store the acquired config.
 * @return Status object.
 */
Status ComponentConfigManager::ParseXml(
    const std::string& file_path, ComponentConfig* config) {
  if (parser_->Open(file_path) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "xml file open failure : file_path=%s", file_path.c_str());
  }
  // Save the file path for log output.
  xml_reading_file_ = file_path;

  Status ret = SENSCORD_STATUS_FAIL(kStatusBlockCore,
      Status::kCauseAborted, "parse config failed");
  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;
  // Perform analysis to the end of the file.
  while (parser_->Parse(&node_type) == 0) {
    if (node_type == osal::kOSXmlElementNode) {
      std::string element;
      parser_->GetElement(&element);
      if (element == kElementComponent) {
        ret = ParseComponent(config);
        SENSCORD_STATUS_TRACE(ret);
        if (!ret.ok()) {
          break;
        }
      } else if (element == kElementLinkageVersions) {
        ret = ParseLinkageVersions(config);
        SENSCORD_STATUS_TRACE(ret);
        if (!ret.ok()) {
          break;
        }
      }
    }
  }
  parser_->Close();

  xml_reading_file_.clear();

  return ret;
}

/**
 * @brief Parse process below component tag.
 * @param[out] (config) Where to store the acquired config.
 * @return Status object.
 */
Status ComponentConfigManager::ParseComponent(ComponentConfig* config) {
  Status ret = ParseComponentElement(config);
  return SENSCORD_STATUS_TRACE(ret);
}

/**
 * @brief Parse component element and obtain it as Config.
 * @param[out] (config) Where to store the acquired config.
 * @return Status object.
 */
Status ComponentConfigManager::ParseComponentElement(ComponentConfig* config) {
  std::string component_name;
  if (parser_->GetAttribute(kAttributeName, &component_name) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "parse attribute %s failed : file_path=%s",
        kAttributeName, xml_reading_file_.c_str());
  }
  if (component_name.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "failed to get component name : file_path=%s",
        xml_reading_file_.c_str());
  }
  uint32_t major = 0;
  Status status = ParseAttributeNumber(kAttributeMajor, &major);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  uint32_t minor = 0;
  status = ParseAttributeNumber(kAttributeMinor, &minor);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  uint32_t patch = 0;
  status = ParseAttributeNumber(kAttributePatch, &patch);
  if (!status.ok()) {
    SENSCORD_LOG_WARNING(
    "unknown patch version, use default value : file_path=%s",
    xml_reading_file_.c_str());
  }
  std::string description;
  status = ParseAttributeString(kAttributeDescription, &description);
  if (!status.ok()) {
    SENSCORD_LOG_WARNING(
    "unknown description, use default value : file_path=%s",
    xml_reading_file_.c_str());
  }

  config->name = component_name;
  config->major_version = major;
  config->minor_version = minor;
  config->patch_version = patch;
  config->description = description;

  return Status::OK();
}

/**
 * @brief Parse process below linkage_version tag.
 * @param[out] (config) Where to store the acquired config.
 * @return Status object.
 */
Status ComponentConfigManager::ParseLinkageVersions(ComponentConfig* config) {
  Status ret = SENSCORD_STATUS_FAIL(kStatusBlockCore,
      Status::kCauseAborted, "parse streams failed");

  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;
  while (parser_->Parse(&node_type) == 0) {
    std::string element;
    if (node_type == osal::kOSXmlElementNode) {
      parser_->GetElement(&element);
      Status status = ParseLinkageVersionsElementNode(element, config);
      SENSCORD_STATUS_TRACE(status);
      if (!status.ok()) {
        ret = status;
        break;
      }
    } else if (node_type == osal::kOSXmlElementEnd) {
      parser_->GetElement(&element);
      if (element == kElementLinkageVersions) {
        // If it is the end tag of linkage, it exits the loop.
        ret = Status::OK();
        break;
      }
    }
  }

  return ret;
}

/**
 * @brief Analyze the element nodes of the linkage_version.
 * @param[in] (element) Element name.
 * @param[out] (config) Where to store the acquired config.
 * @return Status object.
 */
Status ComponentConfigManager::ParseLinkageVersionsElementNode(
    const std::string& element, ComponentConfig* config) {
  Status status;
  if (element == kElementVersion) {
    status = ParseVersion(config);
    SENSCORD_STATUS_TRACE(status);
  } else {
    SENSCORD_LOG_WARNING(
        "unknown element is ignored : element=%s", element.c_str());
  }
  return status;
}

/**
 * @brief Parse version element and obtain it as config.
 * @param[out] (config) Where to store the acquired config.
 * @return Status object.
 */
Status ComponentConfigManager::ParseVersion(
    ComponentConfig* config) {
  std::string name;
  if (parser_->GetAttribute(kAttributeName, &name) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "parse attribute %s failed : file_path=%s",
        kAttributeName, xml_reading_file_.c_str());
  }
  if (name.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "failed to get version name : file_path=%s",
        xml_reading_file_.c_str());
  }
  uint32_t major = 0;
  Status status = ParseAttributeNumber(kAttributeMajor, &major);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  uint32_t minor = 0;
  status = ParseAttributeNumber(kAttributeMinor, &minor);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  uint32_t patch = 0;
  status = ParseAttributeNumber(kAttributePatch, &patch);
  if (!status.ok()) {
    SENSCORD_LOG_WARNING(
    "unknown patch version, use default value : file_path=%s",
    xml_reading_file_.c_str());
  }
  std::string description;
  status = ParseAttributeString(kAttributeDescription, &description);
  if (!status.ok()) {
    SENSCORD_LOG_WARNING(
    "unknown description, use default value : file_path=%s",
    xml_reading_file_.c_str());
  }

  Version version;
  version.name = name;
  version.major = major;
  version.minor = minor;
  version.patch = patch;
  version.description = description;
  config->linkage_versions.push_back(version);

  return Status::OK();
}

/**
 * @brief Parse number attribute element from config.
 * @param[in] (attribute) Target attribute.
 * @param[out] (value) Target attribute value.
 * @return Status object.
 */
Status ComponentConfigManager::ParseAttributeNumber(
    const std::string& attribute, uint32_t* value) {
  std::string tmp;
  if (parser_->GetAttribute(attribute, &tmp) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "parse attribute %s failed : file_path=%s",
        attribute.c_str(), xml_reading_file_.c_str());
  }
  if (tmp.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "%s attribute not set : file_path=%s",
        attribute.c_str(), xml_reading_file_.c_str());
  }
  if (!util::StrToUint(tmp, value)) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "can not be converted to a number : %s=%s",
        attribute.c_str(), tmp.c_str());
  }
  return Status::OK();
}

/**
 * @brief Parse string attribute element from config.
 * @param[in] (attribute) Target attribute.
 * @param[out] (value) Target attribute value.
 * @return Status object.
 */
Status ComponentConfigManager::ParseAttributeString(
    const std::string& attribute, std::string* value) {
  if (parser_->GetAttribute(attribute, value) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "parse attribute %s failed : file_path=%s",
        attribute.c_str(), xml_reading_file_.c_str());
  }
  return Status::OK();
}

/**
 * @brief Initialization of component config parameters
 * @param[out] (config) Component config to initialize.
 */
void ComponentConfigManager::InitComponentConfig(ComponentConfig* config) {
  if (config != NULL) {
    config->name.clear();
    config->major_version = 0;
    config->minor_version = 0;
    config->patch_version = 0;
    config->description.clear();
    config->linkage_versions.clear();
  }
}

}   // namespace senscord
