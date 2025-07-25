/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_PROPERTY_ACCESSOR_H_
#define SENSCORD_DEVELOP_PROPERTY_ACCESSOR_H_

#include <stdint.h>
#include <string>

#include "senscord/config.h"
#include "senscord/noncopyable.h"
#include "senscord/status.h"
#ifndef SENSCORD_SERIALIZE
#include "senscord/develop/property_factory.h"
#endif  // SENSCORD_SERIALIZE

namespace senscord {

/**
 * @brief Property accessor interface class.
 */
class PropertyAccessor : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   * @param[in] (key) Key of property.
   */
  explicit PropertyAccessor(const std::string& key) : key_(key) {}

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
    size_t serialized_size) = 0;

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
    size_t* serialized_size) = 0;

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
    size_t serialized_size) = 0;
#else
  /**
   * @brief Set the property.
   * @param[in] (key) Key of property.
   * @param[in] (property) Property address.
   * @return Status object.
   */
  virtual Status Set(const std::string& key, const void* property) = 0;

  /**
   * @brief Get the property.
   * @param[in] (key) Key of property.
   * @param[in,out] (property) Property address.
   * @return Status object.
   */
  virtual Status Get(const std::string& key, void* property) = 0;
#endif  // SENSCORD_SERIALIZE

  /**
   * @brief Get the key of property
   * @return Property access key.
   */
  const std::string& GetKey() const {
    return key_;
  }

  /**
   * @brief Virtual destructor.
   */
  virtual ~PropertyAccessor() {}

 private:
  std::string key_;
};

#ifndef SENSCORD_SERIALIZE

/**
 * @brief Fast property accessor.
 */
template <class C, typename T>
class FastPropertyAccessor : public PropertyAccessor {
 public:
  /**
   * @brief Constructor.
   * @param[in] (key) Key of property.
   * @param[in] (accessor) Owner of property accessors.
   */
  explicit FastPropertyAccessor(const std::string& key, C* accessor)
      : PropertyAccessor(key), accessor_(accessor) {}

  /**
   * @brief Destructor.
   */
  virtual ~FastPropertyAccessor() {}

  /**
   * @brief Set the property.
   * @param[in] (key) Key of property.
   * @param[in] (property) Property address.
   * @return Status object.
   */
  virtual Status Set(const std::string& key, const void* property) {
    const T* tmp = reinterpret_cast<const T*>(property);
    Status status = accessor_->Set(key, tmp);
    return SENSCORD_STATUS_TRACE(status);
  }

  /**
   * @brief Get the property.
   * @param[in] (key) Key of property.
   * @param[in,out] (property) Property address.
   * @return Status object.
   */
  virtual Status Get(const std::string& key, void* property) {
    T* tmp = reinterpret_cast<T*>(property);
    Status status = accessor_->Get(key, tmp);
    return SENSCORD_STATUS_TRACE(status);
  }

 private:
  C* accessor_;
};

#endif  // SENSCORD_SERIALIZE

}  // namespace senscord

#endif  // SENSCORD_DEVELOP_PROPERTY_ACCESSOR_H_
