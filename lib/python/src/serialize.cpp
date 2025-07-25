/*
 * SPDX-FileCopyrightText: 2021-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include "senscord/senscord.h"
#include "senscord/osal.h"
#include "senscord/noncopyable.h"
#include "frame/channel_core.h"
#include "c_api/c_common.h"
#include "c_api/c_stream.h"
#include "python_gil.h"

namespace c_api = senscord::c_api;
namespace osal = senscord::osal;

extern "C" {

/**
 * @brief Get the serialized property.
 * @param[in]  stream        Stream handle.
 * @param[in]  property_key  Key of property to get.
 * @param[in,out] bytearray  Buffer that stores output property values.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::GetProperty
 */
int32_t senscord_py_stream_get_serialized_property(
    senscord_stream_t stream,
    const char* property_key,
    PyObject* bytearray) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(property_key == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(bytearray == NULL);
  senscord::BinaryProperty binary = {};

  {
    senscord::PythonGlobalInterpreterLock py_lock;

    if (!PyByteArray_Check(bytearray)) {
      c_api::SetLastError(SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
          senscord::Status::kCauseInvalidArgument,
          "bytearray is not PyByteArrayObject"));
      return -1;
    }

    uint8_t* ptr = reinterpret_cast<uint8_t*>(PyByteArray_AsString(bytearray));
    size_t input_size = static_cast<size_t>(PyByteArray_Size(bytearray));
    binary.data.reserve(input_size);
    binary.data.assign(ptr, ptr + input_size);
  }

  senscord::Stream* stream_ptr = c_api::ToPointer<senscord::Stream*>(stream);
  senscord::Status status = stream_ptr->GetProperty(property_key, &binary);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }

  size_t size = binary.data.size();

  {
    senscord::PythonGlobalInterpreterLock py_lock;

    int32_t ret = PyByteArray_Resize(bytearray, static_cast<Py_ssize_t>(size));
    if (ret < 0) {
      c_api::SetLastError(SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
          senscord::Status::kCauseAborted,
          "failed to resize bytearray: ret=%" PRId32, ret));
      return -1;
    }

    if (size > 0) {
      char* ptr = PyByteArray_AsString(bytearray);
      osal::OSMemcpy(ptr, size, &binary.data[0], size);
    }
  }

  return 0;
}

/**
 * @brief Get the serialized property related to this raw data.
 * @param[in]  channel       Channel handle.
 * @param[in]  property_key  Key of property to get.
 * @param[out] bytearray     Buffer that stores output property values.
 * @return 0 is success or minus is failed (error code).
 * @see Channel::GetProperty
 */
int32_t senscord_py_channel_get_serialized_property(
    senscord_channel_t channel,
    const char* property_key,
    PyObject* bytearray) {
  SENSCORD_C_API_ARGUMENT_CHECK(channel == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(property_key == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(bytearray == NULL);

  {
    senscord::PythonGlobalInterpreterLock py_lock;

    if (!PyByteArray_Check(bytearray)) {
      c_api::SetLastError(SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
          senscord::Status::kCauseInvalidArgument,
          "bytearray is not PyByteArrayObject"));
      return -1;
    }
  }

  senscord::Channel* channel_ptr =
      c_api::ToPointer<senscord::Channel*>(channel);

  senscord::BinaryProperty binary = {};
  senscord::Status status = channel_ptr->GetProperty(
      property_key, &binary);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }

  size_t size = binary.data.size();

  {
    senscord::PythonGlobalInterpreterLock py_lock;

    int32_t ret = PyByteArray_Resize(bytearray, static_cast<Py_ssize_t>(size));
    if (ret < 0) {
      c_api::SetLastError(SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
          senscord::Status::kCauseAborted,
          "failed to resize bytearray: ret=%" PRId32, ret));
      return -1;
    }

    if (size > 0) {
      char* ptr = PyByteArray_AsString(bytearray);
      osal::OSMemcpy(ptr, size, &binary.data[0], size);
    }
  }

  return 0;
}

}  // extern "C"
