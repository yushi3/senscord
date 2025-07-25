/*
 * SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <inttypes.h>
#include <string>
#include <vector>
#include "senscord/c_api/senscord_c_api.h"
#include "senscord/senscord.h"
#include "senscord/osal.h"
#include "c_api/c_common.h"

namespace c_api = senscord::c_api;
namespace osal = senscord::osal;

namespace {

/**
 * @brief Gets the value of the specified key.
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] value  Location to store the value.
 * @return 0 is success or minus is failed.
 */
template<typename T>
int32_t GetArgumentValue(
    senscord_event_argument_t args, const char* key, T* value) {
#ifdef SENSCORD_STREAM_EVENT_ARGUMENT
  SENSCORD_C_API_ARGUMENT_CHECK(args == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(key == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(value == NULL);
  const senscord::EventArgument* event =
      c_api::ToPointer<senscord::EventArgument*>(args);
  senscord::Status status = event->Get(key, value);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
#else
  c_api::SetLastError(SENSCORD_STATUS_FAIL(
      senscord::kStatusBlockCore, senscord::Status::kCauseNotSupported,
      "feature is disabled. (SENSCORD_STREAM_EVENT_ARGUMENT=OFF)"));
  return -1;
#endif  // SENSCORD_STREAM_EVENT_ARGUMENT
}

}  // namespace

/**
 * @brief Gets the int8_t value of the specified key.
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] value  Location to store the value.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_getvalue_int8(
    senscord_event_argument_t args, const char* key,
    int8_t* value) {
  return GetArgumentValue(args, key, value);
}

/**
 * @brief Gets the int16_t value of the specified key.
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] value  Location to store the value.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_getvalue_int16(
    senscord_event_argument_t args, const char* key,
    int16_t* value) {
  return GetArgumentValue(args, key, value);
}

/**
 * @brief Gets the int32_t value of the specified key.
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] value  Location to store the value.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_getvalue_int32(
    senscord_event_argument_t args, const char* key,
    int32_t* value) {
  return GetArgumentValue(args, key, value);
}

/**
 * @brief Gets the int64_t value of the specified key.
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] value  Location to store the value.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_getvalue_int64(
    senscord_event_argument_t args, const char* key,
    int64_t* value) {
  return GetArgumentValue(args, key, value);
}

/**
 * @brief Gets the uint8_t value of the specified key.
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] value  Location to store the value.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_getvalue_uint8(
    senscord_event_argument_t args, const char* key,
    uint8_t* value) {
  return GetArgumentValue(args, key, value);
}

/**
 * @brief Gets the uint16_t value of the specified key.
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] value  Location to store the value.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_getvalue_uint16(
    senscord_event_argument_t args, const char* key,
    uint16_t* value) {
  return GetArgumentValue(args, key, value);
}

/**
 * @brief Gets the uint32_t value of the specified key.
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] value  Location to store the value.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_getvalue_uint32(
    senscord_event_argument_t args, const char* key,
    uint32_t* value) {
  return GetArgumentValue(args, key, value);
}

/**
 * @brief Gets the uint64_t value of the specified key.
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] value  Location to store the value.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_getvalue_uint64(
    senscord_event_argument_t args, const char* key,
    uint64_t* value) {
  return GetArgumentValue(args, key, value);
}

/**
 * @brief Gets the float value of the specified key.
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] value  Location to store the value.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_getvalue_float(
    senscord_event_argument_t args, const char* key,
    float* value) {
  return GetArgumentValue(args, key, value);
}

/**
 * @brief Gets the double value of the specified key.
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] value  Location to store the value.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_getvalue_double(
    senscord_event_argument_t args, const char* key,
    double* value) {
  return GetArgumentValue(args, key, value);
}

/**
 * @brief Gets the string of the specified key.
 *
 * If "buffer == NULL" and "length != NULL",
 * the required buffer size is stored in "length".
 *
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] buffer  Location to store the string.
 * @param[in,out] length  [in] Buffer size.
 *                        [out] String length. (not including '\0')
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_getvalue_string(
    senscord_event_argument_t args, const char* key,
    char* buffer, uint32_t* length) {
#ifdef SENSCORD_STREAM_EVENT_ARGUMENT
  SENSCORD_C_API_ARGUMENT_CHECK(length == NULL);
  std::string value;
  int32_t ret = GetArgumentValue(args, key, &value);
  if (ret != 0) {
    return ret;
  }
  senscord::Status status = c_api::StringToCharArray(
      value, buffer, length);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
#else
  c_api::SetLastError(SENSCORD_STATUS_FAIL(
      senscord::kStatusBlockCore, senscord::Status::kCauseNotSupported,
      "feature is disabled. (SENSCORD_STREAM_EVENT_ARGUMENT=OFF)"));
  return -1;
#endif  // SENSCORD_STREAM_EVENT_ARGUMENT
}

/**
 * @brief Gets the binary array of the specified key.
 *
 * If "buffer == NULL" and "length != NULL",
 * the required buffer size is stored in "length".
 *
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] buffer  Location to store the binary array.
 * @param[in,out] length  [in] Buffer size.
 *                        [out] Length of binary array.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_getvalue_binary(
    senscord_event_argument_t args, const char* key,
    void* buffer, uint32_t* length) {
#ifdef SENSCORD_STREAM_EVENT_ARGUMENT
  SENSCORD_C_API_ARGUMENT_CHECK(length == NULL);
  std::vector<uint8_t> value;
  int32_t ret = GetArgumentValue(args, key, &value);
  if (ret != 0) {
    return ret;
  }
  uint32_t buffer_size = *length;
  *length = static_cast<uint32_t>(value.size());
  if (buffer == NULL || buffer_size < *length) {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
        senscord::Status::kCauseOutOfRange, "Insufficient buffer length."));
    return -1;
  }
  osal::OSMemcpy(buffer, buffer_size, value.data(), *length);
  return 0;
#else
  c_api::SetLastError(SENSCORD_STATUS_FAIL(
      senscord::kStatusBlockCore, senscord::Status::kCauseNotSupported,
      "feature is disabled. (SENSCORD_STREAM_EVENT_ARGUMENT=OFF)"));
  return -1;
#endif  // SENSCORD_STREAM_EVENT_ARGUMENT
}

/**
 * @brief Gets the serialized binary array of the specified key.
 *
 * If "buffer == NULL" and "length != NULL",
 * the required buffer size is stored in "length".
 *
 * @param[in] args  Event argument handle.
 * @param[in] key  Argument key.
 * @param[out] buffer  Location to store the binary array.
 * @param[in,out] length  [in] Buffer size.
 *                        [out] Length of binary array.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_get_serialized_binary(
    senscord_event_argument_t args, const char* key,
    void* buffer, uint32_t* length) {
#ifdef SENSCORD_STREAM_EVENT_ARGUMENT
  SENSCORD_C_API_ARGUMENT_CHECK(args == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(key == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(length == NULL);
  const senscord::EventArgument* event =
      c_api::ToPointer<senscord::EventArgument*>(args);
  const std::vector<uint8_t>* binary = event->GetSerializedBinary(key);
  if (binary == NULL) {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
         senscord::Status::kCauseNotFound, "specified key was not found."));
    return -1;
  }
  uint32_t buffer_size = *length;
  *length = static_cast<uint32_t>(binary->size());
  if (buffer == NULL || buffer_size < *length) {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
        senscord::Status::kCauseOutOfRange, "Insufficient buffer length."));
    return -1;
  }
  osal::OSMemcpy(buffer, buffer_size, binary->data(), *length);
  return 0;
#else
  c_api::SetLastError(SENSCORD_STATUS_FAIL(
      senscord::kStatusBlockCore, senscord::Status::kCauseNotSupported,
      "feature is disabled. (SENSCORD_STREAM_EVENT_ARGUMENT=OFF)"));
  return -1;
#endif  // SENSCORD_STREAM_EVENT_ARGUMENT
}

/**
 * @brief Get the number of elements.
 * @param[in] args  Event argument handle.
 * @param[out] count  Location to store the number of elements.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_get_element_count(
    senscord_event_argument_t args, uint32_t* count) {
  SENSCORD_C_API_ARGUMENT_CHECK(args == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(count == NULL);
  const senscord::EventArgument* event =
      c_api::ToPointer<senscord::EventArgument*>(args);
  *count = static_cast<uint32_t>(event->GetSize());
  return 0;
}

/**
 * @brief Gets the key at the specified index.
 * @param[in] args  Event argument handle.
 * @param[in] index  Index (0 to elements-1)
 * @param[out] buffer  Location to store the string.
 * @param[in,out] length  [in] Buffer size.
 *                        [out] String length. (not including '\0')
 * @return 0 is success or minus is failed.
 */
int32_t senscord_event_argument_get_key_string(
    senscord_event_argument_t args, uint32_t index,
    char* buffer, uint32_t* length) {
#ifdef SENSCORD_STREAM_EVENT_ARGUMENT
  SENSCORD_C_API_ARGUMENT_CHECK(args == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(length == NULL);
  const char* key = senscord_event_argument_get_key(args, index);
  if (key == NULL) {
    return -1;
  }
  senscord::Status status = c_api::StringToCharArray(
      key, buffer, length);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
#else
  c_api::SetLastError(SENSCORD_STATUS_FAIL(
      senscord::kStatusBlockCore, senscord::Status::kCauseNotSupported,
      "feature is disabled. (SENSCORD_STREAM_EVENT_ARGUMENT=OFF)"));
  return -1;
#endif  // SENSCORD_STREAM_EVENT_ARGUMENT
}

/**
 * @brief Gets the key at the specified index.
 * @param[in] args  Event argument handle.
 * @param[in] index  Index (0 to elements-1)
 * @return String pointer. Returns NULL if invalid.
 */
const char* senscord_event_argument_get_key(
    senscord_event_argument_t args, uint32_t index) {
#ifdef SENSCORD_STREAM_EVENT_ARGUMENT
  if (args == 0) {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(
        senscord::kStatusBlockCore, senscord::Status::kCauseInvalidArgument,
        "args == 0"));
    return NULL;
  }
  const senscord::EventArgument* event =
      c_api::ToPointer<senscord::EventArgument*>(args);
  size_t count = event->GetSize();
  if (index >= count) {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(
        senscord::kStatusBlockCore, senscord::Status::kCauseOutOfRange,
        "index is invalid."));
    return NULL;
  }
  return event->GetKey(index).c_str();
#else
  c_api::SetLastError(SENSCORD_STATUS_FAIL(
      senscord::kStatusBlockCore, senscord::Status::kCauseNotSupported,
      "feature is disabled. (SENSCORD_STREAM_EVENT_ARGUMENT=OFF)"));
  return NULL;
#endif  // SENSCORD_STREAM_EVENT_ARGUMENT
}
