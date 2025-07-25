/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CONFIGURATION_CORE_CONFIG_H_
#define LIB_CORE_CONFIGURATION_CORE_CONFIG_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "senscord/senscord_types.h"
#include "senscord/logger.h"
#include "senscord/develop/memory_allocator_types.h"

namespace senscord {

/**
 * @brief The address of stream
 */
struct StreamAddress {
  std::string instance_name;    /**< Component instance name. */
  std::string port_type;        /**< Port type. */
  int32_t port_id;              /**< Connection port ID. */
};

/**
 * @brief Settings for the extension library.
 */
struct ExtensionSetting {
  std::string library_name;  /**< Library name */
  std::map<std::string, std::string> arguments;   /**< Arguments */
  std::map<std::string, std::string> allocators;  /**< Allocators */
};

/**
 * @brief Stream setting.
 */
struct StreamSetting {
  /** Stream key. */
  std::string stream_key;
  /** Connecting port address. */
  StreamAddress address;
  /** Frame buffering setting. */
  FrameBuffering frame_buffering;
  /** Stream arguments. (ComponentPortArgument) */
  std::map<std::string, std::string> arguments;
  /** Settings for extension libraries. */
  std::vector<ExtensionSetting> extensions;
  /** ID for each senscord process. */
  std::string identification;
#ifdef SENSCORD_SERVER
  /** Radical address for client stream. */
  StreamAddress radical_address;
  /** Client instance name. */
  std::string client_instance_name;
  /** Whether or not the client tag is specified. */
  bool client_specified;
#else
  const StreamAddress& radical_address;
#endif  // SENSCORD_SERVER

#ifdef SENSCORD_SERVER
  StreamSetting() : address(), radical_address(), client_specified() {
    address.port_id = -1;
    frame_buffering.buffering = kBufferingDefault;
    frame_buffering.num = kBufferNumDefault;
    frame_buffering.format = kBufferingFormatDefault;
  }
#else
  StreamSetting() : address(), radical_address(address) {
    address.port_id = -1;
    frame_buffering.buffering = kBufferingDefault;
    frame_buffering.num = kBufferNumDefault;
    frame_buffering.format = kBufferingFormatDefault;
  }

  StreamSetting(const StreamSetting& rhs)
      : stream_key(rhs.stream_key)
      , address(rhs.address)
      , frame_buffering(rhs.frame_buffering)
      , arguments(rhs.arguments)
      , extensions(rhs.extensions)
      , identification(rhs.identification)
      , radical_address(address) {}

  StreamSetting& operator=(const StreamSetting& rhs) {
    stream_key = rhs.stream_key;
    address = rhs.address;
    frame_buffering = rhs.frame_buffering;
    arguments = rhs.arguments;
    extensions = rhs.extensions;
    identification = rhs.identification;
    return *this;
  }
#endif  // SENSCORD_SERVER
};

/**
 * @brief Component instance configuration.
 */
struct ComponentInstanceConfig {
  std::string instance_name;
  std::string component_name;
  /** Instance arguments. (ComponentArgument) */
  std::map<std::string, std::string> arguments;
  std::map<std::string, std::string> allocator_key_list;
};

#ifdef SENSCORD_SERVER_SETTING
/**
 * @brief Search setting.
 */
struct SearchSetting {
  std::string name;
  bool is_enabled;
  std::map<std::string, std::string> arguments;
};

/**
 * @brief Server setting.
 */
struct ServerSetting {
  std::map<std::string, std::string> arguments;
};
#endif  // SENSCORD_SERVER_SETTING

/**
 * @brief Core configuration.
 */
struct CoreConfig {
  std::vector<StreamSetting> stream_list;
  std::vector<ComponentInstanceConfig> instance_list;
  std::vector<AllocatorConfig> allocator_list;
#ifdef SENSCORD_LOG_ENABLED
  std::map<std::string, LogLevel> tag_logger_list;
#endif  // SENSCORD_LOG_ENABLED
#ifdef SENSCORD_SERVER_SETTING
  std::vector<SearchSetting> search_list;
  std::vector<ServerSetting> server_list;
#endif  // SENSCORD_SERVER_SETTING
#ifdef SENSCORD_STREAM_VERSION
  Version project_version;
#endif  // SENSCORD_STREAM_VERSION
};

/**
 * @brief Searches stream config by stream key.
 * @param[in] (stream_list) Stream config list.
 * @param[in] (stream_key) Stream key.
 * @return Stream config.
 */
StreamSetting* GetStreamConfig(
    std::vector<StreamSetting>* stream_list,
    const std::string& stream_key);

/**
 * @brief Searches stream config by stream key. (Backward match)
 * @param[in] (stream_list) Stream config list.
 * @param[in] (stream_key) Stream key.
 * @return Stream config.
 */
const StreamSetting* GetStreamConfigBackwardMatch(
    const std::vector<StreamSetting>* stream_list,
    const std::string& stream_key);

/**
 * @brief Searches component config by instance name.
 * @param[in] (instance_list) Instance config list.
 * @param[in] (instance_name) Instance name.
 * @return Component config.
 */
ComponentInstanceConfig* GetComponentConfig(
    std::vector<ComponentInstanceConfig>* instance_list,
    const std::string& instance_name);
const ComponentInstanceConfig* GetComponentConfig(
    const std::vector<ComponentInstanceConfig>* instance_list,
    const std::string& instance_name);

/**
 * @brief Searches allocator config by allocator key.
 * @param[in] (allocator_list) Allocator config list.
 * @param[in] (allocator_key) Allocator key.
 * @return Allocator config.
 */
AllocatorConfig* GetAllocatorConfig(
    std::vector<AllocatorConfig>* allocator_list,
    const std::string& allocator_key);
const AllocatorConfig* GetAllocatorConfig(
    const std::vector<AllocatorConfig>* allocator_list,
    const std::string& allocator_key);

}  // namespace senscord

#endif  // LIB_CORE_CONFIGURATION_CORE_CONFIG_H_
