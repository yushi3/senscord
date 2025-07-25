/*
 * SPDX-FileCopyrightText: 2018-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "messagepack/record_property.h"

namespace senscord {

/**
 * @brief Received Property MessagePack to SensCord BinaryProperty.
 * @param[in] (src) Serialized javascript property data.
 * @param[out] (dst) Serialized property data.
 * @return Status object.
 */
Status RecordPropertyComponent::MsgPackToBinary(
    std::vector<uint8_t>* src, std::vector<uint8_t>* dst) {

  RecordPropertyJS prop_js {};
  Status status = DeserializeMsg(
      reinterpret_cast<char*>(src->data()), src->size(), &prop_js);
  if (!status.ok()) {
    return status;
  }

  if (prop_js.formats_num != prop_js.formats_channel_ids.size() ||
      prop_js.formats_num != prop_js.formats_format_names.size()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseDataLoss, "invalid RecordProperty.formats.");
  }
  if (prop_js.name_rules_num != prop_js.name_rules_directory_types.size() ||
      prop_js.name_rules_num != prop_js.name_rules_formats.size()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseDataLoss, "invalid RecordProperty.name_rules.");
  }

  RecordProperty prop {};
  prop.enabled = prop_js.enabled;
  prop.path = prop_js.path;
  prop.buffer_num = prop_js.buffer_num;
  prop.count = prop_js.count;
  for (size_t i = 0; i < prop_js.formats_num; ++i) {
    prop.formats[prop_js.formats_channel_ids[i]] =
        prop_js.formats_format_names[i];
  }
  for (size_t i = 0; i < prop_js.name_rules_num; ++i) {
    prop.name_rules[prop_js.name_rules_directory_types[i]] =
        prop_js.name_rules_formats[i];
  }

  return PropertyToBinary(prop, dst);
}

/**
 * @brief SensCord BinaryProperty to Property MessagePack for send data.
 * @param[in] (src) Serialized property data.
 * @param[out] (dst) Serialized javascript property data.
 * @return Status object.
 */
Status RecordPropertyComponent::BinaryToMsgPack(
    std::vector<uint8_t>* src, std::vector<uint8_t>* dst) {

  RecordProperty prop {};
  Status status = BinaryToProperty(src, prop);
  if (status.ok()) {
    RecordPropertyJS prop_js {};
    prop_js.enabled = prop.enabled;
    prop_js.path = prop.path;
    prop_js.buffer_num = prop.buffer_num;
    prop_js.count = prop.count;
    prop_js.formats_num = static_cast<uint32_t>(prop.formats.size());
    for (auto it = prop.formats.begin(); it != prop.formats.end(); ++it) {
      prop_js.formats_channel_ids.push_back(it->first);
      prop_js.formats_format_names.push_back(it->second);
    }
    prop_js.name_rules_num = static_cast<uint32_t>(prop.name_rules.size());
    for (auto it = prop.name_rules.begin(); it != prop.name_rules.end(); ++it) {
      prop_js.name_rules_directory_types.push_back(it->first);
      prop_js.name_rules_formats.push_back(it->second);
    }
    status = SerializeMessagePack(status, &prop_js, dst);
  }
  return SENSCORD_STATUS_TRACE(status);
}

}  // namespace senscord
