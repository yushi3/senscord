/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "c_api/c_config.h"

#include <string>

#include "senscord/c_api/senscord_c_api.h"
#include "senscord/senscord.h"
#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"
#include "c_api/c_common.h"

namespace c_api = senscord::c_api;
namespace util = senscord::util;
namespace osal = senscord::osal;

/**
 * @brief Creates the config.
 * @param[out] config  Config handle.
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_config_create(
    senscord_config_t* config) {
  SENSCORD_C_API_ARGUMENT_CHECK(config == NULL);
  senscord::Configuration* configuration = NULL;
  senscord::Status status = senscord::Configuration::Create(&configuration);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  c_api::ConfigHandle* handle = new c_api::ConfigHandle;
  handle->config = configuration;
  *config = c_api::ToHandle(handle);
  return 0;
}

/**
 * @brief Deletes the config.
 * @param[in] config  Config handle.
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_config_destroy(
    senscord_config_t config) {
  SENSCORD_C_API_ARGUMENT_CHECK(config == 0);
  c_api::ConfigHandle* handle = c_api::ToPointer<c_api::ConfigHandle*>(config);
  senscord::Configuration::Delete(handle->config);
  delete handle;
  return 0;
}

/**
 * @brief Adds a stream.
 * @param[in] config         Config handle.
 * @param[in] stream_key     Stream key.
 * @param[in] instance_name  Component instance name.
 * @param[in] stream_type    Stream type.
 * @param[in] port_id        Port id.
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_config_add_stream(
    senscord_config_t config,
    const char* stream_key,
    const char* instance_name,
    const char* stream_type,
    int32_t port_id) {
  SENSCORD_C_API_ARGUMENT_CHECK(config == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(stream_key == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(instance_name == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(stream_type == NULL);
  c_api::ConfigHandle* handle = c_api::ToPointer<c_api::ConfigHandle*>(config);
  senscord::Status status = handle->config->AddStream(
      stream_key, instance_name, stream_type, port_id);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Sets the buffering mode of the stream.
 * @param[in] config      Config handle.
 * @param[in] stream_key  Stream key.
 * @param[in] buffering   Buffering setting.
 * @param[in] num         Buffering frame number.
 * @param[in] format      Buffering format.
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_config_set_stream_buffering(
    senscord_config_t config,
    const char* stream_key,
    enum senscord_buffering_t buffering,
    int32_t num,
    enum senscord_buffering_format_t format) {
  SENSCORD_C_API_ARGUMENT_CHECK(config == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(stream_key == NULL);
  c_api::ConfigHandle* handle = c_api::ToPointer<c_api::ConfigHandle*>(config);
  senscord::Status status = handle->config->SetStreamBuffering(
      stream_key,
      static_cast<senscord::Buffering>(buffering), num,
      static_cast<senscord::BufferingFormat>(format));
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Adds a stream argument.
 * @param[in] config          Config handle.
 * @param[in] stream_key      Stream key.
 * @param[in] argument_name   Argument name.
 * @param[in] argument_value  Argument value.
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_config_add_stream_argument(
    senscord_config_t config,
    const char* stream_key,
    const char* argument_name,
    const char* argument_value) {
  SENSCORD_C_API_ARGUMENT_CHECK(config == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(stream_key == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(argument_name == NULL);
  std::string argument_value_str;
  if (argument_value != NULL) {
    argument_value_str = argument_value;
  }
  c_api::ConfigHandle* handle = c_api::ToPointer<c_api::ConfigHandle*>(config);
  senscord::Status status = handle->config->AddStreamArgument(
      stream_key, argument_name, argument_value_str);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Adds an instance.
 * @param[in] config          Config handle.
 * @param[in] instance_name   Component instance name.
 * @param[in] component_name  Component library name.
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_config_add_instance(
    senscord_config_t config,
    const char* instance_name,
    const char* component_name) {
  SENSCORD_C_API_ARGUMENT_CHECK(config == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(instance_name == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(component_name == NULL);
  c_api::ConfigHandle* handle = c_api::ToPointer<c_api::ConfigHandle*>(config);
  senscord::Status status = handle->config->AddInstance(
      instance_name, component_name);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Adds an instance argument.
 * @param[in] config          Config handle.
 * @param[in] instance_name   Component instance name.
 * @param[in] argument_name   Argument name.
 * @param[in] argument_value  Argument value.
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_config_add_instance_argument(
    senscord_config_t config,
    const char* instance_name,
    const char* argument_name,
    const char* argument_value) {
  SENSCORD_C_API_ARGUMENT_CHECK(config == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(instance_name == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(argument_name == NULL);
  std::string argument_value_str;
  if (argument_value != NULL) {
    argument_value_str = argument_value;
  }
  c_api::ConfigHandle* handle = c_api::ToPointer<c_api::ConfigHandle*>(config);
  senscord::Status status = handle->config->AddInstanceArgument(
      instance_name, argument_name, argument_value_str);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Adds an instance allocator.
 * @param[in] config          Config handle.
 * @param[in] instance_name   Component instance name.
 * @param[in] allocator_key   Allocator key.
 * @param[in] allocator_name  Allocator name.
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_config_add_instance_allocator(
    senscord_config_t config,
    const char* instance_name,
    const char* allocator_key,
    const char* allocator_name) {
  SENSCORD_C_API_ARGUMENT_CHECK(config == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(instance_name == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(allocator_key == NULL);
  std::string allocator_name_str;
  if (allocator_name != NULL) {
    allocator_name_str = allocator_name;
  }
  c_api::ConfigHandle* handle = c_api::ToPointer<c_api::ConfigHandle*>(config);
  senscord::Status status = handle->config->AddInstanceAllocator(
      instance_name, allocator_key, allocator_name_str);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Adds an allocator.
 * @param[in] config         Config handle.
 * @param[in] allocator_key  Allocator key.
 * @param[in] type           Allocator type.
 * @param[in] cacheable      Cacheable or not.
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_config_add_allocator(
    senscord_config_t config,
    const char* allocator_key,
    const char* type,
    int32_t cacheable) {
  SENSCORD_C_API_ARGUMENT_CHECK(config == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(allocator_key == NULL);
  std::string type_str;
  if (type != NULL) {
    type_str = type;
  }
  c_api::ConfigHandle* handle = c_api::ToPointer<c_api::ConfigHandle*>(config);
  senscord::Status status = handle->config->AddAllocator(
      allocator_key, type_str, cacheable);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Adds an allocator argument.
 * @param[in] config          Config handle.
 * @param[in] allocator_key   Allocator key.
 * @param[in] argument_name   Argument name.
 * @param[in] argument_value  Argument value.
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_config_add_allocator_argument(
    senscord_config_t config,
    const char* allocator_key,
    const char* argument_name,
    const char* argument_value) {
  SENSCORD_C_API_ARGUMENT_CHECK(config == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(allocator_key == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(argument_name == NULL);
  std::string argument_value_str;
  if (argument_value != NULL) {
    argument_value_str = argument_value;
  }
  c_api::ConfigHandle* handle = c_api::ToPointer<c_api::ConfigHandle*>(config);
  senscord::Status status = handle->config->AddAllocatorArgument(
      allocator_key, argument_name, argument_value_str);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Adds a converter.
 * @param[in] config  Config handle.
 * @param[in] converter_name   Converter name.
 * @param[in] enable_property  Enable Property conversion.
 * @param[in] enable_rawdata   Enable RawData conversion.
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_config_add_converter(
    senscord_config_t config,
    const char* converter_name,
    int32_t enable_property,
    int32_t enable_rawdata) {
  SENSCORD_C_API_ARGUMENT_CHECK(config == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(
      (converter_name == NULL) || (*converter_name == '\0'));
  c_api::ConfigHandle* handle = c_api::ToPointer<c_api::ConfigHandle*>(config);
  c_api::ConverterConfig info = {};
  info.library_name = converter_name;
  info.enable_property = enable_property;
  info.enable_rawdata = enable_rawdata;
  handle->converters.push_back(info);
  return 0;
}
