/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_SKV_PLAYER_SRC_SENSCORD_RAWDATA_CHANNEL_ACCESSOR_H_
#define LIB_COMPONENT_SKV_PLAYER_SRC_SENSCORD_RAWDATA_CHANNEL_ACCESSOR_H_

#include <vector>
#include "src/skv_play_base_channel_accessor.h"
#include "senscord/senscord.h"
#include "src/skv_player_util.h"
#include "src/skv_play_library.h"
#include "senscord/develop/stream_source.h"

/**
 * @brief Skv file manager interface class.
 */
class SenscordRawdataChannelAccessor : public SkvPlayBaseChannelAccessor {
 public:
  /**
   * @brief Initialize members of the component.
   * @param[in] (channel_property_skv_stream_info) 
   * @param[in] (rawdata_skv_stream_info)
   * @param[in] (library) SkvPlayLibrary
   * @param[in] (util) StreamSourceUtility
   */
  virtual senscord::Status Init(
      const SkvStreamInfo &channel_property_skv_stream_info,
      const SkvStreamInfo &rawdata_skv_stream_info,
      SkvPlayLibrary *library,
      senscord::StreamSourceUtility* util,
      senscord::MemoryAllocator* allocator,
      uint32_t channel_id);

  /**
   * @brief Load stream data from disk to memory.
   * @return Status object.
   */
  virtual senscord::Status CacheRawData();

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
  virtual senscord::Status GetValidFrameInfo(
    uint64_t current_time_stamp, uint32_t* rawdata_frame_index,
    uint32_t* property_frame_index, uint64_t* frame_timestamp);

  /**
   * @brief Update channel property.
   * @param[in] (frame_index) index of the frame to be updated
   * @return Status object.
   */
  virtual senscord::Status UpdateProperty(
    uint32_t frame_index);

  /**
   * @brief Get all timestamps.
   * @param[out] (time_stamps) series of time stamp of frame
   * @return Status object.
   */
  virtual senscord::Status GetAllFrameTimestamp(
    std::vector<uint64_t>* time_stamps);

  /**
   * @brief Set valid time interval between frames.
   * @param[in] (interval) interval in microsecond
   * @return Status object.
   */
  virtual senscord::Status SetFrameInterval(uint64_t interval);

  /**
   * @brief Reset frame index of channel accessor
   * @return Status object.
   */
  virtual senscord::Status ResetFrameIndex();

  /**
   * @brief Get depth raw data.
   * @param[in] (timestamp) timestamp of frame
   * @param[out] (channel_raw_data) channel rawdata demanded
   * @return Status object.
   */
  virtual senscord::Status GetRawData(
    uint64_t time_stamp, senscord::ChannelRawData* channel_raw_data);

 private:
  SkvStreamInfo channel_property_skv_stream_info_;
  SkvStreamInfo rawdata_skv_stream_info_;
  SkvPlayLibrary *library_;
  senscord::StreamSourceUtility* util_;
  senscord::MemoryAllocator* allocator_;
  uint32_t rawdata_last_frame_index_;
  uint32_t property_last_frame_index_;
  uint64_t frame_interval_;
  uint32_t channel_id_;

 public:
  /**
   * @brief Constructor.
   */
  SenscordRawdataChannelAccessor();

  /**
   * @brief Destructor.
   */
  ~SenscordRawdataChannelAccessor();
};

#endif  // LIB_COMPONENT_SKV_PLAYER_SRC_SENSCORD_RAWDATA_CHANNEL_ACCESSOR_H_
