/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_SERIALIZED_PROPERTY_ACCESSOR_H_
#define SENSCORD_DEVELOP_SERIALIZED_PROPERTY_ACCESSOR_H_

#include <stdint.h>
#include <string>

#include "senscord/config.h"
#include "senscord/develop/property_accessor.h"

#ifdef SENSCORD_SERIALIZE

namespace senscord {

/**
 * @brief Property accessor for BinaryProperty.
 */
template <class C>
class SerializedPropertyAccessor : public PropertyAccessor {
 public:
  /**
   * @brief Constructor.
   * @param[in] (key) Key of property.
   * @param[in] (accessor) Owner of property accessors.
   */
  explicit SerializedPropertyAccessor(const std::string& key, C* accessor);

  /**
   * @brief Set the serialized property.
   * @param[in] (key) Key of property.
   * @param[in] (serialized_property) Serialized property address.
   * @param[in] (serialized_size) Serialized property size.
   * @return Status object.
   */
  virtual Status Set(
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
  virtual Status Get(
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
  virtual Status Release(
    const std::string& key,
    void* serialized_property,
    size_t serialized_size);

  /**
   * @brief Destructor.
   */
  ~SerializedPropertyAccessor() {}

 private:
  C* accessor_;
};

}   // namespace senscord

// implementations of template methods.
#include "senscord/develop/serialized_property_accessor_private.h"

#endif  // SENSCORD_SERIALIZE
#endif  // SENSCORD_DEVELOP_SERIALIZED_PROPERTY_ACCESSOR_H_
