/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAYER_SENSCORD_FILE_MANAGER_H_
#define LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAYER_SENSCORD_FILE_MANAGER_H_

#include "src/skv_player_base_file_manager.h"
#include <map>
#include <vector>
#include <string>
#include "senscord/senscord.h"
#include "senscord/develop/stream_source.h"
#include "src/skv_play_library.h"
#include "src/skv_player_source.h"
#include "src/skv_player_util.h"
#include "src/skv_play_base_channel_accessor.h"

/**
 * @brief skv senscord file manager class.
 */
class SkvPlaySensCordFileManager : public SkvPlayBaseFileManager {
 public:
  /**
   * @brief Initialize menbers of the component.
   * @param[in] (stream_property) stream property
   * @param[in] (library) SkvPlayLibrary
   * @param[in] (util) StreamSourceUtility
   */
  virtual void Init(
    skv_player::SerializedStreamProperties *stream_property,
    SkvPlayLibrary *library,
    senscord::StreamSourceUtility *util,
    std::map<std::string, SkvStreamInfo> *stream_map,
    senscord::MemoryAllocator* allocator);

  /**
   * @brief Load stream data from disk to memory.
   * @return Status object.
   */
  virtual senscord::Status CacheRawData();


  /**
   * @brief Get frameinfo.
   * @param[in] (time) timestamp of frame.
   * @param[in] (frameinfo) timestamp of frame.
   * @return Status object.
   */
  virtual senscord::Status GetFrame(
    uint64_t time_nano,
    senscord::FrameInfo *frameinfo);

  /**
   * @brief Get all timestamps.
   * @param[out] (time_stamps) series of time stamp of frame
   * @return Status object.
   */
  virtual senscord::Status GetAllFrameTimestamp(
    std::vector<uint64_t>* time_stamps);

  /**
   * @brief Setup stream property.
   * @param[out] (stream_properties) serialized stream properties
   * @return Status object.
   */
  virtual senscord::Status SetupStreamProperty(
    skv_player::SerializedStreamProperties* stream_properties);

  /**
   * @brief Set valid time interval between frames.
   * @param[in] (interval) interval in microsecond
   * @return Status object.
   */
  virtual senscord::Status SetFrameInterval(uint64_t interval_nano);

  /**
   * @brief Reset frame index of channel accessor
   */
  virtual void ResetFrameIndex();

  /**
   * @brief Setup channel accessor.
   * @return Status object.
   */
  virtual senscord::Status SetupChannelAccessor();

  /**
   * @brief Delete channel accessor.
   * @return Status object.
   */
  virtual senscord::Status DeleteChannelAccessor();

  /**
   * @brief Constructor.
   */
  SkvPlaySensCordFileManager();

  /**
   *
   * @brief Destructor.
   */
  virtual ~SkvPlaySensCordFileManager();

 private:
  senscord::FrameInfo *frame_info_;
  senscord::MemoryAllocator* allocator_;
  skv_player::SerializedStreamProperties* stream_property_;
  SkvPlayLibrary *library_;
  senscord::StreamSourceUtility *util_;

 public:
  std::map<std::string, SkvStreamInfo> stream_map_;
  std::map<uint32_t, SkvPlayBaseChannelAccessor*> channel_accessor_list_;
};

#endif  // LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAYER_SENSCORD_FILE_MANAGER_H_
