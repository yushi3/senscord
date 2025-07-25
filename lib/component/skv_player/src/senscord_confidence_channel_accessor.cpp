/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/senscord_confidence_channel_accessor.h"
#include <map>
#include <string>
#include "senscord/logger.h"
#include "senscord/senscord.h"
#include "src/skv_player_common.h"
#include "src/skv_player_util.h"
#include "src/skv_play_library.h"
#include "senscord/serialize.h"
#include "senscord/develop/recorder_common.h"  // ChannelPropertiesForRecord
#include "senscord/develop/stream_source.h"

/**
 * @brief Initialize members of the component.
 * @param[in] (channel_property_skv_stream_info)
 * @param[in] (rawdata_skv_stream_info)
 * @param[in] (library) SkvPlayLibrary
 * @param[in] (util) StreamSourceUtility
 */
senscord::Status SenscordConfidenceChannelAccessor::Init(
    const SkvStreamInfo &channel_property_skv_stream_info,
    const SkvStreamInfo &rawdata_skv_stream_info,
    SkvPlayLibrary *library,
    senscord::StreamSourceUtility* util,
    senscord::MemoryAllocator* allocator,
    uint32_t channel_id) {
  channel_property_skv_stream_info_ = channel_property_skv_stream_info;
  rawdata_skv_stream_info_ = rawdata_skv_stream_info;
  library_ = library;
  util_ = util;
  allocator_ = allocator;
  channel_id_ = channel_id;

  return senscord::Status::OK();
}

/**
 * @brief Load stream data from disk to memory.
 * @return Status object.
 */
senscord::Status SenscordConfidenceChannelAccessor::CacheRawData() {
  // allocate the new memory
  senscord::Memory* raw_memory = NULL;
  senscord::Status status = allocator_->Allocate(
    rawdata_skv_stream_info_.frame_size, &raw_memory);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  // get first frame data to cache the memory
  status = library_->GetFrameData(
    rawdata_skv_stream_info_.id, 0,
    reinterpret_cast<uint8_t*>(raw_memory->GetAddress()));
  allocator_->Free(raw_memory);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  return senscord::Status::OK();
}

/**
 * @brief Get valid frame info
 * @param[in] (time) timestamp of frame.
 * @param[out] (rawdata_frame_index) valid rawdata frame index
 *                                   for a given time stamp.
 * @param[out] (property_frame_index) valid property frame index
 *                                    for a given time stamp.
 * @param[out] (frame_timestamp)  timestamp for a valid frame.
 * @return Status object.
 */
senscord::Status SenscordConfidenceChannelAccessor::GetValidFrameInfo(
  uint64_t current_time_stamp, uint32_t* rawdata_frame_index,
  uint32_t* property_frame_index, uint64_t* frame_timestamp) {
  uint64_t property_time_stamp_ = 0, rawdata_time_stamp_ = 0;
  uint32_t property_frame_index_ = 0, rawdata_frame_index_ = 0;

  // get frame index close to the given timestamp (property)
  senscord::Status status = library_->GetClosestFrameInfoByTimeStamp(
    channel_property_skv_stream_info_.id,
    current_time_stamp, &property_frame_index_, &property_time_stamp_);
  if (!status.ok()) {
  return SENSCORD_STATUS_TRACE(status);
  }

  // get frame index close to the given timestamp (rawdata)
  status = library_->GetClosestFrameInfoByTimeStamp(
    rawdata_skv_stream_info_.id,
    current_time_stamp, &rawdata_frame_index_, &rawdata_time_stamp_);
  if (!status.ok()) {
  return SENSCORD_STATUS_TRACE(status);
  }

  // confirm the equality of timestamp of property and rawdata
  if (property_time_stamp_ != rawdata_time_stamp_) {
    return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
      "fail to get valid frame index(different timestamp)");
  }

  // return frame info, if the time stamp is within the time interval
  if (property_time_stamp_ + frame_interval_ >= current_time_stamp) {
    // time >= property_time_stamp is guaranteed.
    *rawdata_frame_index  = rawdata_frame_index_;
    *property_frame_index = property_frame_index_;
    *frame_timestamp  = rawdata_time_stamp_;
    return senscord::Status::OK();
  } else {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseAborted,
      "fail to get valid frame index(specified time_stamp is not found)");
  }
}

/**
 * @brief Update channel property.
 * @param[in] (frame_index) index of the frame to be updated
 * @return Status object.
 */
senscord::Status SenscordConfidenceChannelAccessor::UpdateProperty(
    uint32_t frame_index) {
  // allocate the new memory
  std::vector<uint8_t> serialized_property(
    channel_property_skv_stream_info_.frame_size);

  // get frame data
  senscord::Status status = library_->GetFrameData(
    channel_property_skv_stream_info_.id, frame_index,
    serialized_property.data());
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // deserialize channel property data
  senscord::serialize::Decoder decoder(serialized_property.data(),
    channel_property_skv_stream_info_.frame_size);
  senscord::ChannelPropertiesForRecord deserialized_properties = {};
  status = decoder.Pop(deserialized_properties);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // update
  std::map<std::string, senscord::BinaryProperty>::iterator itr =
      deserialized_properties.properties.begin();
  std::map<std::string, senscord::BinaryProperty>::iterator end =
      deserialized_properties.properties.end();
  for (; itr != end; ++itr) {
    util_->UpdateChannelProperty(channel_id_, itr->first, &(itr->second));
  }

  return senscord::Status::OK();
}

/**
 * @brief Get all timestamps.
 * @param[out] (time_stamps) series of time stamp of frame
 * @return Status object.
 */
senscord::Status SenscordConfidenceChannelAccessor::GetAllFrameTimestamp(
  std::vector<uint64_t>* time_stamps) {
  std::vector<uint64_t> property_timestamp_vec;
  std::vector<uint64_t> rawdata_timestamp_vec;

  senscord::Status status = library_->GetAllFrameTimestamp(
    channel_property_skv_stream_info_.id,
    &property_timestamp_vec);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  status = library_->GetAllFrameTimestamp(
    rawdata_skv_stream_info_.id,
    &rawdata_timestamp_vec);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  if (property_timestamp_vec != rawdata_timestamp_vec) {
      return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
      "GetPlayTime Error: different timestamps%s");
  }
  *time_stamps = rawdata_timestamp_vec;
  return senscord::Status::OK();
}

/**
 * @brief Set valid time interval between frames.
 * @param[in] (interval) interval in microsecond
 * @return Status object.
 */
senscord::Status SenscordConfidenceChannelAccessor::SetFrameInterval(
  uint64_t interval) {
    if (interval <= 0) {
      return SENSCORD_STATUS_FAIL(
        kBlockName,
        senscord::Status::kCauseInvalidArgument,
        "SetFrameInterval Error:invald value");
    }
    frame_interval_ = interval;
    return senscord::Status::OK();
}

/**
 * @brief Reset frame index of channel accessor
 * @return Status object.
 */
senscord::Status SenscordConfidenceChannelAccessor::ResetFrameIndex() {
  rawdata_last_frame_index_  = 0;
  property_last_frame_index_ = 0;
  return senscord::Status::OK();
}

/**
 * @brief Get confidence raw data.
 * @param[in] (timestamp) timestamp of frame
 * @param[out] (channel_raw_data) channel rawdata demanded
 * @return Status object.
 */
senscord::Status SenscordConfidenceChannelAccessor::GetRawData(
  uint64_t time_stamp, senscord::ChannelRawData* channel_raw_data) {
    uint32_t rawdata_frame_index  = 0, property_frame_index = 0;
    uint64_t frame_timestamp = 0;

    // get valide frame index and its timestamp by specified time stamp
    senscord::Status status = GetValidFrameInfo(time_stamp,
    &rawdata_frame_index, &property_frame_index, &frame_timestamp);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    // exclude first time
    if (property_last_frame_index_ != 0) {
      // check if the valid frame index is equal to the last valid frame index
      if ( property_last_frame_index_ == property_frame_index ||
      rawdata_last_frame_index_ == rawdata_frame_index) {
        return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
          "fail to get valid frame index(specified time_stamp is not found)");
      }
    }

    // allocate the new memory
    senscord::Memory* raw_memory = NULL;
    status = allocator_->Allocate(
      rawdata_skv_stream_info_.frame_size, &raw_memory);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    // update property
    status = UpdateProperty(property_frame_index);
    if (!status.ok()) {
      allocator_->Free(raw_memory);
      return SENSCORD_STATUS_TRACE(status);
    }

    // get frame raw data
    status = library_->GetFrameData(
      rawdata_skv_stream_info_.id, rawdata_frame_index,
      reinterpret_cast<uint8_t*>(raw_memory->GetAddress()));
    if (!status.ok()) {
      allocator_->Free(raw_memory);
      return SENSCORD_STATUS_TRACE(status);
    }

    // return channel rawdata
    channel_raw_data->channel_id         = channel_id_;
    channel_raw_data->data_type          = senscord::kRawDataTypeConfidence;
    channel_raw_data->data_memory        = raw_memory;
    channel_raw_data->data_size          = rawdata_skv_stream_info_.frame_size;
    channel_raw_data->data_offset        = 0;
    channel_raw_data->captured_timestamp = frame_timestamp;

    // update the last frame indexes
    property_last_frame_index_ = property_frame_index;
    rawdata_last_frame_index_  = rawdata_frame_index;

    return senscord::Status::OK();
  }

/**
 * @brief Constructor.
 */
SenscordConfidenceChannelAccessor::SenscordConfidenceChannelAccessor():
    channel_property_skv_stream_info_(), rawdata_skv_stream_info_(),
    library_(NULL), util_(NULL), allocator_(NULL),
    rawdata_last_frame_index_(0), property_last_frame_index_(0) {
  frame_interval_ = kDefaultFrameRate;
}

/**
 * @brief Destructor.
 */
SenscordConfidenceChannelAccessor::~SenscordConfidenceChannelAccessor() {}
