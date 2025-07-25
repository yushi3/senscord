/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_STREAM_PRIVATE_H_
#define SENSCORD_STREAM_PRIVATE_H_

#include <string>

#include "senscord/config.h"
#include "senscord/stream.h"
#include "senscord/serialize.h"

namespace senscord {

// internal class
/**
 * @brief Type of function.
 */
enum StreamFunctionType {
  kStreamFunctionTypeState,
  kStreamFunctionTypeInternal,
  kStreamFunctionTypeComponent,
};

/**
 * @brief RAII-style function lock.
 */
class StreamFunctionLock {
 public:
  /**
   * @brief Acquire the lock of the function.
   * @param[in] (manager) Lock manager.
   * @param[in] (type) Type of function.
   */
  StreamFunctionLock(
      StreamFunctionLockManager* manager, StreamFunctionType type);

  /**
   * @brief Release the lock of the function.
   */
  ~StreamFunctionLock();

  /**
   * @brief Get the lock status.
   * @return Status object.
   */
  Status GetStatus() const;

 private:
  // lock manager
  StreamFunctionLockManager* manager_;
  // lock status
  Status status_;
  // locked flag
  bool locked_;
};

#ifdef SENSCORD_SERIALIZE
/**
 * @brief Get the property.
 * @param[in] (property_key) Key of property to get.
 * @param[out] (property) Location of property.
 * @return Status object.
 */
template <typename T>
Status Stream::GetProperty(const std::string& property_key, T* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "property is null");
  }
  StreamFunctionLock lock(lock_manager_, kStreamFunctionTypeComponent);
  if (!lock.GetStatus().ok()) {
    return SENSCORD_STATUS_TRACE(lock.GetStatus());
  }

  // Serialize input property.
  serialize::SerializedBuffer buffer;
  serialize::Encoder encoder(&buffer);
  Status status = encoder.Push(*property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Get serialized property.
  void* serialized = NULL;
  size_t serialized_length = 0;
  status = GetSerializedProperty(property_key, buffer.data(),
      buffer.size(), &serialized, &serialized_length);
  SENSCORD_STATUS_TRACE(status);

  // Deserialization.
  if (status.ok()) {
    serialize::Decoder decoder(serialized, serialized_length);
    status = decoder.Pop(*property);
    SENSCORD_STATUS_TRACE(status);
  }

  // Release serialized.
  if (serialized != NULL) {
    Status status2 = ReleaseSerializedProperty(property_key, serialized,
        serialized_length);
    if (status.ok()) {
      status = status2;
      SENSCORD_STATUS_TRACE(status);
    }
  }
  return status;
}

/**
 * @brief Get the binary property.
 * @param[in] (property_key) Key of property to get.
 * @param[out] (property) Location of serialized property.
 * @return Status object.
 */
template <>
inline Status Stream::GetProperty(const std::string& property_key,
                                  BinaryProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "property is null");
  }
  StreamFunctionLock lock(lock_manager_, kStreamFunctionTypeComponent);
  if (!lock.GetStatus().ok()) {
    return SENSCORD_STATUS_TRACE(lock.GetStatus());
  }

  // Get serialized property.
  void* serialized = NULL;
  size_t serialized_length = 0;
  const void* input_ptr = NULL;
  size_t input_size = 0;
  if (!property->data.empty()) {
    input_ptr = &property->data[0];
    input_size = property->data.size();
  }
  Status status = GetSerializedProperty(property_key, input_ptr,
      input_size, &serialized, &serialized_length);
  SENSCORD_STATUS_TRACE(status);

  if (status.ok()) {
    uint8_t* ptr = reinterpret_cast<uint8_t*>(serialized);
    property->data.reserve(serialized_length);
    property->data.assign(ptr, ptr + serialized_length);
  }

  // Release serialized.
  if (serialized != NULL) {
    Status status2 = ReleaseSerializedProperty(property_key, serialized,
        serialized_length);
    if (status.ok()) {
      status = status2;
      SENSCORD_STATUS_TRACE(status);
    }
  }
  return status;
}

/**
 * @brief Set the property with key.
 * @param[in] (property_key) Key of property to set.
 * @param[in] (property) Location of property.
 * @return Status object.
 */
template <typename T>
Status Stream::SetProperty(const std::string& property_key,
                           const T* property) {
  StreamFunctionLock lock(lock_manager_, kStreamFunctionTypeComponent);
  if (!lock.GetStatus().ok()) {
    return SENSCORD_STATUS_TRACE(lock.GetStatus());
  }
  Status status;

  if (property != NULL) {
    // Create serialized property
    serialize::SerializedBuffer buffer;
    serialize::Encoder encoder(&buffer);
    status = encoder.Push(*property);
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      // Set property
      status = SetSerializedProperty(property_key, buffer.data(),
          buffer.size());
      SENSCORD_STATUS_TRACE(status);
    }
  } else {
    // Set property
    status = SetSerializedProperty(property_key, NULL, 0);
    SENSCORD_STATUS_TRACE(status);
  }

  return status;
}

/**
 * @brief Set the binary property with key.
 * @param[in] (property_key) Key of property to set.
 * @param[in] (property) Location of serialized property.
 * @return Status object.
 */
template <>
inline Status Stream::SetProperty(const std::string& property_key,
                                  const BinaryProperty* property) {
  StreamFunctionLock lock(lock_manager_, kStreamFunctionTypeComponent);
  if (!lock.GetStatus().ok()) {
    return SENSCORD_STATUS_TRACE(lock.GetStatus());
  }
  const void* input_ptr = NULL;
  size_t input_size = 0;
  if ((property != NULL) && !property->data.empty()) {
    input_ptr = &property->data[0];
    input_size = property->data.size();
  }
  // Set property
  Status status = SetSerializedProperty(property_key, input_ptr, input_size);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_SERIALIZE

}  // namespace senscord
#endif  // SENSCORD_STREAM_PRIVATE_H_
