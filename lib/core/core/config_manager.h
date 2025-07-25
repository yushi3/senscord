/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CORE_CONFIG_MANAGER_H_
#define LIB_CORE_CORE_CONFIG_MANAGER_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <utility>  // pair

#include "senscord/osal.h"
#include "senscord/status.h"
#include "core/internal_types.h"
#include "core/server_config_manager.h"
#include "util/mutex.h"
#include "util/xml_parser.h"

namespace senscord {

/**
 * @brief Config manager class
 */
class ConfigManager {
 public:
  /**
   * @brief Constructor.
   */
  ConfigManager();

  /**
   * @brief Destructor.
   */
  ~ConfigManager();

  /**
   * @brief Copy the specified Config.
   */
  ConfigManager(const ConfigManager& rhs);
  ConfigManager& operator =(const ConfigManager& rhs);

  /**
   * @brief Returns whether or not it has been loaded.
   * @return True if loaded.
   */
  bool IsLoaded() const;

  /**
   * @brief Read the specified Config file
   * @param[in] (filename) Path of config file.
   * @return Status object.
   */
  Status ReadConfig(const std::string& filename);

  /**
   * @brief Finalize Config to make SensCord runnable.
   * @param[in] (identification) SensCord identification.
   * @return Status object.
   */
  Status FinalizeConfig(const std::string& identification);

  /**
   * @brief Set configuration.
   * @param[in] (core_config) Configuration to set.
   */
  void SetConfig(const CoreConfig& core_config);

  /**
   * @brief Clear the read Config information.
   */
  void ClearConfig();

  /**
   * @brief Get the list of Config read by ReadConfig function.
   * @return List of Config.
   */
  const CoreConfig* GetConfig() const;

  /**
   * @brief Search by stream key and return stream config.
   * @param[in] (stream_key) Search stream key.
   * @return Stream config.
   */
  const StreamSetting* GetStreamConfigByStreamKey(
      const std::string& stream_key) const;

  /**
   * @brief Search by component instance name and return config.
   * @param[in] (instance_name) Search component instance name.
   * @return Component config.
   */
  const ComponentInstanceConfig* GetComponentConfigByInstanceName(
      const std::string& instance_name) const;

#ifdef SENSCORD_STREAM_VERSION
  /**
   * @brief Get the unique instance name list.
   * @param[out] (list) Return unique instance name list.
   * @return Status object.
   */
  Status GetInstanceNameList(std::vector<std::string>* list);
#endif  // SENSCORD_STREAM_VERSION

  /**
   * @brief Verify stream config.
   * @param[in,out] (config) Verify target stream config.
   * @return Status object.
   */
  Status VerifyStreamConfig(StreamSetting* config);

#ifdef SENSCORD_SERVER_SETTING
  /**
   * @brief Get server information.
   * @param[in] (server_setting) Server setting.
   * @param[out] (type) Connection type.
   * @param[out] (address) Destination address.
   * @return Status object.
   */
  Status GetServerInfo(
      const ServerSetting& server_setting,
      std::string* type,
      std::string* address);

  /**
   * @brief Check server config.
   * @param[in] (server_setting) Server setting.
   * @param[in] (server_list) Server setting list.
   * @return True if there is a server setting in the list 
   */
  bool CheckServerConfig(
      const ServerSetting& server_setting,
      std::vector<ServerSetting>* server_list);

  /**
   * @brief Read server config.
   * @return Status object.
   */
  Status ReadServerConfig();

  /**
   * @brief Get server config.
   * @param[out] (server_config) Server configuration.
   * @return Status object.
   */
  Status GetServerConfig(ServerConfig* server_config);
#endif  // SENSCORD_SERVER_SETTING

  /**
   * @brief Print the contents of Config analyzed by ConfigManager.
   */
  void PrintConfig();

  /**
   * @brief Set default config to Config.
   * @return Status object.
   */
  Status SetDefaultConfig();

 private:
  typedef std::vector<StreamSetting> StreamVector;
  typedef std::vector<ComponentInstanceConfig> InstanceVector;
#ifdef SENSCORD_SERVER_SETTING
  typedef std::vector<SearchSetting> SearchVector;
  typedef std::vector<ServerSetting> ServerVector;
#endif  // SENSCORD_SERVER_SETTING
  typedef std::map<std::string, std::string> ArgumentMap;
  typedef std::map<std::string, std::string> AllocatorMap;

  /**
   * @brief Default Configs.
   */
  struct DefaultConfigs {
    FrameBuffering frame_buffering;    /**< Frame buffering setting. */
#ifdef SENSCORD_SERVER
    std::string client_instance_name;   /**< Client instance name. */
#endif  // SENSCORD_SERVER
  };

  /**
   * @brief Analysis process of Config file
   * @param[in] (filename) Path of config file.
   * @return Status object.
   */
  Status ParseConfig(const std::string& filename);

  /**
   * @brief Parse sdk element.
   * @return Status object.
   */
  Status ParseSdk();

  /**
   * @brief Parse streams element.
   * @return Status object.
   */
  Status ParseStreams();

  /**
   * @brief Parse stream element.
   * @return Status object.
   */
  Status ParseStream();

  /**
   * @brief Parse address element.
   * @param[out] (config) Where to store the acquired config.
   * @return Status object.
   */
  Status ParseAddress(StreamSetting* config);

  /**
   * @brief Parse frame element.
   * @param[out] (config) Where to store the acquired config.
   */
  void ParseFrame(StreamSetting* config);

  /**
   * @brief Get the value of the Buffer attribute of Frame.
   * @param[out] (config) Where to store the acquired config.
   */
  void ParseAttributeBuffering(StreamSetting* config);

  /**
   * @brief Get the value of the num attribute of Frame.
   * @param[out] (config) Where to store the acquired config.
   */
  void ParseAttributeBufferingNum(StreamSetting* config);

  /**
   * @brief Get the value of the format attribute of Frame.
   * @param[out] (config) Where to store the acquired config.
   */
  void ParseAttributeBufferingFormat(StreamSetting* config);

  /**
   * @brief Parse extension element.
   * @param[out] (config) Where to store the acquired config.
   */
  Status ParseExtension(StreamSetting* config);

#ifdef SENSCORD_SERVER
  /**
   * @brief Parse client element.
   * @param[out] (config) Where to store the acquired config.
   * @return Status object.
   */
  Status ParseClient(StreamSetting* config);
#endif  // SENSCORD_SERVER

  /**
   * @brief Parse instances element.
   * @return Status object.
   */
  Status ParseInstances();

  /**
   * @brief Parse instance element.
   * @return Status object.
   */
  Status ParseInstance();

  /**
   * @brief Parse allocators element.
   * @param[in] (parent_xpath) Parent XPath.
   * @param[out] (instance_config) Where to store the acquired config.
   * @return Status object.
   */
  Status ParseAllocators(
      const std::string& parent_xpath, AllocatorMap* allocators);

  /**
   * @brief Parse allocator element.
   * @param[out] (instance_config) Where to store the acquired config.
   * @return Status object.
   */
  Status ParseAllocatorKey(AllocatorMap* allocators);

  /**
   * @brief Parse arguments element.
   * @param[in] (parent_xpath) Parent XPath.
   * @param[out] (argument_map) Where to store the acquired config.
   * @return Status object.
   */
  Status ParseArguments(
      const std::string& parent_xpath, ArgumentMap* argument_map);

  /**
   * @brief Parse argument element.
   * @param[out] (argument_map) Where to store the acquired config.
   * @return Status object.
   */
  Status ParseArgument(ArgumentMap* argument_map);

  /**
   * @brief Apply default value to config.
   */
  void ApplyDefaultConfig();

  /**
   * @brief Parse instances/defaults element.
   * @return Status object.
   */
  Status ParseInstancesDefaults();

  /**
   * @brief Parse streams/default element.
   * @return Status object.
   */
  Status ParseStreamsDefaults();

#ifdef SENSCORD_SERVER_SETTING
 /**
   * @brief Parse searches element.
   * @return Status object.
   */
  Status ParseSearches();

  /**
   * @brief Parse search element.
   * @return Status object.
   */
  Status ParseSearch();

  /**
   * @brief Parse servers element.
   * @return Status object.
   */
  Status ParseServers();

  /**
   * @brief Parse server element.
   * @return Status object.
   */
  Status ParseServer();
#endif  // SENSCORD_SERVER_SETTING

  /**
   * @brief Parse core element.
   * @return Status object.
   */
  Status ParseCore();

#ifdef SENSCORD_LOG_ENABLED
  /**
   * @brief Check the log element attribute value.
   * @param[in] (attr_value) level or severity attribute value.
   * @param[out] (output) Log level.
   * @return true if successful.
   */
  bool CheckLogLevel(
      const std::string& attr_value, LogLevel* output);

  /**
   * @brief Parse instances log element.
   * @param[out] (log_level) Log level.
   * @return Status object.
   */
  Status ParseLog(LogLevel* log_level);

  /**
   * @brief Parse core/log element.
   * @param[in] (core_config) Core configuration.
   * @return Status object.
   */
  Status ParseCoreLog(CoreConfig* core_config);
#endif  // SENSCORD_LOG_ENABLED

#ifdef SENSCORD_STREAM_VERSION
  /**
   * @brief Parse version element.
   * @return Status object.
   */
  Status ParseVersion();
#endif  // SENSCORD_STREAM_VERSION

#ifdef SENSCORD_SERVER
  /**
   * @brief Count the same client instance name.
   * @param[in] (client_instance_name) Client instance name.
   * @return Number of count.
   */
  uint32_t GetCountSameClientInstance(
      const std::string& client_instance_name) const;

  /**
   * @brief Update client informations.
   * @return Status object.
   */
  Status UpdateClientInstances();

  /**
   * @brief Add allocator keys to client component instance.
   * @param[in] (dest_instance_name) Client instance name to insert.
   * @param[in] (src_address) Port of source.
   * @return Status object.
   */
  Status AddAllocatorKey(
      const std::string& dest_instance_name,
      const StreamAddress& src_address);

  /**
   * @brief Add allocator keys to client component instance.
   * @param[in] (dest_instance_name) Client instance name to insert.
   * @param[in] (allocator_keys) List of allocator key.
   * @return Status object.
   */
  Status AddAllocatorKey(
      const std::string& dest_instance_name,
      const std::map<std::string, std::string>& allocator_keys);
#endif  // SENSCORD_SERVER

  /**
   * @brief Check whether an instance corresponding to the instance name exists.
   * @param[in] (use_instance_name) Name of the interface to be used
   * @return Status object.
   */
  Status CheckExistInstance(const std::string& use_instance_name);

  /**
   * @brief Check the validity of stream's config.
   * @return Status object.
   */
  Status VerifyStream();

  /**
   * @brief Check the validity of config.
   * @return Status object.
   */
  Status VerifyConfig();

  /**
   * @brief Apply default value to frame buffer config.
   * @param[in,out] (config) Applies to default config.
   * @param[in] (default_config) Default config.
   */
  void ApplyDefaultFrameBufferConfig(
      FrameBuffering* config, const FrameBuffering& default_config);

  /**
   * @brief Verify frame buffer config.
   * @param[in,out] (config) Verify target stream config.
   * @return Status object.
   */
  Status VerifyFrameBufferConfig(StreamSetting* config);

  /**
   * @brief Add identification.
   * @param[in] (identification) SensCord identification.
   * @return Status object.
   */
  Status AddIdentification(const std::string& identification);

  /**
   * @brief Concatenate Id string.
   * @param[out] (target) Target string.
   * @param[in] (identification) SensCord identification.
   * @return Status object.
   */
  Status ConcatenateIdString(std::string* target,
                             const std::string& identification);

  /**
   * @brief Print the definition of Stream.
   */
  void PrintStreamConfig();

  /**
   * @brief Print the frame buffer config.
   * @param[in] (buffer_config) Frame buffer config.
   */
  void PrintBuffering(const FrameBuffering &buffer_config);

  /**
   * @brief Print port arguments setting.
   * @param[in] (arguments) Arguments to pass to the port.
   */
  void PrintPortArgument(
      const std::map<std::string, std::string>& arguments);

  /**
   * @brief Print default parameter setting.
   */
  void PrintDefaultConfig();

  /**
   * @brief Print instance definition.
   */
  void PrintInstanceConfig();

  /**
   * @brief Print component arguments setting.
   * @param [in] (arguments) Arguments to pass to the component.
   */
  void PrintComponentArguments(
       const std::map<std::string, std::string>& arguments);

  /**
   * @brief Print allocator keys.
   * @param[in] (allocator_key_list) List of key of the allocator.
   */
  void PrintAllocator(
      const std::map<std::string, std::string>& allocator_key_list);

#ifdef SENSCORD_SERVER_SETTING
  /**
   * @brief Print search setting.
   */
  void PrintSearchConfig();

  /**
   * @brief Print server setting.
   */
  void PrintServerConfig();
#endif  // SENSCORD_SERVER_SETTING

#ifdef SENSCORD_STREAM_VERSION
  /**
   * @brief Print project version.
   */
  void PrintProjectVersion();
#endif  // SENSCORD_STREAM_VERSION

#ifdef SENSCORD_LOG_ENABLED
  /**
   * @brief Print log severities
   */
  void PrintLogSeverity();

  /**
   * @brief Get severity label from LogSeverity.
   * @param[in] (severity) Type of severity.
   * @return severity text
   */
  std::string GetLogSeverityLabel(
      util::Logger::LogSeverity severity);
#endif  // SENSCORD_LOG_ENABLED

  // XML Parser Class.
  util::XmlParser* parser_;

  // SDK's Config
  CoreConfig core_config_;

  // SDK's Default config
  DefaultConfigs default_config_;

  // config lock object
  util::Mutex* mutex_;

  // Config read flag
  bool read_;

  // SensCord identification
  std::string identification_;

  // Server config manager
  ServerConfigManager* server_config_manager_;
};

}    // namespace senscord
#endif  // LIB_CORE_CORE_CONFIG_MANAGER_H_
