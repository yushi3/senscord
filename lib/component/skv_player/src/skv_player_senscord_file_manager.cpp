/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/skv_player_senscord_file_manager.h"
#include <map>
#include <utility>
#include <string>
#include <vector>
#include <algorithm>
#include "senscord/osal.h"
#include "senscord/logger.h"
#include "senscord/serialize.h"
#include "senscord/property_types.h"
#include "senscord/develop/recorder_common.h"  // StreamPropertiesForRecord
#include "src/skv_player_common.h"
#include "src/skv_play_base_channel_accessor.h"
#include "src/senscord_depth_channel_accessor.h"
#include "src/senscord_confidence_channel_accessor.h"
#include "src/senscord_pointcloud_channel_accessor.h"
#include "src/senscord_rawdata_channel_accessor.h"

/**
 * @brief Initialize members of the component.
 * @param[in] (frame_info) frame information
 * @param[in] (stream_property) stream property
 * @param[in] (library) SkvPlayLibrary
 * @param[in] (util) StreamSourceUtility
 */
void SkvPlaySensCordFileManager::Init(
    skv_player::SerializedStreamProperties* stream_property,
    SkvPlayLibrary* library,
    senscord::StreamSourceUtility* util,
    std::map<std::string, SkvStreamInfo>* stream_map,
    senscord::MemoryAllocator* allocator) {
  stream_property_ = stream_property;
  library_ = library;
  util_ = util;
  stream_map_ = *stream_map;
  allocator_ = allocator;
  return;
}

/**
 * @brief Load stream data from disk to memory.
 * @return Status object.
 */
senscord::Status SkvPlaySensCordFileManager::CacheRawData() {
  if (channel_accessor_list_.empty()) {
    return SENSCORD_STATUS_FAIL(kBlockName,
                            senscord::Status::kCauseNone,
                            "GetFrame: Channel Accessor does not exist.");
  }
  std::map<uint32_t, SkvPlayBaseChannelAccessor*>::iterator itr =
  channel_accessor_list_.begin();
  for (; itr != channel_accessor_list_.end(); ++itr) {
    senscord::Status status = itr->second->CacheRawData();
    if (!status.ok()) {
        SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
        continue;
    }
  }
  return senscord::Status::OK();
}

/**
 * @brief Get all timestamps.
 * @param[out] (time_stamps) series of time stamp of frame
 * @return Status object.
 */
senscord::Status SkvPlaySensCordFileManager::GetAllFrameTimestamp(
    std::vector<uint64_t>* time_stamps) {
  if (time_stamps == NULL) {
  return SENSCORD_STATUS_FAIL(kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: GetAllFrameTimestamp");
  }
  if (channel_accessor_list_.empty()) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseNone,
        "GetAllFrameTimestamp: Channel Accessor does not exist.");
  }
  std::map<uint32_t, SkvPlayBaseChannelAccessor*>::iterator itr =
  channel_accessor_list_.begin();
  std::vector<uint64_t> timestamp_vec;
  for (; itr != channel_accessor_list_.end(); ++itr) {
    std::vector<uint64_t> accessor_timestamps;
    std::vector<uint64_t> result_timestamps;
    senscord::Status status =
      itr->second->GetAllFrameTimestamp(&accessor_timestamps);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    std::set_union(timestamp_vec.begin(), timestamp_vec.end(),
    accessor_timestamps.begin(), accessor_timestamps.end(),
    back_inserter(result_timestamps));
    timestamp_vec = result_timestamps;
  }

  std::vector<uint64_t>::iterator time_itr = timestamp_vec.begin();
  for (std::vector<uint64_t>::iterator time_end = timestamp_vec.end();
       time_itr != time_end; ++time_itr) {
    // Convert micro sec to nano sec.
    time_stamps->push_back((*time_itr) * 1000);
  }
  return senscord::Status::OK();
}

/**
 * @brief Get frameinfo.
 * @param[in] (time) timestamp of frame.
 * @param[in] (frameinfo) timestamp of frame.
 * @return Status object.
 */
senscord::Status SkvPlaySensCordFileManager::GetFrame(
    uint64_t time_nano,
    senscord::FrameInfo *frameinfo) {
  if (frameinfo == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: GetFrame");
  }
  if (channel_accessor_list_.empty()) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseNone,
      "GetFrame: Channel Accessor does not exist.");
  }

  uint64_t time_micro = time_nano / 1000;  // nano sec to micro sec
  std::map<uint32_t, SkvPlayBaseChannelAccessor*>::iterator itr =
  channel_accessor_list_.begin();
  for (; itr != channel_accessor_list_.end(); ++itr) {
    senscord::ChannelRawData channel_raw_data = {};
    senscord::Status status =
      itr->second->GetRawData(time_micro, &channel_raw_data);
    if (!status.ok()) {
      SENSCORD_LOG_WARNING(
        "fail to get %" PRIu32 " channel raw data : %s",
        itr->first, status.ToString().c_str());
        continue;
    }
    frameinfo->channels.push_back(channel_raw_data);
  }
  return senscord::Status::OK();
}

/**
 * @brief Get stream property.
 * @param[in] (stream_property) stream property
 * @return Status object.
 */
senscord::Status SkvPlaySensCordFileManager::SetupStreamProperty(
    skv_player::SerializedStreamProperties* stream_properties) {
  if (stream_properties == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: SetupStreamProperty");
  }
  size_t buffer_size;
  senscord::Status status = library_->GetCustomBufferSize(
    kSkvStreamPropertyName, &buffer_size);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  std::vector<uint8_t> serialized_property(buffer_size);
  status = library_->GetCustomBufferData(
    kSkvStreamPropertyName, serialized_property.data());
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  senscord::serialize::Decoder decoder(
      serialized_property.data(), buffer_size);
  senscord::StreamPropertiesForRecord stream_properties_deserialized = {};
  status = decoder.Pop(stream_properties_deserialized);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Set the stream properties retrieved from the recording file.
  std::map<std::string, senscord::BinaryProperty>::iterator itr =
      stream_properties_deserialized.properties.begin();
  std::map<std::string, senscord::BinaryProperty>::iterator end =
      stream_properties_deserialized.properties.end();
  for (; itr != end; ++itr) {
    if (itr->first == senscord::kChannelInfoPropertyKey) {
      // Skip to setting from record file to set a fixed value.
      continue;
    }
    stream_properties->insert(std::make_pair(itr->first, itr->second));
  }

  // ChannelInfoProperty
  std::vector<std::string> target_names;
  target_names.push_back(kSkvStreamNameDepth);
  target_names.push_back(kSkvStreamNameDepthFloat);
  target_names.push_back(kSkvStreamNameConfidence);
  target_names.push_back(kSkvStreamNameFloatConfidence);
  target_names.push_back(kSkvStreamNamePointCloud);
  target_names.push_back(kSkvStreamNamePointCloudFloat);
  target_names.push_back(kSkvStreamNameRawData);
  target_names.push_back(kSkvStreamNameSecondRawData);
  // Get channel info property.
  senscord::ChannelInfoProperty channel_info = {};
  status = skv_player::GetChannelInfoPropertyFromSkvStream(
      target_names, stream_map_, &channel_info);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Serialize
  senscord::BinaryProperty binary = {};
  status = skv_player::EncodeDeserializedProperty(channel_info, &binary);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Apply
  stream_properties->insert(
      std::make_pair(senscord::kChannelInfoPropertyKey, binary));

  return senscord::Status::OK();
}

/**
 * @brief Set valid time interval between frames.
 * @param[in] (interval) interval in microsecond
 * @return Status object.
 */
senscord::Status SkvPlaySensCordFileManager::SetFrameInterval(
    uint64_t interval_nano) {
  if (channel_accessor_list_.empty()) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseNone,
      "SetFrameInterval: Channel Accessor does not exist.");
  }

  uint64_t interval_micro = interval_nano / 1000;  // nano to micro sec
  std::map<uint32_t, SkvPlayBaseChannelAccessor*>::iterator itr =
    channel_accessor_list_.begin();
  for (; itr != channel_accessor_list_.end(); ++itr) {
    senscord::Status status = itr->second->SetFrameInterval(interval_micro);
    if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
    }
  }
  return senscord::Status::OK();
}

/**
 * @brief Reset frame index of channel accessor
 */
void SkvPlaySensCordFileManager::ResetFrameIndex() {
  // reset frame index for all channel accessors.
  std::map<uint32_t, SkvPlayBaseChannelAccessor*>::iterator itr =
    channel_accessor_list_.begin();
  for (; itr != channel_accessor_list_.end(); ++itr) {
    itr->second->ResetFrameIndex();
  }
}

/**
 * @brief Setup channel accessor.
 * @return Status object.
 */
senscord::Status SkvPlaySensCordFileManager::SetupChannelAccessor() {
  // setup depth channel accessor
  std::map<std::string, SkvStreamInfo>::iterator found =
    stream_map_.find(kSkvPropertyNameDepth);
  if (found != stream_map_.end()) {
    if (stream_map_.find(kSkvStreamNameDepth) != stream_map_.end()) {
      SkvPlayBaseChannelAccessor* accessor =
        new SenscordDepthChannelAccessor();
      accessor->Init(found->second,
                    stream_map_[kSkvStreamNameDepth],
                    library_, util_,
                    allocator_,
                    skv_player::kChannelIdDepth);
      channel_accessor_list_.insert(
          std::make_pair(skv_player::kChannelIdDepth, accessor));
    } else {
      SENSCORD_LOG_INFO(
        "missing rawdata stream(%s)", kSkvStreamNameDepth);
    }
  }

  // setup depth-float channel accessor
  found = stream_map_.find(kSkvPropertyNameDepthFloat);
  if (found != stream_map_.end()) {
    if (stream_map_.find(kSkvStreamNameDepthFloat) != stream_map_.end()) {
      SkvPlayBaseChannelAccessor* accessor =
        new SenscordDepthChannelAccessor();
      accessor->Init(found->second,
                    stream_map_[kSkvStreamNameDepthFloat],
                    library_, util_,
                    allocator_,
                    skv_player::kChannelIdDepthFloat);
      channel_accessor_list_.insert(
        std::make_pair(skv_player::kChannelIdDepthFloat, accessor));
    } else {
      SENSCORD_LOG_INFO(
      "missing rawdata stream(%s)", kSkvStreamNameDepthFloat);
    }
  }

  // setup confidnece channel accessor
  found = stream_map_.find(kSkvPropertyNameConfidence);
  if (found != stream_map_.end()) {
    if (stream_map_.find(kSkvStreamNameConfidence) != stream_map_.end()) {
      SkvPlayBaseChannelAccessor* accessor =
        new SenscordConfidenceChannelAccessor();
      accessor->Init(found->second,
                    stream_map_[kSkvStreamNameConfidence],
                    library_, util_,
                    allocator_,
                    skv_player::kChannelIdConfidence);
      channel_accessor_list_.insert(
        std::make_pair(skv_player::kChannelIdConfidence, accessor));
    } else {
      SENSCORD_LOG_INFO(
        "missing rawdata stream(%s)", kSkvStreamNameConfidence);
    }
  }

  // setup confidnece-float channel accessor
  found = stream_map_.find(kSkvPropertyNameConfidenceFloat);
  if (found != stream_map_.end()) {
    if (stream_map_.find(kSkvStreamNameFloatConfidence) != stream_map_.end()) {
      SkvPlayBaseChannelAccessor* accessor =
        new SenscordConfidenceChannelAccessor();
      accessor->Init(found->second,
                    stream_map_[kSkvStreamNameFloatConfidence],
                    library_, util_,
                    allocator_,
                    skv_player::kChannelIdConfidenceFloat);
      channel_accessor_list_.insert(
        std::make_pair(skv_player::kChannelIdConfidenceFloat, accessor));
    } else {
      SENSCORD_LOG_INFO(
        "missing rawdata stream(%s)", kSkvStreamNameFloatConfidence);
    }
  }

  // setup pointcloud channel accessor
  found = stream_map_.find(kSkvPropertyNamePointCloud);
  if (found != stream_map_.end()) {
    if (stream_map_.find(kSkvStreamNamePointCloud) != stream_map_.end()) {
      SkvPlayBaseChannelAccessor* accessor =
        new SenscordPointCloudChannelAccessor();
      accessor->Init(found->second,
                    stream_map_[kSkvStreamNamePointCloud],
                    library_, util_,
                    allocator_,
                    skv_player::kChannelIdPointCloud);
      channel_accessor_list_.insert(
        std::make_pair(skv_player::kChannelIdPointCloud, accessor));
    } else {
      SENSCORD_LOG_INFO(
        "missing rawdata stream(%s)", kSkvStreamNamePointCloud);
    }
  }

  // setup pointcloud-float channel accessor
  found = stream_map_.find(kSkvPropertyNamePointCloudFloat);
  if (found != stream_map_.end()) {
    if (stream_map_.find(kSkvStreamNamePointCloudFloat) != stream_map_.end()) {
      SkvPlayBaseChannelAccessor* accessor =
        new SenscordPointCloudChannelAccessor();
      accessor->Init(found->second,
                    stream_map_[kSkvStreamNamePointCloudFloat],
                    library_, util_,
                    allocator_,
                    skv_player::kChannelIdPointCloudFloat);
      channel_accessor_list_.insert(
        std::make_pair(skv_player::kChannelIdPointCloudFloat, accessor));
    } else {
      SENSCORD_LOG_INFO(
        "missing rawdata stream(%s)", kSkvStreamNamePointCloudFloat);
    }
  }

  // setup rawdata channel accessor
  found = stream_map_.find(kSkvPropertyNameRawData);
  if (found != stream_map_.end()) {
    if (stream_map_.find(kSkvStreamNameRawData) != stream_map_.end()) {
      SkvPlayBaseChannelAccessor* accessor =
        new SenscordRawdataChannelAccessor();
      accessor->Init(found->second,
                    stream_map_[kSkvStreamNameRawData],
                    library_, util_,
                    allocator_,
                    skv_player::kChannelIdRawData);
      channel_accessor_list_.insert(
          std::make_pair(skv_player::kChannelIdRawData, accessor));
    } else {
      SENSCORD_LOG_INFO(
        "missing rawdata stream(%s)", kSkvStreamNameRawData);
    }
  }

  // setup second rawdata channel accessor
  found = stream_map_.find(kSkvPropertyNameSecondRawData);
  if (found != stream_map_.end()) {
    if (stream_map_.find(
        kSkvStreamNameSecondRawData) != stream_map_.end()) {
      SkvPlayBaseChannelAccessor* accessor =
        new SenscordRawdataChannelAccessor();
      accessor->Init(found->second,
                    stream_map_[kSkvStreamNameSecondRawData],
                    library_, util_,
                    allocator_,
                    skv_player::kChannelIdRawDataSecond);
      channel_accessor_list_.insert(
      std::make_pair(skv_player::kChannelIdRawDataSecond, accessor));
    } else {
      SENSCORD_LOG_INFO(
        "missing rawdata stream(%s)", kSkvStreamNameSecondRawData);
    }
  }
  if (channel_accessor_list_.empty()) {
    return SENSCORD_STATUS_FAIL(kBlockName,
                              senscord::Status::kCauseInvalidArgument,
                              "SetupChannelAccessor:Stream not found");
  }
  return senscord::Status::OK();
}

/**
 * @brief Delete channel accessor.
 * @return Status object.
 */
senscord::Status SkvPlaySensCordFileManager::DeleteChannelAccessor() {
  // release the remaining frames.
  while (!channel_accessor_list_.empty()) {
    std::map<uint32_t, SkvPlayBaseChannelAccessor*>::iterator itr =
    channel_accessor_list_.begin();
    delete itr->second;
    channel_accessor_list_.erase(itr);
  }
  return senscord::Status::OK();
}

/**
 * @brief Constructor.
 */
SkvPlaySensCordFileManager::SkvPlaySensCordFileManager():
  frame_info_(NULL), allocator_(NULL), stream_property_(NULL),
  library_(NULL), util_(NULL), stream_map_(), channel_accessor_list_() {
}

/**
 * @brief Destructor.
 */
SkvPlaySensCordFileManager::~SkvPlaySensCordFileManager() {
  DeleteChannelAccessor();
}
