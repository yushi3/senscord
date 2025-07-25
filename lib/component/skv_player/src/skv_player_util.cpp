/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/skv_player_util.h"
#include <utility>  // std::make_pair
#include "src/skv_player_common.h"

namespace skv_player {
/**
 * @brief Get channel information from channel id.
 * @param[in] (channel_id) The id of channel.
 * @param[out] (info) channel information.
 * @return Status object.
 */
senscord::Status GetChannelInfoParameter(
    uint32_t channel_id,
    senscord::ChannelInfo* info) {
  if (info == NULL) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidArgument, "invalid parameter");
  }

  switch (channel_id) {
    case kChannelIdDepth:
    case kChannelIdDepthFloat:
      // depth channel
      // depth(float) channel
      info->raw_data_type = senscord::kRawDataTypeDepth;
      info->description = kDefaultDescriptionDepth;
      break;
    case kChannelIdConfidence:
    case kChannelIdConfidenceFloat:
      // confidence channel
      info->raw_data_type = senscord::kRawDataTypeConfidence;
      info->description = kDefaultDescriptionConfidence;
      break;
    case kChannelIdPointCloud:
    case kChannelIdPointCloudFloat:
      // point-cloud channel
      // point-cloud(float) channel
      info->raw_data_type = senscord::kRawDataTypePointCloud;
      info->description = kDefaultDescriptionPointCloud;
      break;
    case kChannelIdRawData:
      // rawdata(1st) channel
      info->raw_data_type = senscord::kRawDataTypeImage;
      info->description = kDefaultDescriptionRawDataFirst;
      break;
    case kChannelIdRawDataSecond:
      // rawdata(2nd) channel
      info->raw_data_type = senscord::kRawDataTypeImage;
      info->description = kDefaultDescriptionRawDataSecond;
      break;
    default:
      return SENSCORD_STATUS_FAIL(kBlockName,
          senscord::Status::kCauseInvalidArgument,
          "invalid parameter: channel_id = %" PRIu32, channel_id);
  }

  return senscord::Status::OK();
}

/**
 * @brief Get channel info property from skv stream.
 * @param[in] (target_name) The name of target skv stream.
 * @param[in] (skv_stream_list) The information of skv stream.
 * @param[out] (prop) channel info property.
 * @return Status object.
 */
senscord::Status GetChannelInfoPropertyFromSkvStream(
    const std::vector<std::string>& target_names,
    const std::map<std::string, SkvStreamInfo>& skv_stream_list,
    senscord::ChannelInfoProperty* prop) {
  if (prop == NULL) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidArgument, "invalid parameter");
  }

  std::vector<std::string>::const_iterator itr = target_names.begin();
  std::vector<std::string>::const_iterator end = target_names.end();
  for (; itr != end; ++itr) {
    if (skv_stream_list.find(*itr) == skv_stream_list.end()) {
      // not found
      continue;
    }

    // Assign channel id from skv stream name.
    uint32_t channel_id = 0;
    if ((*itr == kSkvStreamNameDepth) ||
        (*itr == kSkvStreamNameIntZ)) {
      channel_id = kChannelIdDepth;
    } else if ((*itr == kSkvStreamNameConfidence) ||
               (*itr == kSkvStreamNameIntConfidence)) {
      channel_id = kChannelIdConfidence;
    } else if ((*itr == kSkvStreamNamePointCloud) ||
               (*itr == kSkvStreamNameIntPointCloud)) {
      channel_id = kChannelIdPointCloud;
    } else if ((*itr == kSkvStreamNameDepthFloat) ||
               (*itr == kSkvStreamNameFloatZ)) {
      channel_id = kChannelIdDepthFloat;
    } else if (*itr == kSkvStreamNameFloatConfidence) {
      channel_id = kChannelIdConfidenceFloat;
    } else if ((*itr == kSkvStreamNamePointCloudFloat) ||
               (*itr == kSkvStreamNameFloatPointCloud)) {
      channel_id = kChannelIdPointCloudFloat;
    } else if ((*itr == kSkvStreamNameRawData) ||
               (*itr == kSkvStreamNameTofRawData)) {
      channel_id = kChannelIdRawData;
    } else if (*itr == kSkvStreamNameSecondRawData) {
      channel_id = kChannelIdRawDataSecond;
    } else {
      return SENSCORD_STATUS_FAIL(kBlockName,
          senscord::Status::kCauseInvalidArgument,
          "Unexpected stream name: %s", itr->c_str());
    }

    // Get channel information.
    senscord::ChannelInfo info = {};
    senscord::Status status = GetChannelInfoParameter(channel_id, &info);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    // append channel info
    prop->channels.insert(std::make_pair(channel_id, info));
  }

  if (prop->channels.size() == 0) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseNotFound,
        "Channel information is not found.");
  }

  return senscord::Status::OK();
}
}  // namespace skv_player
