/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CORE_CORE_BEHAVIOR_H_
#define LIB_CORE_CORE_CORE_BEHAVIOR_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "core/version_manager.h"
#include "senscord/senscord.h"
#include "senscord/messenger.h"
#include "senscord/senscord_types.h"
#include "senscord/property_types.h"
#include "senscord/status.h"
#include "senscord/stream.h"
#include "senscord/noncopyable.h"

namespace senscord {

// forward declaration
class StreamManager;
class ConfigManager;

/**
 * @brief Interface class that represents core behavior.
 */
class CoreBehavior : private util::Noncopyable {
 public:
  /**
   * @brief Destructor.
   */
  virtual ~CoreBehavior() {}

  /**
   * @brief Create instance.
   *
   * Use 'delete' to release the created instance.
   * @return created instance.
   */
  virtual CoreBehavior* CreateInstance() const = 0;

  /**
   * @brief Initialize Core.
   * @param[in] (stream_manager) The stream manager.
   * @param[in] (config_manager) The config manager.
   * @return Status object.
   */
  virtual Status Init(
      StreamManager* stream_manager, ConfigManager* config_manager) = 0;

  /**
   * @brief Finalize Core and close all opened streams.
   * @return Status object.
   */
  virtual Status Exit() = 0;

  /**
   * @brief Get supported streams list.
   * @param[out] (streamtypeinfo) Supported streams list.
   * @return Status object.
   */
  virtual Status GetStreamList(
      std::vector<StreamTypeInfo>* streamtypeinfo) = 0;

  /**
   * @brief Get the count of opened by stream key in the process.
   * @param[in] (stream_key) Stream key.
   * @param[out] (count) Opened count.
   * @return Status object.
   */
  virtual Status GetOpenedStreamCount(
      const std::string& stream_key, uint32_t* count) = 0;

  /**
   * @brief Get the version of this core library.
   * @param[out] (version) The version of this core library.
   * @return Status object.
   */
  virtual Status GetVersion(SensCordVersion* version) = 0;

#ifdef SENSCORD_SERVER_SETTING
  /**
   * @brief Get the configuration of this core library.
   * @param[out] (config) The configuration of this core library.
   * @return Status object.
   */
  virtual Status GetConfig(ServerConfig* config) = 0;
#endif  // SENSCORD_SERVER_SETTING

  /**
   * @brief Open the new stream from key and specified setting.
   * @param[in] (key) The key of the stream to open.
   * @param[in] (setting) Setting to open stream.
   * @param[out] (stream) The new stream pointer.
   * @return Status object.
   */
  virtual Status OpenStream(
      const std::string& key,
      const OpenStreamSetting* setting,
      Stream** stream) = 0;

  /**
   * @brief Close the opened stream.
   * @param[in] (stream) The opened stream pointer.
   * @return Status object.
   */
  virtual Status CloseStream(Stream* stream) = 0;

  /**
   * @brief Open the new publisher from key.
   * @param[out] (publisher) The new publisher pointer.
   * @param[in] (server) The connect to server name.
   * @param[in] (key) The key of the stream to open.
   * @param[in] (callback) Callback for release frame.
   * @return Status object.
   */
  virtual Status OpenPublisher(
      Publisher** publisher, const std::string& server,
      const std::string& key, Core::OnReleaseFrameCallback callback) = 0;

  /**
   * @brief Close the opened publisher.
   * @param[in] (publisher) The opened publisher pointer.
   * @return Status object.
   */
  virtual Status ClosePublisher(Publisher* publisher) = 0;

  /**
   * @brief Get config manager.
   * @return Config manager.
   */
  virtual ConfigManager* GetConfigManager() const = 0;
};

/**
 * @brief Default core behavior.
 */
class DefaultCoreBehavior : public CoreBehavior {
 public:
  /**
   * @brief Constructor.
   */
  DefaultCoreBehavior();

  /**
   * @brief Destructor.
   */
  virtual ~DefaultCoreBehavior();

  /**
   * @brief Create instance.
   *
   * Use 'delete' to release the created instance.
   * @return created instance.
   */
  virtual CoreBehavior* CreateInstance() const;

  /**
   * @brief Initialize Core.
   * @param[in] (stream_manager) The stream manager.
   * @param[in] (config_manager) The stream manager.
   * @return Status object.
   */
  virtual Status Init(
      StreamManager* stream_manager, ConfigManager* config_manager);

  /**
   * @brief Finalize Core and close all opened streams.
   * @return Status object.
   */
  virtual Status Exit();

  /**
   * @brief Get supported streams list.
   * @param[out] (streamtypeinfo) Supported streams list.
   * @return Status object.
   */
  virtual Status GetStreamList(std::vector<StreamTypeInfo>* streamtypeinfo);

  /**
   * @brief Get the count of opened by stream key in the process.
   * @param[in] (stream_key) Stream key.
   * @param[out] (count) Opened count.
   * @return Status object.
   */
  virtual Status GetOpenedStreamCount(
      const std::string& stream_key, uint32_t* count);

  /**
   * @brief Get the version of this core library.
   * @param[out] (version) The version of this core library.
   * @return Status object.
   */
  virtual Status GetVersion(SensCordVersion* version);

#ifdef SENSCORD_SERVER_SETTING
  /**
   * @brief Get the configuration of this core library.
   * @param[out] (config) The configuration of this core library.
   * @return Status object.
   */
  virtual Status GetConfig(ServerConfig* config);
#endif  // SENSCORD_SERVER_SETTING

  /**
   * @brief Open the new stream from key and specified setting.
   * @param[in] (key) The key of the stream to open.
   * @param[in] (setting) Setting to open stream.
   * @param[out] (stream) The new stream pointer.
   * @return Status object.
   */
  virtual Status OpenStream(
      const std::string& key,
      const OpenStreamSetting* setting,
      Stream** stream);

  /**
   * @brief Close the opened stream.
   * @param[in] (stream) The opened stream pointer.
   * @return Status object.
   */
  virtual Status CloseStream(Stream* stream);

  /**
   * @brief Open the new publisher from key.
   * @param[out] (publisher) The new publisher pointer.
   * @param[in] (server) The connect to server name.
   * @param[in] (key) The key of the stream to open.
   * @param[in] (callback) Callback for release frame.
   * @return Status object.
   */
  virtual Status OpenPublisher(
      Publisher** publisher, const std::string& server,
      const std::string& key, Core::OnReleaseFrameCallback callback);

  /**
   * @brief Close the opened publisher.
   * @param[in] (publisher) The opened publisher pointer.
   * @return Status object.
   */
  virtual Status ClosePublisher(Publisher* publisher);

  /**
   * @brief Get config manager.
   * @return Config manager.
   */
  virtual ConfigManager* GetConfigManager() const;

 protected:
  /**
   * @brief Get stream manager.
   * @return Stream manager.
   */
  StreamManager* GetStreamManager() const;

#ifdef SENSCORD_STREAM_VERSION
  /**
   * @brief Read component config.
   * @return Status object.
   */
  virtual Status ReadComponentConfig();
#endif  // SENSCORD_STREAM_VERSION

  /**
   * @brief Get stream config.
   * @param[in] (key) The key of the stream to open.
   * @param[in] (setting) Setting to open stream.
   * @param[out] (config) Stream config are returned.
   * @return Status object.
   */
  Status GetStreamConfig(
      const std::string& key,
      const OpenStreamSetting* setting,
      StreamSetting* config) const;

 protected:
  // Version manager
  VersionManager* version_manager_;

 private:
  struct Impl;
  Impl* pimpl_;
};

}  // namespace senscord

#endif  // LIB_CORE_CORE_CORE_BEHAVIOR_H_
