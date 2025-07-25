/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_CONFIGURATION_H_
#define SENSCORD_CONFIGURATION_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/logger.h"
#include "senscord/noncopyable.h"
#include "senscord/senscord_types.h"
#include "senscord/status.h"

namespace senscord {
/**
 * @brief Type that represents the <arguments> of senscord.xml
 */
typedef std::map<std::string, std::string> ConfigArgument;

const char kSearchTypeSsdp[] = "ssdp";
const char kSearchTypeUcom[] = "ucom";
const char kLogSeverityTypeCore[] = "core";
const char kLogSeverityTypeInstance[] = "instance";
const char kLogLevelTypeCore[] = "core";
const char kLogLevelTypeDefaults[] = "defaults";

/** Default allocator key */
const char kDefaultAllocatorKey[] = "_default";

/**
 * @brief Interface class of Configuration API
 */
class Configuration : private util::Noncopyable {
 public:
  /**
   * @brief destructor.
   */
  virtual ~Configuration() {}

  /**
   * @brief Adds a stream.
   * @param[in] (stream_key) Stream key.
   * @param[in] (instance_name) Component instance name.
   * @param[in] (stream_type) Stream type.
   * @param[in] (port_id) Port id.
   * @return Status object.
   */
  virtual Status AddStream(
      const std::string& stream_key,
      const std::string& instance_name,
      const std::string& stream_type,
      int32_t port_id) = 0;

  /**
   * @brief Sets the buffering mode of the stream.
   * @param[in] (stream_key) Stream key.
   * @param[in] (buffering) Buffering "ON" or "OFF".
   * @param[in] (num) Buffering frame number.
   * @param[in] (format) Buffering format.
   * @return Status object.
   */
  virtual Status SetStreamBuffering(
      const std::string& stream_key,
      Buffering buffering,
      int32_t num,
      BufferingFormat format) = 0;

  /**
   * @brief Adds a stream argument.
   * @param[in] (stream_key) Stream key.
   * @param[in] (argument_name) Argument name.
   * @param[in] (argument_value) Argument value.
   * @return Status object.
   */
  virtual Status AddStreamArgument(
      const std::string& stream_key,
      const std::string& argument_name,
      const std::string& argument_value) = 0;

  /**
   * @brief Adds an instance.
   * @param[in] (instance_name) Component instance name.
   * @param[in] (component_name) Component library name.
   * @return Status object.
   */
  virtual Status AddInstance(
      const std::string& instance_name,
      const std::string& component_name) = 0;

  /**
   * @brief Adds an instance argument.
   * @param[in] (instance_name) Component instance name.
   * @param[in] (argument_name) Argument name.
   * @param[in] (argument_value) Argument value.
   * @return Status object.
   */
  virtual Status AddInstanceArgument(
      const std::string& instance_name,
      const std::string& argument_name,
      const std::string& argument_value) = 0;

  /**
   * @brief Adds an instance allocator.
   * @param[in] (instance_name) Component instance name.
   * @param[in] (allocator_key) Allocator key.
   * @param[in] (allocator_name) Allocator name.
   * @return Status object.
   */
  virtual Status AddInstanceAllocator(
      const std::string& instance_name,
      const std::string& allocator_key,
      const std::string& allocator_name) = 0;

  /**
   * @brief Adds an allocator.
   * @param[in] (allocator_key) Allocator key.
   * @param[in] (type) Allocator type.
   * @param[in] (cacheable) Cacheable or not.
   * @return Status object.
   */
  virtual Status AddAllocator(
      const std::string& allocator_key,
      const std::string& type,
      bool cacheable) = 0;

  /**
   * @brief Adds an allocator argument.
   * @param[in] (allocator_key) Allocator key.
   * @param[in] (argument_name) Argument name.
   * @param[in] (argument_value) Argument value.
   * @return Status object.
   */
  virtual Status AddAllocatorArgument(
      const std::string& allocator_key,
      const std::string& argument_name,
      const std::string& argument_value) = 0;

  /**
   * @brief Set the SensCord server search configuration.
   * @param[in] (type) Search type.
   * @param[in] (is_enabled) Enable or disable of specified search.
   * @param[in] (arguments) Arguments for the specified search.
   * @return Status object.
   */
  virtual Status SetSearch(const std::string& type, const bool is_enabled,
      const ConfigArgument* arguments) = 0;

  /**
   * @brief Get the SensCord server search configuration.
   * @param[in] (type) Search type.
   * @param[out] (is_enabled) The enable/disable status of specified search.
   * @param[out] (arguments) Arguments for the specified search.
   * @return Status object.
   */
  virtual Status GetSearch(
      const std::string& type, bool* is_enabled, ConfigArgument* arguments) = 0;

  /**
   * @brief Set the log level.
   * @param[in] (type) Log type.
   * @param[in] (level) Log level.
   * @return Status object.
   */
  virtual Status SetLogLevel(
      const std::string& type, LogLevel level) = 0;

  /**
   * @brief Get the log level.
   * @param[in] (type) Log type.
   * @param[out] (level) Log level.
   * @return Status object.
   */
  virtual Status GetLogLevel(
      const std::string& type, LogLevel* level) = 0;

  /**
   * @brief Get server list.
   * @param[out] server list. (Key=UID)
   * @return Status object.
   */
  virtual Status GetServerList(
      std::map<uint32_t, ConfigArgument>* servers) = 0;

  /**
   * @brief Add server.
   * @param[in] (arguments) Arguments for the specified server.
   * @param[out] UID.
   * @return Status object.
   */
  virtual Status AddServer(
      const ConfigArgument& arguments, uint32_t* uid) = 0;

  /**
   * @brief Remove server.
   * @param[in] UID.
   * @param[out] (arguments) Arguments for the specified server.
   * @return Status object.
   */
  virtual Status RemoveServer(
      const uint32_t& uid, ConfigArgument* arguments) = 0;

  /**
   * @deprecated Use SetLogLevel().
   * @brief Set the log severity.
   * @param[in] (type) Log type.
   * @param[in] (severity) Log severity.
   * @return Status object.
   */
  Status SetLogSeverity(
      const std::string& type, const util::Logger::LogSeverity severity) {
    return SetLogLevel(type, severity);
  }

  /**
   * @deprecated Use GetLogLevel().
   * @brief Get the log severity.
   * @param[in] (type) Log type.
   * @param[out] (severity) Log severity.
   * @return Status object.
   */
  Status GetLogSeverity(
      const std::string& type, util::Logger::LogSeverity* severity) {
    return GetLogLevel(type, severity);
  }

  /**
   * @brief Create Configuration instance.
   * @return Configuration object.
   */
  static Status Create(Configuration** configuration);

  /**
   * @brief Delete Configuration instance.
   * @param[in] (configuration) Configuration object.
   */
  static void Delete(Configuration* configuration);
};

}  // namespace senscord

#endif  // SENSCORD_CONFIGURATION_H_
