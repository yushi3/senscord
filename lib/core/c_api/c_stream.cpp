/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "c_api/c_stream.h"

#include <inttypes.h>

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <utility>
#include <set>

#include "senscord/c_api/senscord_c_api.h"
#include "senscord/senscord.h"
#include "senscord/property_types.h"
#include "senscord/stream.h"
#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"
#include "c_api/c_common.h"
#include "c_api/converter_manager.h"
#include "stream/stream_core.h"
#include "util/autolock.h"

namespace c_api = senscord::c_api;
namespace util = senscord::util;
namespace osal = senscord::osal;

namespace {

/**
 * @brief Frame received callback function.
 */
void OnFrameReceived(
    senscord::Stream* stream, void* private_data) {
  c_api::FrameCallbackParam* param =
      reinterpret_cast<c_api::FrameCallbackParam*>(private_data);
  senscord_stream_t stream_handle = c_api::ToHandle(stream);
  param->callback(stream_handle, param->private_data);
}

/**
 * @brief Event received callback function.
 */
void OnEventReceived(
    senscord::Stream* stream, const std::string& event_type,
    const senscord::EventArgument& args, void* private_data) {
  c_api::EventCallbackParam* param =
      reinterpret_cast<c_api::EventCallbackParam*>(private_data);
  if (param->callback != NULL) {
    senscord_stream_t stream_handle = c_api::ToHandle(stream);
    senscord_event_argument_t event_handle = c_api::ToHandle(&args);
    param->callback(
        stream_handle, event_type.c_str(), event_handle, param->private_data);
  } else if (param->callback_old != NULL) {
    param->callback_old(event_type.c_str(), NULL, param->private_data);
  }
}

/**
 * @brief Register the callback for event receiving.
 * @param[in] stream        Stream handle.
 * @param[in] event_type    Event type to receive.
 * @param[in] callback      Function pointer. (new)
 * @param[in] callback_old  Function pointer. (old)
 * @param[in] private_data  Private data for callback.
 * @return 0 is success or minus is failed.
 */
int32_t RegisterEventCallback(
    senscord_stream_t stream,
    const char* event_type,
    const senscord_event_received_callback2 callback,
    const senscord_event_received_callback callback_old,
    void* private_data) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(event_type == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(
      callback == NULL && callback_old == NULL);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  c_api::ResourceEventCallback* event_callback =
      stream_ptr->GetResources()->Create<c_api::ResourceEventCallback>(
          c_api::kResourceEventCallback);

  c_api::EventCallbackParam* param = new c_api::EventCallbackParam;
  param->callback = callback;
  param->callback_old = callback_old;
  param->private_data = private_data;

  {
    util::AutoLock _lock(&event_callback->mutex);

    senscord::Status status = stream_ptr->RegisterEventCallback(
        event_type, OnEventReceived, param);
    if (!status.ok()) {
      c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
      delete param;
      return -1;
    }

    // Releases the old parameter and sets new parameter.
    std::pair<c_api::EventCallbackList::iterator, bool> ret =
        event_callback->list.insert(std::make_pair(event_type, param));
    if (!ret.second) {
      delete ret.first->second;
      ret.first->second = param;
    }
  }

  return 0;
}

/**
 * @brief Release the gotten frame.
 * @param[in] stream  Stream handle.
 * @param[in] frame   Received frame by senscord_stream_get_frame().
 * @param[in] used    Whether the raw data of the channel was referenced.
 * @return 0 is success or minus is failed (error code).
 */
int32_t ReleaseFrame(
    senscord_stream_t stream,
    senscord_frame_t frame,
    bool used) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  senscord::Frame* frame_ptr = c_api::ToPointer<senscord::Frame*>(frame);
  senscord::Status status;
  if (used) {
    status = stream_ptr->ReleaseFrame(frame_ptr);
  } else {
    status = stream_ptr->ReleaseFrameUnused(frame_ptr);
  }
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

}  // namespace

/**
 * @brief Start stream.
 * @param[in] stream  Stream handle.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::Start
 */
int32_t senscord_stream_start(
    senscord_stream_t stream) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  senscord::Status status = stream_ptr->Start();
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Stop stream.
 * @param[in] stream  Stream handle.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::Stop
 */
int32_t senscord_stream_stop(
    senscord_stream_t stream) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  senscord::Status status = stream_ptr->Stop();
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Get the received frame.
 * @param[in]  stream        Stream handle.
 * @param[out] frame         Location of received frame.
 * @param[in]  timeout_msec  Time of wait msec if no received.
 *                           0 is polling, minus is forever.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::GetFrame
 * @see senscord_TIMEOUT_POLLING, senscord_TIMEOUT_FOREVER
 */
int32_t senscord_stream_get_frame(
    senscord_stream_t stream,
    senscord_frame_t* frame,
    int32_t timeout_msec) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(frame == NULL);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  senscord::Frame* frame_ptr = NULL;
  senscord::Status status = stream_ptr->GetFrame(&frame_ptr, timeout_msec);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  *frame = c_api::ToHandle(frame_ptr);
  return 0;
}

/**
 * @brief Release the gotten frame.
 * @param[in] stream  Stream handle.
 * @param[in] frame   Received frame by senscord_stream_get_frame().
 * @return 0 is success or minus is failed (error code).
 * @see Stream::ReleaseFrame
 */
int32_t senscord_stream_release_frame(
    senscord_stream_t stream,
    senscord_frame_t frame) {
  return ReleaseFrame(stream, frame, true);
}

/**
 * @brief Release the gotten frame.
 *
 * Use this function if you do not refer to the raw data of the channel.
 *
 * @param[in] stream  Stream handle.
 * @param[in] frame   Received frame by senscord_stream_get_frame().
 * @return 0 is success or minus is failed (error code).
 * @see Stream::ReleaseFrameUnused
 */
int32_t senscord_stream_release_frame_unused(
    senscord_stream_t stream,
    senscord_frame_t frame) {
  return ReleaseFrame(stream, frame, false);
}

/**
 * @brief Clear frames have not gotten.
 * @param[in]  stream        Stream handle.
 * @param[out] frame_number  Number of cleared frames. (optional)
 * @return 0 is success or minus is failed (error code).
 * @see Stream::ClearFrames
 */
int32_t senscord_stream_clear_frames(
    senscord_stream_t stream,
    int32_t* frame_number) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  senscord::Status status = stream_ptr->ClearFrames(frame_number);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Get the property.
 * @param[in]  stream        Stream handle.
 * @param[in]  property_key  Key of property to get.
 * @param[in,out] value      Pointer to the structure of the property.
 * @param[in]  value_size    Size of property structure.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::GetProperty
 */
int32_t senscord_stream_get_property(
    senscord_stream_t stream,
    const char* property_key,
    void* value,
    size_t value_size) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(property_key == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(value == NULL);

  senscord::Status status;
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
#ifdef SENSCORD_SERIALIZE
  senscord::BinaryProperty property;

  // serialize
  status = senscord::ConverterManager::GetInstance()->Serialize(
      senscord::kConverterTypeProperty,
      property_key, value, value_size, &property.data);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }

  // get property
  status = stream_ptr->GetProperty(property_key, &property);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }

  // deserialize
  status = senscord::ConverterManager::GetInstance()->Deserialize(
      senscord::kConverterTypeProperty,
      property_key, property.data.data(), property.data.size(),
      value, value_size);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
#else
  status = senscord::ConverterManager::GetInstance()->GetStreamProperty(
      stream_ptr, property_key, value, value_size);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
#endif  // SENSCORD_SERIALIZE
  return 0;
}

/**
 * @brief Set the property with key.
 * @param[in] stream        Stream handle.
 * @param[in] property_key  Key of property to set.
 * @param[in] value         Pointer to the structure of the property.
 * @param[in] value_size    Size of property structure.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::SetProperty
 */
int32_t senscord_stream_set_property(
    senscord_stream_t stream,
    const char* property_key,
    const void* value,
    size_t value_size) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(property_key == NULL);

  senscord::Status status;
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
#ifdef SENSCORD_SERIALIZE
  senscord::BinaryProperty property;

  if (value != NULL) {
    // serialize
    status = senscord::ConverterManager::GetInstance()->Serialize(
        senscord::kConverterTypeProperty,
        property_key, value, value_size, &property.data);
    if (!status.ok()) {
      c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
      return -1;
    }
  }

  // set property
  status = stream_ptr->SetProperty(property_key, &property);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
#else
  status = senscord::ConverterManager::GetInstance()->SetStreamProperty(
      stream_ptr, property_key, value, value_size);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
#endif  // SENSCORD_SERIALIZE
  return 0;
}

/**
 * @brief Get the serialized property.
 * @param[in]  stream        Stream handle.
 * @param[in]  property_key  Key of property to get.
 * @param[out] buffer        Buffer that stores output property values.
 * @param[in]  buffer_size   Buffer size.
 * @param[out] output_size   Size of output property. (optional)
 * @return 0 is success or minus is failed (error code).
 * @see Stream::GetProperty
 */
int32_t senscord_stream_get_serialized_property(
    senscord_stream_t stream,
    const char* property_key,
    void* buffer,
    size_t buffer_size,
    size_t* output_size) {
#ifdef SENSCORD_SERIALIZE
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(property_key == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(buffer == NULL);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);

  uint8_t* ptr = reinterpret_cast<uint8_t*>(buffer);
  senscord::BinaryProperty binary = {};
  binary.data.reserve(buffer_size);
  binary.data.assign(ptr, ptr + buffer_size);

  senscord::Status status = stream_ptr->GetProperty(
      property_key, &binary);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }

  *output_size = binary.data.size();
  if (*output_size > 0) {
    if (*output_size > buffer_size) {
      c_api::SetLastError(SENSCORD_STATUS_FAIL(
          senscord::kStatusBlockCore, senscord::Status::kCauseOutOfRange,
          "buffer_size[%" PRIuS "] < output_size[%" PRIuS "]",
          buffer_size, *output_size));
      return -1;
    }
    osal::OSMemcpy(buffer, buffer_size, &binary.data[0], *output_size);
  }

  return 0;
#else
  c_api::SetLastError(SENSCORD_STATUS_FAIL(
      senscord::kStatusBlockCore, senscord::Status::kCauseNotSupported,
      "feature is disabled. (SENSCORD_SERIALIZE=OFF)"));
  return -1;
#endif  // SENSCORD_SERIALIZE
}

/**
 * @brief Set the property with key.
 * @param[in] stream        Stream handle.
 * @param[in] property_key  Key of property to set.
 * @param[in] buffer        Buffer that contains input property values.
 * @param[in] buffer_size   Buffer size.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::SetProperty
 */
int32_t senscord_stream_set_serialized_property(
    senscord_stream_t stream,
    const char* property_key,
    const void* buffer,
    size_t buffer_size) {
#ifdef SENSCORD_SERIALIZE
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(property_key == NULL);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);

  senscord::BinaryProperty binary = {};
  if (buffer != NULL) {
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buffer);
    binary.data.reserve(buffer_size);
    binary.data.assign(ptr, ptr + buffer_size);
  }

  senscord::Status status = stream_ptr->SetProperty(
      property_key, &binary);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
#else
  c_api::SetLastError(SENSCORD_STATUS_FAIL(
      senscord::kStatusBlockCore, senscord::Status::kCauseNotSupported,
      "feature is disabled. (SENSCORD_SERIALIZE=OFF)"));
  return -1;
#endif  // SENSCORD_SERIALIZE
}

/**
 * @brief Get the user data property.
 * @param[in]  stream       Stream handle.
 * @param[out] buffer       Buffer that stores output property values.
 * @param[in]  buffer_size  Buffer size.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::GetProperty
 */
int32_t senscord_stream_get_userdata_property(
    senscord_stream_t stream,
    void* buffer,
    size_t buffer_size) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(buffer == NULL);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);

  senscord::UserDataProperty property = {};

  senscord::Status status = stream_ptr->GetProperty(
      senscord::kUserDataPropertyKey, &property);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }

  // TODO: output property size.
  if (!property.data.empty()) {
    int32_t ret = osal::OSMemcpy(buffer, buffer_size,
        &property.data[0], property.data.size());
    if (ret < 0) {
      c_api::SetLastError(SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
          senscord::Status::kCauseUnknown,
          "memcpy failed. ret=%" PRIx32, ret));
      return -1;
    }
  } else {
    osal::OSMemset(buffer, 0, buffer_size);
  }

  return 0;
}

/**
 * @brief Set the user data property.
 * @param[in] stream       Stream handle.
 * @param[in] buffer       Buffer that contains input property values.
 * @param[in] buffer_size  Buffer size.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::SetProperty
 */
int32_t senscord_stream_set_userdata_property(
    senscord_stream_t stream,
    const void* buffer,
    size_t buffer_size) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  if (buffer == NULL) {
    senscord::Status status = stream_ptr->SetProperty(
        senscord::kUserDataPropertyKey,
        reinterpret_cast<senscord::UserDataProperty*>(NULL));
    if (!status.ok()) {
      c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
      return -1;
    }
    return 0;
  }

  senscord::UserDataProperty property = {};
  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buffer);
  property.data.reserve(buffer_size);
  property.data.assign(ptr, ptr + buffer_size);

  senscord::Status status = stream_ptr->SetProperty(
      senscord::kUserDataPropertyKey, &property);

  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Get the count of supported property key on this stream.
 * @param[in]  stream  Stream handle.
 * @param[out] count   Count of supported property key.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::GetPropertyList
 */
int32_t senscord_stream_get_property_count(
    senscord_stream_t stream,
    uint32_t* count) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(count == NULL);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  c_api::ResourcePropertyList* resource =
      stream_ptr->GetResources()->Create<c_api::ResourcePropertyList>(
          c_api::kResourcePropertyList);
  {
    util::AutoLock _lock(&resource->mutex);
    senscord::Status status = stream_ptr->GetPropertyList(
        &resource->property_list);
    if (!status.ok()) {
      c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
      return -1;
    }
    *count = static_cast<uint32_t>(resource->property_list.size());
  }
  return 0;
}

/**
 * @brief Get the supported property key on this stream.
 * @param[in]  stream        Stream handle.
 * @param[in]  index         Index of supported property key list.
 *                           (min=0, max=count-1)
 * @param[out] property_key  Location of property key.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::GetPropertyList
 */
int32_t senscord_stream_get_property_key(
    senscord_stream_t stream,
    uint32_t index,
    const char** property_key) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(property_key == NULL);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  c_api::ResourcePropertyList* resource =
      stream_ptr->GetResources()->Create<c_api::ResourcePropertyList>(
          c_api::kResourcePropertyList);
  {
    util::AutoLock _lock(&resource->mutex);
    if (resource->property_list.empty()) {
      senscord::Status status = stream_ptr->GetPropertyList(
          &resource->property_list);
      if (!status.ok()) {
        c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
        return -1;
      }
    }
    if (index >= resource->property_list.size()) {
      c_api::SetLastError(SENSCORD_STATUS_FAIL(
          senscord::kStatusBlockCore, senscord::Status::kCauseOutOfRange,
          "index(%" PRIu32 ") is larger than list.size(%" PRIuS ")",
          index, resource->property_list.size()));
      return -1;
    }
    *property_key = resource->property_list[index].c_str();
  }
  return 0;
}

/**
 * @brief Get the supported property key on this stream.
 * @param[in]  stream     Stream handle.
 * @param[in]  index      Index of supported property key list.
 *                        (min=0, max=count-1)
 * @param[out] buffer     Location to store the property key string.
 * @param[in,out] length  [in] Buffer size.
 *                        [out] String length. (not including '\0')
 * @return 0 is success or minus is failed (error code).
 * @see Stream::GetPropertyList
 */
int32_t senscord_stream_get_property_key_string(
    senscord_stream_t stream,
    uint32_t index,
    char* buffer,
    uint32_t* length) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(length == NULL);
  const char* property_key = NULL;
  int32_t ret = senscord_stream_get_property_key(stream, index, &property_key);
  if (ret == 0) {
    senscord::Status status = c_api::StringToCharArray(
        property_key, buffer, length);
    if (!status.ok()) {
      c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
      ret = -1;
    }
  }
  return ret;
}

/**
 * @brief Lock to access properties.
 * @param[in] stream        Stream handle.
 * @param[in] timeout_msec  Time of wait msec if locked already.
 *                          0 is polling, minus is forever.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::LockProperty
 * @see senscord_TIMEOUT_POLLING, senscord_TIMEOUT_FOREVER
 */
int32_t senscord_stream_lock_property(
    senscord_stream_t stream,
    int32_t timeout_msec) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  senscord::Status status = stream_ptr->LockProperty(timeout_msec);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Lock to access properties (specify key).
 * @param[in] stream        Stream handle.
 * @param[in] keys          Property keys for lock targets.
 * @param[in] count         Count of property keys.
 * @param[in] timeout_msec  Time of wait msec if locked already.
 *                          0 is polling, minus is forever.
 * @param[out] lock_resource  Locked properties resource.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::LockProperty
 * @see senscord_TIMEOUT_POLLING, senscord_TIMEOUT_FOREVER
 */
int32_t senscord_stream_lock_property_with_key(
    senscord_stream_t stream,
    const char* keys[],
    uint32_t count,
    int32_t timeout_msec,
    senscord_property_lock_resource_t* lock_resource) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(lock_resource == NULL);
  std::set<std::string> tmp_keys;
  if (keys != NULL && count > 0) {
    for (uint32_t i = 0; i < count; ++i) {
      if (keys[i] != NULL) {
        tmp_keys.insert(keys[i]);
      }
    }
  }
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  senscord::PropertyLockResource* handle_ptr = NULL;
  senscord::Status status = stream_ptr->LockProperty(
      tmp_keys, timeout_msec, &handle_ptr);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  *lock_resource = c_api::ToHandle(handle_ptr);
  return 0;
}

/**
 * @brief Unlock to access properties.
 * @param[in] stream  Stream handle.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::UnlockProperty
 */
int32_t senscord_stream_unlock_property(
    senscord_stream_t stream) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  senscord::Status status = stream_ptr->UnlockProperty();
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Unlock to access properties (specify resource).
 * @param[in] stream  Stream handle.
 * @param[in] lock_resource  Locked properties resource.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::UnlockProperty
 */
int32_t senscord_stream_unlock_property_by_resource(
    senscord_stream_t stream,
    senscord_property_lock_resource_t lock_resource) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(lock_resource == 0);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  senscord::PropertyLockResource* handle_ptr =
      c_api::ToPointer<senscord::PropertyLockResource*>(lock_resource);
  senscord::Status status = stream_ptr->UnlockProperty(handle_ptr);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  return 0;
}

/**
 * @brief Register the callback for frame reached.
 * @param[in] stream        Stream handle.
 * @param[in] callback      Function pointer.
 * @param[in] private_data  Private data for callback.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::RegisterFrameCallback
 */
int32_t senscord_stream_register_frame_callback(
    senscord_stream_t stream,
    const senscord_frame_received_callback callback,
    void* private_data) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(callback == NULL);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  c_api::ResourceFrameCallback* frame_callback =
      stream_ptr->GetResources()->Create<c_api::ResourceFrameCallback>(
          c_api::kResourceFrameCallback);

  c_api::FrameCallbackParam* param = new c_api::FrameCallbackParam;
  param->callback = callback;
  param->private_data = private_data;

  {
    util::AutoLock _lock(&frame_callback->mutex);

    senscord::Status status = stream_ptr->RegisterFrameCallback(
        OnFrameReceived, param);
    if (!status.ok()) {
      c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
      delete param;
      return -1;
    }

    // Releases the old parameter and sets new parameter.
    delete frame_callback->param;
    frame_callback->param = param;
  }

  return 0;
}

/**
 * @brief Unregister the callback for frame reached.
 * @param[in] stream  Stream handle.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::UnregisterFrameCallback
 */
int32_t senscord_stream_unregister_frame_callback(
    senscord_stream_t stream) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  c_api::ResourceFrameCallback* frame_callback =
      stream_ptr->GetResources()->Get<c_api::ResourceFrameCallback>(
          c_api::kResourceFrameCallback);

  if (frame_callback != NULL) {
    util::AutoLock _lock(&frame_callback->mutex);

    senscord::Status status = stream_ptr->UnregisterFrameCallback();
    if (!status.ok()) {
      c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
      return -1;
    }
  }

  // Releases the registered parameter.
  stream_ptr->GetResources()->Release(c_api::kResourceFrameCallback);

  return 0;
}

/**
 * @deprecated
 * @brief Register the callback for event receiving.
 * @param[in] stream        Stream handle.
 * @param[in] event_type    Event type to receive.
 * @param[in] callback      Function pointer.
 * @param[in] private_data  Private data for callback.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::RegisterEventCallback
 */
int32_t senscord_stream_register_event_callback(
    senscord_stream_t stream,
    const char* event_type,
    const senscord_event_received_callback callback,
    void* private_data) {
  return RegisterEventCallback(
      stream, event_type, NULL, callback, private_data);
}

/**
 * @brief Register the callback for event receiving.
 * @param[in] stream        Stream handle.
 * @param[in] event_type    Event type to receive.
 * @param[in] callback      Function pointer.
 * @param[in] private_data  Private data for callback.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::RegisterEventCallback
 */
int32_t senscord_stream_register_event_callback2(
    senscord_stream_t stream,
    const char* event_type,
    const senscord_event_received_callback2 callback,
    void* private_data) {
  return RegisterEventCallback(
      stream, event_type, callback, NULL, private_data);
}

/**
 * @brief Unregister the event callback.
 * @param[in] stream      Stream handle.
 * @param[in] event_type  Event type to receive.
 * @return 0 is success or minus is failed (error code).
 * @see Stream::UnregisterEventCallback
 */
int32_t senscord_stream_unregister_event_callback(
    senscord_stream_t stream,
    const char* event_type) {
  SENSCORD_C_API_ARGUMENT_CHECK(stream == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(event_type == NULL);
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  c_api::ResourceEventCallback* event_callback =
      stream_ptr->GetResources()->Get<c_api::ResourceEventCallback>(
          c_api::kResourceEventCallback);

  bool list_empty = false;
  if (event_callback != NULL) {
    util::AutoLock _lock(&event_callback->mutex);

    senscord::Status status =
        stream_ptr->UnregisterEventCallback(event_type);
    if (!status.ok()) {
      c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
      return -1;
    }

    // Releases the registered parameter.
    c_api::EventCallbackList::iterator itr =
        event_callback->list.find(event_type);
    if (itr != event_callback->list.end()) {
      delete itr->second;
      event_callback->list.erase(itr);
    }
    list_empty = event_callback->list.empty();
  } else {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(
        senscord::kStatusBlockCore, senscord::Status::kCauseNotFound,
        "no registered event type: %s", event_type));
    return -1;
  }

  if (list_empty) {
    stream_ptr->GetResources()->Release(c_api::kResourceEventCallback);
  }

  return 0;
}
