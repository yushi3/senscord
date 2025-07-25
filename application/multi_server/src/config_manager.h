/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APPLICATION_MULTI_SERVER_CONFIG_MANAGER_H_
#define APPLICATION_MULTI_SERVER_CONFIG_MANAGER_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "senscord/osal.h"
#include "senscord/status.h"
#include "senscord/senscord.h"
#include "multi_server.h"

namespace senscord {
namespace server {

/**
 * @brief Config manager class
 */
class ConfigManager : private util::Noncopyable {
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
   * @brief Read the specified Config file
   * @param[in] (config_path) Path of config file.
   * @return Status object.
   */
  Status ReadConfig(const std::string& config_path);

  /**
   * @brief Set the server configuration.
   * @param[in] (server_config) Server configuration to be set.
   * @return Status object.
   */
  Status SetConfig(const ServerConfig& server_config);

  /**
   * @brief Get whether the client function is enabled or not.
   * @param[out] (enabled) If enabled returns true.
   * @return Status object.
   */
  Status GetClientEnabled(bool* enabled) const;

  /**
   * @brief Get the listener setting list.
   * @param[out] (listeners) Listener setting list.
   * @return Status object.
   */
  Status GetListenerList(std::vector<ListenerSetting>* listeners) const;

  /**
   * @brief Search by stream key and return stream config.
   * @param[in] (stream_key) Search stream key.
   * @param[in] (connection_key) Search connection key.
   * @param[out] (config) Return stream config.
   * @return Status object.
   */
  Status GetStreamConfigByStreamKey(
      const std::string& stream_key,
      const std::string& connection_key,
      OpenStreamSetting* config) const;

  /**
   * @brief Verify whether it is a supported stream.
   * @param[in] (supported_streams) Supported streams list.
   */
  void VerifySupportedStream(
      const std::vector<StreamTypeInfo>& supported_streams) const;

  /**
   * @brief Print the contents of Config analyzed by ConfigManager.
   */
  void PrintConfig() const;

 private:
  /**
   * @brief Search by stream key and return stream setting.
   * @param[in] (stream_key) Search stream key.
   * @return Return stream setting.
   */
  const StreamSetting* GetStreamSetting(const std::string& stream_key) const;

  /**
   * @brief Backward matching or not.
   * @param[in] (target) Search target string.
   * @param[in] (suffix) The string of backward matches to search for.
   * @return True is matched backwards.
   */
  bool IsBackwardMatch(
      const std::string& target, const std::string& suffix) const;

  /**
   * @brief Perform parameter check of Config.
   * @param[in] (server_config) Server configuration.
   * @return Status object.
   */
  Status VerifyConfig(const ServerConfig& server_config) const;

  /**
   * @brief Clear the read Config information.
   */
  void ClearConfig();

  /**
   * @brief Set default config to Config.
   * @param[out] (config) Where to set the default config.
   */
  void SetDefaultStreamConfig(FrameBuffering* config) const;

  /**
   * @brief Analysis process of Config file
   * @param[in] (filename) Path of config file.
   * @return Status object.
   */
  Status ParseConfig(const std::string& filename);

  /**
   * @brief Parse server element and obtain it as Config.
   * @return Status object.
   */
  Status ParseServer();

  /**
   * @brief Analyze the element nodes of the server.
   * @param[in] (element) Element name.
   * @return Status object.
   */
  Status ParseServerElementNode(const std::string& element);

  /**
   * @brief Parse streams element and obtain it as Config.
   * @return Status object.
   */
  Status ParseStreams();

  /**
   * @brief Analyze the element nodes of the streams.
   * @param[in] (element) Element name.
   * @return Status object.
   */
  Status ParseStreamsElementNode(const std::string& element);

  /**
   * @brief Parse stream element and obtain it as Config.
   * @return Status object.
   */
  Status ParseStream();

  /**
   * @brief Analyze children in stream and reflect on Config
   * @param[out] (stream_setting) Where to store the acquired config.
   * @return Status object.
   */
  Status ParseStreamChildNode(StreamSetting* stream_setting);

  /**
   * @brief Analyze the element nodes of the stream.
   * @param[in] (element) Element name.
   * @param[out] (stream_setting) Where to store the acquired config.
   * @return Status object.
   */
  Status ParseStreamElementNode(
      const std::string& element, StreamSetting* stream_setting);

  /**
   * @brief Parse frame element and obtain it as Config.
   * @param[out] (connection_buffering) Where to store the acquired config.
   */
  void ParseFrame(ConnectionBuffering* connection_buffering);

  /**
   * @brief Is the target included in listeners.
   * @param[in] (connection) target connection.
   * @return true is contains.
   */
  bool ContainsListener(const std::string& connection);

  /**
   * @brief Get the value of the Buffer attribute of Frame.
   * @param[out] (buffering) Where to store the acquired config.
   */
  void ParseAttributeBuffering(Buffering* buffering);

  /**
   * @brief Get the value of the num attribute of Frame.
   * @param[out] (num) Where to store the acquired config.
   */
  void ParseAttributeBufferingNum(int32_t* num);

  /**
   * @brief Get the value of the format attribute of Frame.
   * @param[out] (format) Where to store the acquired config.
   */
  void ParseAttributeBufferingFormat(BufferingFormat* format);

  /**
   * @brief Parse default element and obtain it as Config.
   * @return Status object.
   */
  Status ParseDefaults();

  /**
   * @brief Analyze the element nodes of the default.
   * @param[in] (element) Element name.
   * @return Status object.
   */
  Status ParseDefaultElementNode(const std::string& element);

  /**
   * @brief Parse listeners element and obtain it as Config.
   * @return Status object.
   */
  Status ParseListeners();

  /**
   * @brief Analyze the element nodes of the listeners.
   * @param[in] (element) Element name.
   * @return Status object.
   */
  Status ParseListenersElementNode(const std::string& element);

  /**
   * @brief Parse listener element and obtain it as Config.
   * @return Status object.
   */
  Status ParseListener();

  /**
   * @brief Parse connection attribute of config.
   * @param[out] (value) Connection attribute value.
   * @return Status object.
   */
  Status ParseAttributeConnection(std::string* value);

  /**
   * @brief Parse address (or addressPrimary) attribute of config.
   * @param[out] (value) Address attribute value.
   * @return Status object.
   */
  Status ParseAttributeAddress(std::string* value);

  /**
   * @brief Parse addressSecondary attribute of config.
   * @param[out] (value) Address attribute value.
   */
  void ParseAttributeAddressSecondary(std::string* value);

  /**
   * @brief Parse client attribute of config.
   * @param[out] (client_enabled) Whether the client is enabled.
   */
  void ParseAttributeClient(bool* client_enabled);

  /**
   * @brief Parse key attribute of config.
   * @param[out] (value) Key attribute value.
   * @return Status object.
   */
  Status ParseAttributeKey(std::string* value);

  /**
   * @brief Print the definition of Stream.
   */
  void PrintStreamConfig() const;

  /**
   * @brief Print the frame buffer config.
   * @param[in] (buffer_config) Frame buffer config.
   * @param[in] (connection) Apply to connection.
   */
  void PrintBuffering(
      const FrameBuffering &buffer_config, const std::string& connection) const;

  /**
   * @brief Print default parameter setting.
   */
  void PrintDefaultConfig() const;

  // config lock object
  osal::OSMutex* mutex_;

  // XML Parser Class.
  osal::OSXmlParser* parser_;

  // The path of the current configuration file.
  std::string current_config_path_;

  // Server config
  ServerConfig server_config_;

  // Default stream settings.
  StreamSetting default_stream_setting_;
};

}  // namespace server
}  // namespace senscord

#endif  // APPLICATION_MULTI_SERVER_CONFIG_MANAGER_H_
