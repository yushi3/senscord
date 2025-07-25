/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_PLAYER_SRC_PLAYER_PROPERTY_ACCESSOR_H_
#define LIB_COMPONENT_PLAYER_SRC_PLAYER_PROPERTY_ACCESSOR_H_

#include <stdint.h>
#include <string>
#include "senscord/develop/property_accessor.h"
#include "./player_component.h"

class PlayerComponent;

namespace player {

class PlayerPropertyAccessor : public senscord::PropertyAccessor {
 public:
  /**
   * @brief Set the serialized property.
   * @param[in] (key) Key of property.
   * @param[in] (serialized_property) Serialized property address.
   * @param[in] (serialized_size) Serialized property size.
   * @return Status object.
   */
  virtual senscord::Status Set(
    const std::string& key,
    const void* serialized_property,
    size_t serialized_size);

  /**
   * @brief Get and create new serialized property.
   * @param[in] (key) Key of property.
   * @param[in] (serialized_input_property) Input serialized property address.
   * @param[in] (serialized_input_size) Input serialized property size.
   * @param[out] (serialized_property) New serialized property address.
   * @param[out] (serialized_size) Serialized property size.
   * @return Status object.
   */
  virtual senscord::Status Get(
    const std::string& key,
    const void* serialized_input_property,
    size_t serialized_input_size,
    void** serialized_property,
    size_t* serialized_size);

  /**
   * @brief Release the serialized property.
   * @param[in] (key) Key of property.
   * @param[in] (serialized_property) Serialized property address by Get().
   * @param[in] (serialized_size) Serialized property size by Get().
   * @return Status object.
   */
  virtual senscord::Status Release(
    const std::string& key,
    void* serialized_property,
    size_t serialized_size);

  /**
   * @brief Constructor.
   * @param[in] (key) Key of property.
   * @param[in] (player_component) The player component.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   */
  explicit PlayerPropertyAccessor(
    const std::string& key,
    PlayerComponent* player_component,
    const std::string& port_type,
    int32_t port_id);

  /**
   * @brief destructor.
   */
  ~PlayerPropertyAccessor();

 private:
  PlayerComponent* player_component_;
  std::string port_type_;
  int32_t port_id_;
};

}   // namespace player
#endif  // LIB_COMPONENT_PLAYER_SRC_PLAYER_PROPERTY_ACCESSOR_H_
