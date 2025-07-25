/*
 * SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_COMPONENT_STREAM_SOURCE_PROPERTY_ACCESSOR_H_
#define LIB_CORE_COMPONENT_STREAM_SOURCE_PROPERTY_ACCESSOR_H_

#include <string>
#include "component/stream_source_function_lock_manager.h"
#include "senscord/develop/property_accessor.h"

namespace senscord {

/**
 * @brief Property accessor for StreamSource.
 */
class StreamSourcePropertyAccessor : public PropertyAccessor {
 public:
  /**
   * @brief Constructor.
   * @param[in] (key) Key of property.
   * @param[in] (lock_manager) FunctionLockManager of owner class.
   */
  StreamSourcePropertyAccessor(
      const std::string& key,
      StreamSourceFunctionLockManager* lock_manager);

#ifdef SENSCORD_SERIALIZE
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
#else
  /**
   * @brief Set the property.
   * @param[in] (key) Key of property.
   * @param[in] (property) Property address.
   * @return Status object.
   */
  virtual Status Set(const std::string& key, const void* property);

  /**
   * @brief Set the property.
   * @param[in] (key) Key of property.
   * @param[in,out] (property) Property address.
   * @return Status object.
   */
  virtual Status Get(const std::string& key, void* property);
#endif  // SENSCORD_SERIALIZE

  /**
   * @brief Set property accessor.
   * @param[in] (accessor) Owner of property accessors.
   */
  void SetPropertyAccessor(PropertyAccessor* accessor) {
    accessor_ = accessor;
  }

  /**
   * @brief Destructor.
   */
  ~StreamSourcePropertyAccessor();

 private:
  PropertyAccessor* accessor_;
  StreamSourceFunctionLockManager* lock_manager_;
};

}   // namespace senscord

#endif  // LIB_CORE_COMPONENT_STREAM_SOURCE_PROPERTY_ACCESSOR_H_
