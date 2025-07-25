/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "clientless_core_behavior.h"

#include <stdint.h>
#include <string>
#include <vector>

// TODO: private header access
#include "core/config_manager.h"
#include "core/stream_manager.h"
#include "component/component_manager.h"
#include "stream/stream_core.h"

namespace senscord {
namespace server {

/**
 * @brief Constructor.
 */
ClientlessCoreBehavior::ClientlessCoreBehavior() {
}

/**
 * @brief Destructor.
 */
ClientlessCoreBehavior::~ClientlessCoreBehavior() {
}

/**
 * @brief Create instance.
 *
 * Use 'delete' to release the created instance.
 * @return created instance.
 */
CoreBehavior* ClientlessCoreBehavior::CreateInstance() const {
  return new ClientlessCoreBehavior;
}

#ifdef SENSCORD_STREAM_VERSION
/**
 * @brief Read component config.
 * @return Status object.
 */
Status ClientlessCoreBehavior::ReadComponentConfig() {
  // get config
  ConfigManager* config_manager = GetConfigManager();
  const CoreConfig* config = config_manager->GetConfig();
  // get instance name(unique, clientless)
  std::vector<std::string> instance_name_list;
  typedef std::vector<StreamSetting>::const_iterator stream_it;
  stream_it sitr = config->stream_list.begin();
  stream_it send = config->stream_list.end();
  for (; sitr != send; ++sitr) {
    if (std::find(instance_name_list.begin(), instance_name_list.end(),
        sitr->radical_address.instance_name) == instance_name_list.end()) {
      instance_name_list.push_back(sitr->radical_address.instance_name);
    }
  }
  // read config
  ComponentManager* component_manager = ComponentManager::GetInstance();
  Status status = component_manager->ReadComponentConfig(
      config_manager, instance_name_list);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get the version of this core library.
 * @param[out] (version) The version of this core library.
 * @return Status object.
 */
Status ClientlessCoreBehavior::GetVersion(SensCordVersion* version) {
  if (version == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "version is null");
  }
  if (version_manager_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "version_manager is null");
  }

  Status status = version_manager_->GetVersion(version, true);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_STREAM_VERSION

/**
 * @brief Open the new stream from key and specified setting.
 * @param[in] (key) The key of the stream to open.
 * @param[in] (setting) Setting to open stream.
 * @param[out] (stream) The new stream pointer.
 * @return Status object.
 */
Status ClientlessCoreBehavior::OpenStream(
    const std::string& key,
    const OpenStreamSetting* setting,
    Stream** stream) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "stream is null");
  }
  StreamManager* stream_manager = GetStreamManager();
  if (stream_manager == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "stream manager is invalid");
  }

  // Get stream config.
  StreamSetting open_config = {};
  Status status = GetStreamConfig(key, setting, &open_config);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // set the real address. (clientless)
  open_config.address = open_config.radical_address;

  ConfigManager* config_manager = GetConfigManager();
  status = config_manager->VerifyStreamConfig(&open_config);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Get the new stream.
  StreamCore* stream_core = NULL;
  status = stream_manager->GetStream(open_config, &stream_core);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Open stream.
  status = stream_core->Open(this);
  if (!status.ok()) {
    stream_manager->ReleaseStream(stream_core);
    return SENSCORD_STATUS_TRACE(status);
  }
  *stream = stream_core;
  return status;
}

}  // namespace server
}  // namespace senscord
