/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <string>
#include <vector>
#include <map>
#include <iterator>

#include "senscord/c_api/senscord_c_api.h"
#include "senscord/senscord.h"
#include "senscord/frame.h"
#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"
#include "frame/channel_core.h"
#include "c_api/c_common.h"
#include "c_api/converter_manager.h"

namespace c_api = senscord::c_api;
namespace osal = senscord::osal;

/**
 * @brief Get the channel ID.
 * @param[in]  channel     Channel handle.
 * @param[out] channel_id  Channel ID.
 * @return 0 is success or minus is failed (error code).
 * @see Channel::GetChannelId
 */
int32_t senscord_channel_get_channel_id(
    senscord_channel_t channel,
    uint32_t* channel_id) {
  SENSCORD_C_API_ARGUMENT_CHECK(channel == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(channel_id == NULL);
  senscord::ChannelCore* channel_ptr =
      c_api::ToPointer<senscord::ChannelCore*>(channel);
  *channel_id = channel_ptr->GetChannelId();
  return 0;
}

/**
 * @brief Get the raw data.
 * @param[in]  channel   Channel handle.
 * @param[out] raw_data  Raw data.
 * @return 0 is success or minus is failed (error code).
 * @see Channel::GetRawData
 */
int32_t senscord_channel_get_raw_data(
    senscord_channel_t channel,
    struct senscord_raw_data_t* raw_data) {
  SENSCORD_C_API_ARGUMENT_CHECK(channel == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(raw_data == NULL);
  senscord::ChannelCore* channel_ptr =
      c_api::ToPointer<senscord::ChannelCore*>(channel);
  senscord::Channel::RawData tmp_raw_data;
  senscord::Status status = channel_ptr->GetRawData(&tmp_raw_data);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  raw_data->address = tmp_raw_data.address;
  raw_data->size = tmp_raw_data.size;
  raw_data->timestamp = tmp_raw_data.timestamp;
  raw_data->type = channel_ptr->GetType().c_str();
  return 0;
}

/**
 * @brief Convert the raw data.
 * @param[in]  channel         Channel handle.
 * @param[out] output_rawdata  Pointer to the structure of the rawdata.
 * @param[out] output_size     Size of rawdata structure.
 * @return 0 is success or minus is failed (error code).
 */
int32_t senscord_channel_convert_rawdata(
    senscord_channel_t channel,
    void* output_rawdata,
    size_t output_size) {
#ifdef SENSCORD_SERIALIZE
  SENSCORD_C_API_ARGUMENT_CHECK(channel == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(output_rawdata == NULL);

  senscord::ChannelCore* channel_ptr =
      c_api::ToPointer<senscord::ChannelCore*>(channel);
  senscord::Channel::RawData tmp_raw_data = {};
  senscord::Status status = channel_ptr->GetRawData(&tmp_raw_data);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }

  // deserialize
  status = senscord::ConverterManager::GetInstance()->Deserialize(
      senscord::kConverterTypeRawData, tmp_raw_data.type,
      tmp_raw_data.address, tmp_raw_data.size, output_rawdata, output_size);
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
 * @brief Get the property related to this channel.
 * @param[in]  channel       Channel handle.
 * @param[in]  property_key  Key of property to get.
 * @param[out] value         Pointer to the structure of the property.
 * @param[in]  value_size    Size of property structure.
 * @return 0 is success or minus is failed (error code).
 * @see Channel::GetProperty
 */
int32_t senscord_channel_get_property(
    senscord_channel_t channel,
    const char* property_key,
    void* value,
    size_t value_size) {
  SENSCORD_C_API_ARGUMENT_CHECK(channel == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(property_key == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(value == NULL);

  senscord::Status status;
  const std::string key = property_key;
  senscord::ChannelCore* channel_ptr =
      c_api::ToPointer<senscord::ChannelCore*>(channel);
#ifdef SENSCORD_SERIALIZE
  senscord::BinaryProperty property;

  // get property
  status = channel_ptr->GetProperty(key, &property);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }

  // deserialize
  status = senscord::ConverterManager::GetInstance()->Deserialize(
      senscord::kConverterTypeProperty,
      key, property.data.data(), property.data.size(), value, value_size);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
#else
  // get property
  status = senscord::ConverterManager::GetInstance()->GetChannelProperty(
      channel_ptr, key, value, value_size);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
#endif  // SENSCORD_SERIALIZE
  return 0;
}

/**
 * @brief Get the serialized property related to this raw data.
 * @param[in]  channel       Channel handle.
 * @param[in]  property_key  Key of property to get.
 * @param[out] buffer        Buffer that stores output property values.
 * @param[in]  buffer_size   Buffer size.
 * @param[out] output_size   Size of output property. (optional)
 * @return 0 is success or minus is failed (error code).
 * @see Channel::GetProperty
 */
int32_t senscord_channel_get_serialized_property(
    senscord_channel_t channel,
    const char* property_key,
    void* buffer,
    size_t buffer_size,
    size_t* output_size) {
#ifdef SENSCORD_SERIALIZE
  SENSCORD_C_API_ARGUMENT_CHECK(channel == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(property_key == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(buffer == NULL);
  senscord::ChannelCore* channel_ptr =
      c_api::ToPointer<senscord::ChannelCore*>(channel);

  senscord::BinaryProperty binary = {};
  senscord::Status status = channel_ptr->GetProperty(
      property_key, &binary);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }

  *output_size = binary.data.size();
  if (*output_size > 0) {
    if (*output_size > buffer_size) {
      c_api::SetLastError(SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
          senscord::Status::kCauseInvalidArgument,
          "buffer_size(%" PRIu32 ") is smaller than output size(%" PRIuS ")",
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
 * @brief Get the count of stored property key on this channel.
 * @param[in]  channel  Channel handle.
 * @param[out] count    Count of stored property key.
 * @return 0 is success or minus is failed (error code).
 * @see Channel::GetPropertyList
 */
int32_t senscord_channel_get_property_count(
    senscord_channel_t channel,
    uint32_t* count) {
  SENSCORD_C_API_ARGUMENT_CHECK(channel == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(count == NULL);
  senscord::ChannelCore* channel_ptr =
      c_api::ToPointer<senscord::ChannelCore*>(channel);
  const std::map<std::string, uint32_t>& property_list =
      channel_ptr->GetPropertyList();
  *count = static_cast<uint32_t>(property_list.size());
  return 0;
}

/**
 * @brief Get the stored property key on this channel.
 * @param[in]  channel       Channel handle.
 * @param[in]  index         Index of stored property key list.
 *                           (min=0, max=count-1)
 * @param[out] property_key  Location of property key.
 * @return 0 is success or minus is failed (error code).
 * @see Channel::GetPropertyList
 */
int32_t senscord_channel_get_property_key(
    senscord_channel_t channel,
    uint32_t index,
    const char** property_key) {
  SENSCORD_C_API_ARGUMENT_CHECK(channel == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(property_key == NULL);
  senscord::ChannelCore* channel_ptr =
      c_api::ToPointer<senscord::ChannelCore*>(channel);
  const std::map<std::string, uint32_t>& property_list =
      channel_ptr->GetPropertyList();
  if (index >= property_list.size()) {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(
        senscord::kStatusBlockCore, senscord::Status::kCauseOutOfRange,
        "index(%" PRIu32 ") is larger than list.size(%" PRIuS ")",
        index, property_list.size()));
    return -1;
  }
  std::map<std::string, uint32_t>::const_iterator pos = property_list.begin();
  std::advance(pos, index);
  *property_key = pos->first.c_str();
  return 0;
}

/**
 * @brief Get the stored property key on this channel.
 * @param[in]  channel    Channel handle.
 * @param[in]  index      Index of stored property key list.
 *                        (min=0, max=count-1)
 * @param[out] buffer     Location to store the property key string.
 * @param[in,out] length  [in] Buffer size.
 *                        [out] String length. (not including '\0')
 * @return 0 is success or minus is failed (error code).
 * @see Channel::GetPropertyList
 */
int32_t senscord_channel_get_property_key_string(
    senscord_channel_t channel,
    uint32_t index,
    char* buffer,
    uint32_t* length) {
  SENSCORD_C_API_ARGUMENT_CHECK(channel == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(length == NULL);
  const char* property_key = NULL;
  int32_t ret = senscord_channel_get_property_key(
      channel, index, &property_key);
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
 * @brief Get the count of updated property key on this channel.
 * @param[in]  channel  Channel handle.
 * @param[out] count    Count of updated property key.
 * @return 0 is success or minus is failed (error code).
 * @see Channel::GetUpdatedPropertyList
 */
int32_t senscord_channel_get_updated_property_count(
    senscord_channel_t channel,
    uint32_t* count) {
  SENSCORD_C_API_ARGUMENT_CHECK(channel == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(count == NULL);
  senscord::ChannelCore* channel_ptr =
      c_api::ToPointer<senscord::ChannelCore*>(channel);
  const std::vector<std::string>& property_list =
      channel_ptr->GetUpdatedPropertyList();
  *count = static_cast<uint32_t>(property_list.size());
  return 0;
}

/**
 * @brief Get the updated property key on this channel.
 * @param[in]  channel       Channel handle.
 * @param[in]  index         Index of updated property key list.
 *                           (min=0, max=count-1)
 * @param[out] property_key  Location of property key.
 * @return 0 is success or minus is failed (error code).
 * @see Channel::GetUpdatedPropertyList
 */
int32_t senscord_channel_get_updated_property_key(
    senscord_channel_t channel,
    uint32_t index,
    const char** property_key) {
  SENSCORD_C_API_ARGUMENT_CHECK(channel == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(property_key == NULL);
  senscord::ChannelCore* channel_ptr =
      c_api::ToPointer<senscord::ChannelCore*>(channel);
  const std::vector<std::string>& property_list =
      channel_ptr->GetUpdatedPropertyList();
  if (index >= property_list.size()) {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(
        senscord::kStatusBlockCore, senscord::Status::kCauseOutOfRange,
        "index(%" PRIu32 ") is larger than list.size(%" PRIuS ")",
        index, property_list.size()));
    return -1;
  }
  *property_key = property_list[index].c_str();
  return 0;
}

/**
 * @brief Get the updated property key on this channel.
 * @param[in]  channel    Channel handle.
 * @param[in]  index      Index of updated property key list.
 *                        (min=0, max=count-1)
 * @param[out] buffer     Location to store the property key string.
 * @param[in,out] length  [in] Buffer size.
 *                        [out] String length. (not including '\0')
 * @return 0 is success or minus is failed (error code).
 * @see Channel::GetUpdatedPropertyList
 */
int32_t senscord_channel_get_updated_property_key_string(
    senscord_channel_t channel,
    uint32_t index,
    char* buffer,
    uint32_t* length) {
  SENSCORD_C_API_ARGUMENT_CHECK(channel == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(length == NULL);
  const char* property_key = NULL;
  int32_t ret = senscord_channel_get_updated_property_key(
      channel, index, &property_key);
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
