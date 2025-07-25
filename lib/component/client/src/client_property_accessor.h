/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_CLIENT_CLIENT_PROPERTY_ACCESSOR_H_
#define LIB_COMPONENT_CLIENT_CLIENT_PROPERTY_ACCESSOR_H_

#include <stdint.h>
#include <string>
#include "senscord/develop/property_accessor.h"
#include "./component_client.h"

namespace client {

class ClientPropertyAccessor : public senscord::PropertyAccessor {
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
   * @param[in] (client_component) The client component.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   */
  explicit ClientPropertyAccessor(
    const std::string& key,
    ClientComponent* client_component,
    const std::string& port_type,
    int32_t port_id);

  /**
   * @brief destructor.
   */
  ~ClientPropertyAccessor();

 private:
  ClientComponent* client_component_;
  std::string port_type_;
  int32_t port_id_;
};

}   // namespace client
#endif  // LIB_COMPONENT_CLIENT_CLIENT_PROPERTY_ACCESSOR_H_
