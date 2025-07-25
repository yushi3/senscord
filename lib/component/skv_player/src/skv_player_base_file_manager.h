/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAYER_BASE_FILE_MANAGER_H_
#define LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAYER_BASE_FILE_MANAGER_H_

#include <stdint.h>
#include <map>
#include <string>
#include <vector>
#include "senscord/senscord.h"
#include "src/skv_player_util.h"
#include "src/skv_play_library.h"
#include "senscord/develop/stream_source.h"

/**
 * @brief Skv file manager interface class.
 */
class SkvPlayBaseFileManager {
 public:
  /**
   * @brief Initialize members of the component.
   * @param[in] (frame_info) frame information
   * @param[in] (stream_property) stream property
   * @param[in] (library) SkvPlayLibrary
   * @param[in] (util) StreamSourceUtility
   */
  virtual void Init(
    skv_player::SerializedStreamProperties* stream_property,
    SkvPlayLibrary* library,
    senscord::StreamSourceUtility* util,
    std::map<std::string, SkvStreamInfo>* stream_map,
    senscord::MemoryAllocator* allocator) = 0;

  /**
   * @brief Load stream data from disk to memory.
   * @return Status object.
   */
  virtual senscord::Status CacheRawData() = 0;

  /**
   * @brief Get frameinfo.
   * @param[in] (time) timestamp of frame.
   * @param[in] (frameinfo) timestamp of frame.
   * @return Status object.
   */
  virtual senscord::Status GetFrame(
    uint64_t time,
    senscord::FrameInfo *frameinfo) = 0;

  /**
   * @brief Get all timestamps.
   * @param[out] (time_stamps) series of time stamp of frame
   * @return Status object.
   */
  virtual senscord::Status GetAllFrameTimestamp(
    std::vector<uint64_t>* time_stamps) = 0;

  /**
   * @brief Setup stream property.
   * @param[out] (stream_properties) serialized stream properties
   * @return Status object.
   */
  virtual senscord::Status SetupStreamProperty(
    skv_player::SerializedStreamProperties* stream_properties) = 0;

  /**
  * @brief Set valid time interval between frames.
  * @param[in] (interval) interval in microsecond 
  * @return Status object.
  */
  virtual senscord::Status SetFrameInterval(uint64_t interval) = 0;

  /**
  * @brief Reset frame index of channel accessor
  */
  virtual void ResetFrameIndex() = 0;

  /**
   * @brief Setup channel accessor.
   * @return Status object.
   */
  virtual senscord::Status SetupChannelAccessor() = 0;

  /**
   * @brief Delete channel accessor.
   * @return Status object.
   */
  virtual senscord::Status DeleteChannelAccessor() = 0;

  /**
   * @brief Constructor.
   */
  SkvPlayBaseFileManager() {}

  /**
   * @brief Destructor.
   */
  virtual ~SkvPlayBaseFileManager() {}
};

#endif  // LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAYER_BASE_FILE_MANAGER_H_
