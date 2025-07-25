/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_SENSCORD_H_
#define SENSCORD_SENSCORD_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/senscord_types.h"
#include "senscord/status.h"
#include "senscord/noncopyable.h"
#include "senscord/frame.h"
#include "senscord/stream.h"
#include "senscord/configuration.h"
#include "senscord/messenger.h"

namespace senscord {

// internal class
class StreamManager;
class ConfigManager;
class ServerConfigManager;
class CoreFunctionLockManager;
class CoreBehavior;

/**
 * @brief The core class of managing streams.
 */
class Core : private util::Noncopyable {
 public:
  /**
   * @brief Initialize Core, called at once.
   * @return Status object.
   */
  virtual Status Init();
  virtual Status Init(const Configuration* config);

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
  virtual Status GetStreamList(std::vector<StreamTypeInfo> *streamtypeinfo);

  /**
   * @brief Get the count of opened by stream key in the process.
   * @param[in] (stream_key) Stream key.
   * @param[out] (count) Opened count.
   * @return Status object.
   */
  virtual Status GetOpenedStreamCount(
    const std::string& stream_key,
    uint32_t* count);

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
   * @brief Open the new stream from key.
   * @param[in] (key) The key of the stream to open.
   * @param[out] (stream) The new stream pointer.
   * @return Status object.
   */
  Status OpenStream(const std::string& key, Stream** stream) {
    return OpenStream(key, NULL, stream);
  }

  /**
   * @brief Open the new stream from key and specified setting.
   * @param[in] (key) The key of the stream to open.
   * @param[in] (setting) Setting to open stream.
   * @param[out] (stream) The new stream pointer.
   * @return Status object.
   */
  Status OpenStream(
      const std::string& key,
      const OpenStreamSetting& setting,
      Stream** stream) {
    return OpenStream(key, &setting, stream);
  }

  /**
   * @brief Close the opened stream.
   * @param[in] (stream) The opened stream pointer.
   * @return Status object.
   */
  virtual Status CloseStream(Stream* stream);

  /**
   * @brief Register the callback for release frame.
   * @param[in] (param) Publisher parameter.
   * @param[in] (frameinfo) The information about used extension frames.
   */
  typedef void (* OnReleaseFrameCallback)(
      const PublisherParam& param, const FrameInfo& frameinfo);

  /**
   * @brief Open the new publisher from key.
   * @param[out] (publisher) The new publisher pointer.
   * @param[in] (key) The key of the stream to open.
   * @param[in] (callback) Callback for release frame.
   * @return Status object.
   */
  Status OpenPublisher(
      Publisher** publisher, const std::string& key,
      OnReleaseFrameCallback callback) {
    return OpenPublisher(publisher, "", key, callback);
  }

  /**
   * @brief Open the new publisher from key (specify server).
   * @param[out] (publisher) The new publisher pointer.
   * @param[in] (server) The connect to server name.
   * @param[in] (key) The key of the stream to open.
   * @param[in] (callback) Callback for release frame.
   * @return Status object.
   */
  Status OpenPublisher(
      Publisher** publisher, const std::string& server,
      const std::string& key, OnReleaseFrameCallback callback);

  /**
   * @brief Close the opened publisher.
   * @param[in] (publisher) The opened publisher pointer.
   * @return Status object.
   */
  Status ClosePublisher(Publisher* publisher);

  /**
   * @brief Constructor.
   */
  Core();

  /**
   * @brief Destructor.
   */
  virtual ~Core();

 protected:
  /**
   * @brief Set the core behavior.
   * @param[in] (behavior) The core behavior.
   */
  void SetBehavior(CoreBehavior* behavior);

  /**
   * @brief Copy and set the config manager.
   * @param[in] (config_manager) The config manager.
   */
  void SetConfigManager(const ConfigManager* config_manager);

 private:
  /**
   * @brief Open the new stream.
   * @param[in] (key) The key of the stream to open.
   * @param[in] (setting) Setting to open stream.
   * @param[out] (stream) The new stream pointer.
   * @return Status object.
   */
  virtual Status OpenStream(
      const std::string& key,
      const OpenStreamSetting* setting,
      Stream** stream);

 private:
  // Stream manager
  StreamManager* stream_manager_;
  // Config manager
  ConfigManager* config_manager_;

  // Exclusive lock manager
  CoreFunctionLockManager* lock_manager_;

  // Core behavior
  CoreBehavior* behavior_;
};

}    // namespace senscord
#endif  // SENSCORD_SENSCORD_H_
