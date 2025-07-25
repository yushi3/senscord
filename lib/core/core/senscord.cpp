/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/senscord.h"

#include <string>
#include <vector>

#include "logger/logger.h"
#include "core/internal_types.h"
#include "core/core_behavior.h"
#include "core/stream_manager.h"
#include "core/config_manager.h"
#include "core/core_function_lock_manager.h"
#include "configuration/configuration_core.h"
#include "util/mutex.h"
#include "util/autolock.h"
#include "util/singleton.h"
#include "messenger/publisher_core.h"

namespace senscord {

/**
 * @brief Constructor.
 */
Core::Core() {
  util::SingletonManager::Init();

  stream_manager_ = new StreamManager();
  config_manager_ = new ConfigManager();
  lock_manager_ = new CoreFunctionLockManager();
  behavior_ = new DefaultCoreBehavior();
}

/**
 * @brief Destructor.
 */
Core::~Core() {
  // execute the exit process whenever possible
  Exit();

  delete behavior_;
  behavior_ = NULL;
  delete lock_manager_;
  lock_manager_ = NULL;
  delete config_manager_;
  config_manager_ = NULL;
  delete stream_manager_;
  stream_manager_ = NULL;

  util::SingletonManager::Exit();
}

/**
 * @brief Set the core behavior.
 * @param[in] (behavior) The core behavior.
 */
void Core::SetBehavior(CoreBehavior* behavior) {
  delete behavior_;
  behavior_ = behavior;
}

/**
 * @brief Copy and set the config manager.
 * @param[in] (config_manager) The config manager.
 */
void Core::SetConfigManager(const ConfigManager* config_manager) {
  *config_manager_ = *config_manager;
}

/**
 * @brief Initialize Core, called at once.
 * @return Status object.
 */
Status Core::Init() {
  return Init(NULL);
}

/**
 * @brief Initialize Core, called at once.
 * @param[in] (configuration) The Configuration pointer.
 * @return Status object.
 */
Status Core::Init(const Configuration* config) {
  CoreFunctionLock lock(lock_manager_, kFunctionTypeInit);
  if (!lock.status().ok()) {
    return SENSCORD_STATUS_TRACE(lock.status());
  }

  if (behavior_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "behavior is null");
  }

  if (!config_manager_->IsLoaded()) {
    if (config) {
      const ConfigurationCore& configuration_core =
          *static_cast<const ConfigurationCore*>(config);
      config_manager_->SetConfig(configuration_core.GetConfig());
    } else {
      ConfigurationCore configuration_core;
      Status status = configuration_core.InitConfig();
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
      config_manager_->SetConfig(configuration_core.GetConfig());
    }
  }

  Status status = behavior_->Init(stream_manager_, config_manager_);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  lock_manager_->SetCoreInitialized(true);
  return Status::OK();
}

/**
 * @brief Finalize Core and close all opened streams.
 * @return Status object.
 */
Status Core::Exit() {
  CoreFunctionLock lock(lock_manager_, kFunctionTypeExit);
  if (!lock.status().ok()) {
    return SENSCORD_STATUS_TRACE(lock.status());
  }

  if (behavior_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "behavior is null");
  }
  Status status = behavior_->Exit();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  lock_manager_->SetCoreInitialized(false);
  return Status::OK();
}

/**
 * @brief Get supported streams list.
 * @param[out] (streamtypeinfo) Supported streams list.
 * @return Status object.
 */
Status Core::GetStreamList(std::vector<StreamTypeInfo> *streamtypeinfo) {
  if (streamtypeinfo == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "streamtypeinfo is null");
  }

  CoreFunctionLock lock(lock_manager_, kFunctionTypeReadOnly);
  if (!lock.status().ok()) {
    return SENSCORD_STATUS_TRACE(lock.status());
  }

  if (behavior_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "behavior is null");
  }
  Status status = behavior_->GetStreamList(streamtypeinfo);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get the count of opened by stream key.
 * @param[in] (stream_key) Stream key.
 * @param[out] (count) Opened count.
 * @return Status object.
 */
Status Core::GetOpenedStreamCount(
    const std::string& stream_key,
    uint32_t* count) {
  if (count == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "count is null");
  }
  if (stream_key.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "stream_key is empty");
  }

  CoreFunctionLock lock(lock_manager_, stream_key, *config_manager_);
  if (!lock.status().ok()) {
    return SENSCORD_STATUS_TRACE(lock.status());
  }

  if (behavior_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "behavior is null");
  }
  Status status = behavior_->GetOpenedStreamCount(stream_key, count);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get the version of this core library.
 * @param[out] (version) The version of this core library.
 * @return Status object.
 */
Status Core::GetVersion(SensCordVersion* version) {
  if (version == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  CoreFunctionLock lock(lock_manager_, kFunctionTypeReadOnly);
  if (!lock.status().ok()) {
    return SENSCORD_STATUS_TRACE(lock.status());
  }

  if (behavior_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "behavior is null");
  }
  Status status = behavior_->GetVersion(version);
  return SENSCORD_STATUS_TRACE(status);
}

#ifdef SENSCORD_SERVER_SETTING
/**
 * @brief Get the configuration of this core library.
 * @param[out] (config) The configuration of this core library.
 * @return Status object.
 */
Status Core::GetConfig(ServerConfig* config) {
  if (config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseInvalidArgument,
                                "invalid parameter");
  }

  CoreFunctionLock lock(lock_manager_, kFunctionTypeReadOnly);
  if (!lock.status().ok()) {
    return SENSCORD_STATUS_TRACE(lock.status());
  }

  if (behavior_ == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidOperation, "behavior is null");
  }
  Status status = behavior_->GetConfig(config);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_SERVER_SETTING

/**
 * @brief Open the new stream.
 * @param[in] (key) The key of the stream to open.
 * @param[in] (setting) Setting to open stream.
 * @param[out] (stream) The new stream pointer.
 * @return Status object.
 */
Status Core::OpenStream(
    const std::string& key,
    const OpenStreamSetting* setting,
    Stream** stream) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "stream is null");
  }
  if (key.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "key is empty");
  }

  CoreFunctionLock lock(lock_manager_, key, *config_manager_);
  if (!lock.status().ok()) {
    return SENSCORD_STATUS_TRACE(lock.status());
  }

  if (behavior_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "behavior is null");
  }
  // Open stream.
  Status status = behavior_->OpenStream(key, setting, stream);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Close the opened stream.
 * @param[in] (stream) The opened stream pointer.
 * @return Status object.
 */
Status Core::CloseStream(Stream* stream) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "stream is null");
  }

  CoreFunctionLock lock(
      lock_manager_, stream_manager_, stream, *config_manager_);
  if (!lock.status().ok()) {
    return SENSCORD_STATUS_TRACE(lock.status());
  }

  if (behavior_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "behavior is null");
  }

  // already closed, returns the returned status.
  bool is_closed = false;
  Status status = lock_manager_->GetCloseStreamStatus(stream, &is_closed);
  if (!status.ok() || is_closed) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Close stream.
  status = behavior_->CloseStream(stream);

  lock_manager_->SetCloseStreamStatus(stream, status);

  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Open the new publisher from key.
 * @param[out] (publisher) The new publisher pointer.
 * @param[in] (key) The key of the stream to open.
 * @param[in] (callback) Callback for release frame.
 * @return Status object.
 */
Status Core::OpenPublisher(
    Publisher** publisher, const std::string& server,
    const std::string& key, OnReleaseFrameCallback callback) {
  if (publisher == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "publisher is null");
  }
  if (callback == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "callback is null");
  }
  if (key.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "key is empty");
  }

  CoreFunctionLock lock(lock_manager_, key, *config_manager_);
  if (!lock.status().ok()) {
    return SENSCORD_STATUS_TRACE(lock.status());
  }

  if (behavior_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "behavior is null");
  }

  Status status = behavior_->OpenPublisher(publisher, server, key, callback);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Close the opened publisher.
 * @param[in] (publisher) The opened publisher pointer.
 * @return Status object.
 */
Status Core::ClosePublisher(Publisher* publisher) {
  if (publisher == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "publisher is null");
  }

  PublisherCore* publisher_core = static_cast<PublisherCore*>(publisher);
  CoreFunctionLock lock(
      lock_manager_, publisher_core->GetKey(), *config_manager_);

  Status status = behavior_->ClosePublisher(publisher);
  return SENSCORD_STATUS_TRACE(status);
}

}    // namespace senscord
