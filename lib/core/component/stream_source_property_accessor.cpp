/*
 * SPDX-FileCopyrightText: 2020-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "component/stream_source_property_accessor.h"

#include <string>

#include "component/stream_source_adapter.h"
#include "component/stream_source_function_lock_manager.h"

namespace senscord {

/**
 * @brief Constructor.
 * @param[in] (key) Key of property.
 * @param[in] (lock_manager) FunctionLockManager of owner class.
 */
StreamSourcePropertyAccessor::StreamSourcePropertyAccessor(
    const std::string& key,
    StreamSourceFunctionLockManager* lock_manager)
    : PropertyAccessor(key), accessor_(NULL), lock_manager_(lock_manager) {}

/**
 * @brief Destructor.
 */
StreamSourcePropertyAccessor::~StreamSourcePropertyAccessor() {
  delete accessor_;
  accessor_ = NULL;
}

#ifdef SENSCORD_SERIALIZE
/**
 * @brief Set the serialized property.
 * @param[in] (key) Key of property.
 * @param[in] (serialized_property) Serialized property address.
 * @param[in] (serialized_size) Serialized property size.
 * @return Status object.
 */
Status StreamSourcePropertyAccessor::Set(
    const std::string& key,
    const void* serialized_property,
    size_t serialized_size) {
  if (accessor_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseInvalidArgument,
        "accessor is NULL");
  }
  StreamSourceFunctionLock lock(lock_manager_, kFunctionTypeProperty);
  Status status = accessor_->Set(key, serialized_property, serialized_size);
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
Status StreamSourcePropertyAccessor::Get(
    const std::string& key,
    const void* serialized_input_property,
    size_t serialized_input_size,
    void** serialized_property,
    size_t* serialized_size) {
  if (accessor_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseInvalidArgument,
        "accessor is NULL");
  }
  StreamSourceFunctionLock lock(lock_manager_, kFunctionTypeProperty);
  Status status = accessor_->Get(key, serialized_input_property,
      serialized_input_size, serialized_property, serialized_size);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Release the serialized property.
 * @param[in] (key) Key of property.
 * @param[in] (serialized_property) Serialized property address by Get().
 * @param[in] (serialized_size) Serialized property size by Get().
 * @return Status object.
 */
Status StreamSourcePropertyAccessor::Release(
    const std::string& key,
    void* serialized_property,
    size_t serialized_size) {
  if (accessor_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseInvalidArgument,
        "accessor is NULL");
  }
  Status status = accessor_->Release(key, serialized_property, serialized_size);
  return SENSCORD_STATUS_TRACE(status);
}
#else
/**
 * @brief Set the property.
 * @param[in] (key) Key of property.
 * @param[in] (property) Property address.
 * @return Status object.
 */
Status StreamSourcePropertyAccessor::Set(
    const std::string& key, const void* property) {
  if (accessor_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseInvalidArgument,
        "accessor is NULL");
  }
  StreamSourceFunctionLock lock(lock_manager_, kFunctionTypeProperty);
  Status status = accessor_->Set(key, property);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Set the property.
 * @param[in] (key) Key of property.
 * @param[in,out] (property) Property address.
 * @return Status object.
 */
Status StreamSourcePropertyAccessor::Get(
    const std::string& key, void* property) {
  if (accessor_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseInvalidArgument,
        "accessor is NULL");
  }
  StreamSourceFunctionLock lock(lock_manager_, kFunctionTypeProperty);
  Status status = accessor_->Get(key, property);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_SERIALIZE

}   // namespace senscord
