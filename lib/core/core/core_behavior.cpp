/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core/core_behavior.h"

#include <stdint.h>
#include <string>
#include <map>

#include "core/version.h"
#include "core/internal_types.h"
#include "core/config_manager.h"
#include "core/stream_manager.h"
#include "stream/stream_core.h"
#include "util/senscord_utils.h"
#include "component/component_manager.h"
#include "component/component_adapter.h"
#include "allocator/memory_manager.h"
#include "record/recorder_manager.h"
#include "extension/extension_manager.h"
#include "senscord/connection_manager.h"
#include "senscord/environment.h"
#include "messenger/messenger_manager.h"

namespace senscord {

struct DefaultCoreBehavior::Impl {
  ConfigManager* config_manager;
  StreamManager* stream_manager;
  ExtensionManager* extension;
  MessengerManager* messenger;
};

/**
 * @brief Constructor.
 */
DefaultCoreBehavior::DefaultCoreBehavior() :
    version_manager_(), pimpl_(new Impl()) {
}

/**
 * @brief Destructor.
 */
DefaultCoreBehavior::~DefaultCoreBehavior() {
  delete pimpl_;
  pimpl_ = NULL;

  delete version_manager_;
  version_manager_ = NULL;
}

/**
 * @brief Create instance.
 *
 * Use 'delete' to release the created instance.
 * @return created instance.
 */
CoreBehavior* DefaultCoreBehavior::CreateInstance() const {
  return new DefaultCoreBehavior;
}

/**
 * @brief Initialize Core.
 * @param[in] (stream_manager) The stream manager.
 * @param[in] (config_manager) The config manager.
 * @return Status object.
 */
Status DefaultCoreBehavior::Init(
    StreamManager* stream_manager, ConfigManager* config_manager) {
  SENSCORD_STATUS_ARGUMENT_CHECK(stream_manager == NULL);
  SENSCORD_STATUS_ARGUMENT_CHECK(config_manager == NULL);

  Status status;
  pimpl_->stream_manager = stream_manager;
  pimpl_->config_manager = config_manager;
  const CoreConfig* config = config_manager->GetConfig();

#ifdef SENSCORD_LOG_ENABLED
  for (std::map<std::string, LogLevel>::const_iterator
      itr = config->tag_logger_list.begin(),
      end = config->tag_logger_list.end();
      itr != end; ++itr) {
    util::LoggerFactory::GetInstance()->CreateLogger(
        itr->first, itr->second);
  }
  config_manager->PrintConfig();
#endif  // SENSCORD_LOG_ENABLED

  // initialize memory manager
  status = MemoryManager::GetInstance()->Init(config->allocator_list);
  SENSCORD_STATUS_TRACE(status);

#ifdef SENSCORD_RECORDER
  // initialize recorder manager
  if (status.ok()) {
    status = RecorderManager::GetInstance()->Init();
    SENSCORD_STATUS_TRACE(status);
  }
#endif  // SENSCORD_RECORDER
#ifdef SENSCORD_SERVER
  // initialize connection manager
  if (status.ok()) {
    status = ConnectionManager::GetInstance()->Init();
    SENSCORD_STATUS_TRACE(status);
  }
#endif  // SENSCORD_SERVER
#ifdef SENSCORD_SERVER_SETTING
  // read server config
  if (status.ok()) {
    status = config_manager->ReadServerConfig();
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      config_manager->PrintConfig();
    }
  }
#endif  // SENSCORD_SERVER_SETTING

  if (status.ok() && config->stream_list.empty()) {
    status = SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseAborted,
        "stream is not registered");
  }

  if (status.ok()) {
    ExtensionManager* extension = ExtensionManager::GetInstance();
    status = extension->Init(*config);
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      pimpl_->extension = extension;
    }
  }

  if (status.ok()) {
    MessengerManager* messenger = MessengerManager::GetInstance();
    status = messenger->Init();
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      pimpl_->messenger = messenger;
    }
  }

  ComponentManager::GetInstance();

#ifdef SENSCORD_STREAM_VERSION
  // read component config
  if (status.ok()) {
    status = ReadComponentConfig();
    SENSCORD_STATUS_TRACE(status);
  }
#endif  // SENSCORD_STREAM_VERSION

  if (status.ok() && (version_manager_ == NULL)) {
    version_manager_ = new VersionManager(config_manager);
  }

  if (!status.ok()) {
    Exit();
  }

  return status;
}

/**
 * @brief Finalize Core and close all opened streams.
 * @return Status object.
 */
Status DefaultCoreBehavior::Exit() {
  Status status;
  if (pimpl_->stream_manager != NULL) {
    StreamCore* stream = NULL;
    while ((stream = pimpl_->stream_manager->GetRegisteredStream()) != NULL) {
      SENSCORD_LOG_WARNING(
          "close the stream that is still open. stream_key=%s",
          stream->GetKey().c_str());
      status = CloseStream(stream);
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
    }
    pimpl_->stream_manager->ReleaseStreamAll();
  }

  if (status.ok()) {
    delete version_manager_;
    version_manager_ = NULL;
  }

  if (status.ok() && (pimpl_->messenger != NULL)) {
    status = pimpl_->messenger->Exit();
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      pimpl_->messenger = NULL;
    }
  }

  if (status.ok() && (pimpl_->extension != NULL)) {
    status = pimpl_->extension->Exit();
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      pimpl_->extension = NULL;
    }
  }

  if (status.ok()) {
    if (pimpl_->config_manager != NULL) {
      pimpl_->config_manager->ClearConfig();
      pimpl_->config_manager = NULL;
    }

    pimpl_->stream_manager = NULL;
  }

  return status;
}

/**
 * @brief Get supported streams list.
 * @param[out] (streamtypeinfo) Supported streams list.
 * @return Status object.
 */
Status DefaultCoreBehavior::GetStreamList(
    std::vector<StreamTypeInfo>* streamtypeinfo) {
  if (streamtypeinfo == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "streamtypeinfo is null");
  }

  // set supported streams list.
  streamtypeinfo->clear();
  const CoreConfig* config = pimpl_->config_manager->GetConfig();

  for (std::vector<StreamSetting>::const_iterator
      itr = config->stream_list.begin(), end = config->stream_list.end();
      itr != end; ++itr) {
    StreamTypeInfo info = {};
    info.key  = itr->stream_key;
    info.type = itr->radical_address.port_type;
    info.id = itr->identification;
    streamtypeinfo->push_back(info);
  }

  return Status::OK();
}

/**
 * @brief Get the count of opened by stream key.
 * @param[in] (stream_key) Stream key.
 * @param[out] (count) Opened count.
 * @return Status object.
 */
Status DefaultCoreBehavior::GetOpenedStreamCount(
    const std::string& stream_key, uint32_t* count) {
  if (count == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "count is null");
  }

  const StreamSetting* stream_config =
      pimpl_->config_manager->GetStreamConfigByStreamKey(stream_key);
  if (stream_config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound,
        "unable to get config from Stream key : key=%s",
        stream_key.c_str());
  }

  Status status;
  ComponentAdapter* adapter = ComponentManager::GetInstance()->GetAdapter(
      stream_config->address.instance_name);
  if (adapter == NULL) {
    // not opened yet.
    *count = 0;
  } else {
    status = adapter->GetOpenedStreamCount(
        stream_config->address.port_type, stream_config->address.port_id,
        count);
    SENSCORD_STATUS_TRACE(status);
  }

  return status;
}

/**
 * @brief Get the version of this core library.
 * @param[out] (version) The version of this core library.
 * @return Status object.
 */
Status DefaultCoreBehavior::GetVersion(SensCordVersion* version) {
  if (version == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "version is null");
  }
  if (version_manager_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "version_manager_ is null");
  }

  Status status = version_manager_->GetVersion(version, false);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  return Status::OK();
}

#ifdef SENSCORD_SERVER_SETTING
/**
 * @brief Get the configuration of this core library.
 * @param[out] (config) The configuration of this core library.
 * @return Status object.
 */
Status DefaultCoreBehavior::GetConfig(ServerConfig* config) {
  if (config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseInvalidArgument,
                                "config is null");
  }

  Status status = pimpl_->config_manager->ReadServerConfig();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  status = pimpl_->config_manager->GetServerConfig(config);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return Status::OK();
}
#endif  // SENSCORD_SERVER_SETTING

/**
 * @brief Open the new stream from key and specified setting.
 * @param[in] (key) The key of the stream to open.
 * @param[in] (setting) Setting to open stream.
 * @param[out] (stream) The new stream pointer.
 * @return Status object.
 */
Status DefaultCoreBehavior::OpenStream(
    const std::string& key,
    const OpenStreamSetting* setting,
    Stream** stream) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "stream is null");
  }
  if (pimpl_->stream_manager == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "stream manager is invalid");
  }

  // Get stream config.
  StreamSetting open_config = {};
  Status status = GetStreamConfig(key, setting, &open_config);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  status = pimpl_->config_manager->VerifyStreamConfig(&open_config);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Get the new stream.
  StreamCore* stream_core = NULL;
  status = pimpl_->stream_manager->GetStream(open_config, &stream_core);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Open stream.
  status = stream_core->Open(this);
  if (!status.ok()) {
    pimpl_->stream_manager->ReleaseStream(stream_core);
    return SENSCORD_STATUS_TRACE(status);
  }
  *stream = stream_core;
  return status;
}

/**
 * @brief Close the opened stream.
 * @param[in] (stream) The opened stream pointer.
 * @return Status object.
 */
Status DefaultCoreBehavior::CloseStream(Stream* stream) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "stream is null");
  }
  if (pimpl_->stream_manager == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "stream manager is invalid");
  }

  StreamCore* stream_core = static_cast<StreamCore*>(stream);

  // Close stream.
  Status status = stream_core->Close();
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    status = pimpl_->stream_manager->ReleaseStream(stream_core);
    SENSCORD_STATUS_TRACE(status);
  }

  return status;
}

/**
 * @brief Open the new publisher from key.
 * @param[out] (publisher) The new publisher pointer.
 * @param[in] (server) The connect to server name.
 * @param[in] (key) The key of the stream to open.
 * @param[in] (callback) Callback for release frame.
 * @return Status object.
 */
Status DefaultCoreBehavior::OpenPublisher(
    Publisher** publisher, const std::string& server,
    const std::string& key, Core::OnReleaseFrameCallback callback) {
  StreamSetting open_config = {};
  Status status = GetStreamConfig(key, NULL, &open_config);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
#ifdef SENSCORD_SERVER
    if (!server.empty()) {
      open_config.client_instance_name = server;
    }
#endif  // SENSCORD_SERVER

    MessengerManager* msg_manager = MessengerManager::GetInstance();
    PublisherCore* publisher_core = NULL;
    status = msg_manager->GetPublisher(
        open_config, callback, this, &publisher_core);
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      *publisher = publisher_core;
    }
  }
  return status;
}

/**
 * @brief Close the opened publisher.
 * @param[in] (publisher) The opened publisher pointer.
 * @return Status object.
 */
Status DefaultCoreBehavior::ClosePublisher(Publisher* publisher) {
  MessengerManager* msg_manager = MessengerManager::GetInstance();
  PublisherCore* publisher_core = static_cast<PublisherCore*>(publisher);
  Status status = msg_manager->ReleasePublisher(publisher_core);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get stream manager.
 * @return Stream manager.
 */
StreamManager* DefaultCoreBehavior::GetStreamManager() const {
  return pimpl_->stream_manager;
}

/**
 * @brief Get config manager.
 * @return Config manager.
 */
ConfigManager* DefaultCoreBehavior::GetConfigManager() const {
  return pimpl_->config_manager;
}

#ifdef SENSCORD_STREAM_VERSION
/**
 * @brief Read component config.
 * @return Status object.
 */
Status DefaultCoreBehavior::ReadComponentConfig() {
  // get instance name
  std::vector<std::string> instance_name_list;
  Status status =
      pimpl_->config_manager->GetInstanceNameList(&instance_name_list);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  // read config
  ComponentManager* component_manager = ComponentManager::GetInstance();
  status = component_manager->ReadComponentConfig(
      pimpl_->config_manager, instance_name_list);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_STREAM_VERSION

/**
 * @brief Get stream config.
 * @param[in] (key) The key of the stream to open.
 * @param[in] (setting) Setting to open stream.
 * @param[out] (config) Stream config are returned.
 * @return Status object.
 */
Status DefaultCoreBehavior::GetStreamConfig(
    const std::string& key, const OpenStreamSetting* setting,
    StreamSetting* config) const {
  if (config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "config is null");
  }
  // Get stream config.
  const StreamSetting* stream_config =
      pimpl_->config_manager->GetStreamConfigByStreamKey(key);
  if (stream_config == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound,
        "unable to get config from Stream key : key=%s",
        key.c_str());
  }

  *config = *stream_config;
  if (setting != NULL) {
    config->frame_buffering = setting->frame_buffering;
    // marge stream argument, overwrite same name.
    for (std::map<std::string, std::string>::const_iterator itr =
        setting->arguments.begin(); itr != setting->arguments.end(); ++itr) {
      config->arguments[itr->first] = itr->second;
    }
  }
  return Status::OK();
}

}  // namespace senscord
