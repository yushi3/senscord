/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "client_property_accessor.h"
#include <string>
#include "./client_log.h"
#include "./component_client.h"

namespace client {

/**
 * @brief Set the serialized property.
 * @param[in] (key) Key of property.
 * @param[in] (serialized_property) Serialized property address.
 * @param[in] (serialized_size) Serialized property size.
 * @return Status object.
 */
senscord::Status ClientPropertyAccessor::Set(
    const std::string& key,
    const void* serialized_property,
    size_t serialized_size) {
  SENSCORD_CLIENT_LOG_DEBUG("[client] SetProperty called: key=%s",
      key.c_str());
  senscord::Status status = client_component_->SetProperty(
      port_type_, port_id_, key,
      serialized_property, serialized_size);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get and create new serialized property.
 * @param[in] (key) Key of property.
 * @param[in] (serialized_input_property) Input serialized property address.
 * @param[in] (serialized_input_size) Input serialized property size.
 * @param[out] (serialized_property) New serialized property address.
 * @param[out] (serialized_size) Serialized property size.
 * @return Status object.
 */
senscord::Status ClientPropertyAccessor::Get(
    const std::string& key,
    const void* serialized_input_property,
    size_t serialized_input_size,
    void** serialized_property,
    size_t* serialized_size) {
  SENSCORD_CLIENT_LOG_DEBUG("[client] GetProperty called: key=%s",
      key.c_str());
  senscord::Status status = client_component_->GetProperty(
      port_type_, port_id_, key,
      serialized_input_property, serialized_input_size,
      serialized_property, serialized_size);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Release the serialized property.
 * @param[in] (key) Key of property.
 * @param[in] (serialized_property) Serialized property address by Get().
 * @param[in] (serialized_size) Serialized property size by Get().
 * @return Status object.
 */
senscord::Status ClientPropertyAccessor::Release(
    const std::string& key,
    void* serialized_property,
    size_t serialized_size) {
  SENSCORD_CLIENT_LOG_DEBUG("[client] ReleaseProperty called: key=%s",
      key.c_str());
  senscord::Status status = client_component_->ReleaseProperty(
      key, serialized_property, serialized_size);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Constructor.
 * @param[in] (key) Key of property.
 * @param[in] (client_component) The client component.
 * @param[in] (port_type) The type of port.
 * @param[in] (port_id) The ID of port type.
 */
ClientPropertyAccessor::ClientPropertyAccessor(
    const std::string& key,
    ClientComponent* client_component,
    const std::string& port_type,
    int32_t port_id)
    : PropertyAccessor(key), client_component_(client_component),
      port_type_(port_type), port_id_(port_id) {}

/**
 * @brief destructor.
 */
ClientPropertyAccessor::~ClientPropertyAccessor() {
  client_component_ = NULL;
}

}   // namespace client
