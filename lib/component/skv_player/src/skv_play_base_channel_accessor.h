/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAY_BASE_CHANNEL_ACCESSOR_H_
#define LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAY_BASE_CHANNEL_ACCESSOR_H_

#include <stdint.h>
#include <string>
#include <vector>
#include "senscord/senscord.h"
#include "src/skv_player_util.h"
#include "src/skv_play_library.h"
#include "senscord/develop/stream_source.h"

/**
 * @brief Skv file manager interface class.
 */
class SkvPlayBaseChannelAccessor {
 public:
  /**
   * @brief Initialize members of the component.
   * @param[in] (channel_property_skv_stream_info)
   * @param[in] (rawdata_skv_stream_info)
   * @param[in] (library) SkvPlayLibrary
   * @param[in] (util) StreamSourceUtility
   * @param[in] (allocator) allocator
   * @param[in] (channel_id) channel_id
   */
  virtual senscord::Status Init(
    const SkvStreamInfo &channel_property_skv_stream_info,
    const SkvStreamInfo &rawdata_skv_stream_info,
    SkvPlayLibrary *library,
    senscord::StreamSourceUtility* util,
    senscord::MemoryAllocator* allocator,
    uint32_t channel_id) = 0;

  /**
   * @brief Load stream data from disk to memory.
   * @return Status object.
   */
  virtual senscord::Status CacheRawData() = 0;

  /**
   * @brief Get valid frame info
   * @param[in] (time) timestamp of frame.
   * @param[out] (rawdata_frame_index)      valid rawdata frame index 
   *                                        for a given time stamp.
   * @param[out] (property_frame_index)     valid property frame index 
   *                                        for a given time stamp.
   * @param[out] (frame_timestamp)  timestamp for a valid frame.
   * @return Status object.
   */
  virtual senscord::Status GetValidFrameInfo(
    uint64_t current_time_stamp, uint32_t* rawdata_frame_index,
    uint32_t* property_frame_index, uint64_t* frame_timestamp) = 0;

  /**
   * @brief Update property.
   * @param[in] (frame_index) index of the frame to be updated
   * @return Status object.
   */
  virtual senscord::Status UpdateProperty(
    uint32_t frame_index) = 0;

  /**
   * @brief Get all timestamps.
   * @param[out] (time_stamps) series of time stamp of frame
   * @return Status object.
   */
  virtual senscord::Status GetAllFrameTimestamp(
    std::vector<uint64_t>* time_stamps) = 0;

  /**
   * @brief Set valid time interval between frames.
   * @param[in] (interval) interval in microsecond
   * @return Status object.
   */
  virtual senscord::Status SetFrameInterval(uint64_t interval) = 0;

  /**
   * @brief Reset frame index of channel accessor
   * @return Status object.
   */
  virtual senscord::Status ResetFrameIndex() = 0;

  /**
   * @brief Get depth raw data.
   * @param[in] (timestamp) timestamp of frame
   * @param[out] (channel_raw_data) channel rawdata demanded
   * @return Status object.
   */
  virtual senscord::Status GetRawData(
    uint64_t time_stamp, senscord::ChannelRawData* channel_raw_data) = 0;

  /**
   * @brief Constructor.
   */
  SkvPlayBaseChannelAccessor() {}

  /**
   * @brief Destructor.
   */
  virtual ~SkvPlayBaseChannelAccessor() {}
};

#endif  // LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAY_BASE_CHANNEL_ACCESSOR_H_
