/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_COMPONENT_COMPONENT_CONFIG_MANAGER_H_
#define LIB_CORE_COMPONENT_COMPONENT_CONFIG_MANAGER_H_

#include "senscord/config.h"

#ifdef SENSCORD_STREAM_VERSION

#include <string>
#include <vector>
#include <map>

#include "senscord/osal.h"
#include "loader/class_dynamic_loader.h"
#include "senscord/develop/component.h"
#include "core/internal_types.h"
#include "util/mutex.h"

namespace senscord {

/**
 * @brief Component config manager.
 */
class ComponentConfigManager : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   */
  ComponentConfigManager();

  /**
   * @brief Destructor.
   */
  ~ComponentConfigManager();

  /**
   * @brief Read information on the compontnt`s configuration file.
   * @param[in] (name) Name of component to use.
   * @return Status object.
   */
  Status ReadConfig(const std::string& name);

  /**
   * @brief Acquire component config with component name as key.
   * @param[in] (name) Component name to get config.
   * @param[out] (config) Component Config storage pointer.
   * @return Status object.
   */
  Status GetConfig(const std::string& name, ComponentConfig **config);

 private:
  /**
   * @brief Read information on the compontnt`s configuration file.
   * @param[in] (name) Name of component to use.
   * @param[out] (config) The destination to store the analysis result of xml.
   * @return Status object.
   */
  Status Read(const std::string& name, ComponentConfig* config);

  /**
   * @brief Check if the target component config exists in the target path.
   * @param[in] (name) Component name to parse config.
   * @param[in] (path) Path to the directory where xml is located.
   * @return Check result (true: exists. / false: not found.)
   */
  bool ConfigExistsAtPath(const std::string& name, const std::string& path);

  /**
   * @brief Parse xml file.
   * @param[in] (file_path) Parse target xml path.
   * @param[out] (config) Where to store the acquired config.
   * @return Status object.
   */
  Status ParseXml(const std::string& file_path, ComponentConfig* config);

  /**
   * @brief Parse process below component tag.
   * @param[out] (config) Where to store the acquired config.
   * @return Status object.
   */
  Status ParseComponent(ComponentConfig* config);

  /**
   * @brief Parse component element and obtain it as Config.
   * @param[out] (config) Where to store the acquired config.
   * @return Status object.
   */
  Status ParseComponentElement(ComponentConfig* config);

  /**
   * @brief Parse number attribute element from config.
   * @param[in] (attribute) Target attribute.
   * @param[out] (value) Target attribute value.
   * @return Status object.
   */
  Status ParseAttributeNumber(const std::string& attribute, uint32_t* value);

  /**
   * @brief Parse string attribute element from config.
   * @param[in] (attribute) Target attribute.
   * @param[out] (value) Target attribute value.
   * @return Status object.
   */
  Status ParseAttributeString(const std::string& attribute, std::string* value);

  /**
   * @brief Parse process below linkage_version tag.
   * @param[out] (config) Where to store the acquired config.
   * @return Status object.
   */
  Status ParseLinkageVersions(ComponentConfig* config);

  /**
   * @brief Analyze the element nodes of the linkage_version.
   * @param[in] (element) Element name.
   * @return Status object.
   */
  Status ParseLinkageVersionsElementNode(
      const std::string& element, ComponentConfig* config);

  /**
   * @brief Parse version element and obtain it as config.
   * @param[out] (config) Where to store the acquired config.
   * @return Status object.
   */
  Status ParseVersion(ComponentConfig* config);

  /**
   * @brief Initialization of component config parameters
   * @param[out] (config) Component config to initialize.
   */
  void InitComponentConfig(ComponentConfig* config);

  // xml parser
  osal::OSXmlParser* parser_;

  // store component config
  std::map<std::string, ComponentConfig*> component_configs_;

  // File path used for log output.
  std::string xml_reading_file_;

  // config lock object
  util::Mutex* mutex_;
};

}  // namespace senscord

#else  // SENSCORD_STREAM_VERSION

#include "senscord/noncopyable.h"

namespace senscord {

class ComponentConfigManager : private util::Noncopyable {
};

}  // namespace senscord

#endif  // SENSCORD_STREAM_VERSION
#endif  // LIB_CORE_COMPONENT_COMPONENT_CONFIG_MANAGER_H_
