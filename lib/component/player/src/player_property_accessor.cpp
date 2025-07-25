/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "player_property_accessor.h"

#include <string>

#include "senscord/logger.h"
#include "./player_component.h"

namespace player {

/**
 * @brief Set the serialized property.
 * @param[in] (key) Key of property.
 * @param[in] (serialized_property) Serialized property address.
 * @param[in] (serialized_size) Serialized property size.
 * @return Status object.
 */
senscord::Status PlayerPropertyAccessor::Set(
    const std::string& key,
    const void* serialized_property,
    size_t serialized_size) {
  SENSCORD_LOG_DEBUG("[player] SetProperty called: key=%s",
      key.c_str());
  senscord::Status status = player_component_->SetProperty(
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
senscord::Status PlayerPropertyAccessor::Get(
    const std::string& key,
    const void* serialized_input_property,
    size_t serialized_input_size,
    void** serialized_property,
    size_t* serialized_size) {
  SENSCORD_LOG_DEBUG("[player] GetProperty called: key=%s",
      key.c_str());
  senscord::Status status = player_component_->GetProperty(
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
senscord::Status PlayerPropertyAccessor::Release(
    const std::string& key,
    void* serialized_property,
    size_t serialized_size) {
  SENSCORD_LOG_DEBUG("[player] ReleaseProperty called: key=%s",
      key.c_str());
  senscord::Status status = player_component_->ReleaseProperty(
      key, serialized_property, serialized_size);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Constructor.
 * @param[in] (key) Key of property.
 * @param[in] (player_component) The player component.
 * @param[in] (port_type) The type of port.
 * @param[in] (port_id) The ID of port type.
 */
PlayerPropertyAccessor::PlayerPropertyAccessor(
    const std::string& key,
    PlayerComponent* player_component,
    const std::string& port_type,
    int32_t port_id)
    : PropertyAccessor(key), player_component_(player_component),
      port_type_(port_type), port_id_(port_id) {}

/**
 * @brief destructor.
 */
PlayerPropertyAccessor::~PlayerPropertyAccessor() {
  player_component_ = NULL;
}

}   // namespace player
