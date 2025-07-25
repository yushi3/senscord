/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAYER_SOURCE_H_
#define LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAYER_SOURCE_H_

#include <map>
#include <string>
#include <vector>
#include "senscord/develop/stream_source.h"
#include "senscord/property_types.h"
#include "src/skv_play_library.h"
#include "src/skv_player_util.h"
#include "src/skv_player_base_file_manager.h"

/**
 * @brief The stream source of pseudo image (new style).
 */
class SkvPlayerSource : public senscord::DepthStreamSource {
 public:
  /**
   * @brief Constructor
   */
  SkvPlayerSource();

  /**
   * @brief Destructor
   */
  ~SkvPlayerSource();

  /**
   * @brief Open the stream source.
   * @param[in] (core) The core instance.
   * @param[in] (util) The utility accessor to core.
   * @return The status of function.
   */
  virtual senscord::Status Open(
      senscord::Core* core,
      senscord::StreamSourceUtility* util);

  /**
   * @brief Close the stream source.
   * @return The status of function.
   */
  virtual senscord::Status Close();

  /**
   * @brief Start the stream source.
   * @return The status of function.
   */
  virtual senscord::Status Start();

  /**
   * @brief Stop the stream source.
   * @return The status of function.
   */
  virtual senscord::Status Stop();

  /**
   * @brief Pull up the new frames.
   * @param[out] (frames) The information about new frames.
   */
  virtual void GetFrames(std::vector<senscord::FrameInfo>* frames);

  /**
   * @brief Release the used frame.
   * @param[in] (frameinfo) The information about used frame.
   * @param[in] (referenced_channel_ids) List of referenced channel IDs.
   *                                     (NULL is the same as empty)
   * @return The status of function.
   */
  virtual senscord::Status ReleaseFrame(
    const senscord::FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids);

  /// Mandatory properties.
  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Get(
    const std::string& key,
    senscord::DepthProperty* property);

  /**
   * @brief Set the stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Set(
    const std::string& key,
    const senscord::DepthProperty* property);

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Get(
    const std::string& key,
    senscord::ImageProperty* property);

  /**
   * @brief Set the stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Set(
    const std::string& key,
    const senscord::ImageProperty* property);

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Get(
    const std::string& key,
    senscord::ConfidenceProperty* property);

  /**
   * @brief Set the stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Set(
    const std::string& key,
    const senscord::ConfidenceProperty* property);

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Get(
    const std::string& key,
    senscord::ChannelInfoProperty* property);

  /**
   * @brief Set the stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Set(
    const std::string& key,
    const senscord::ChannelInfoProperty* property);

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Get(
    const std::string& key,
    senscord::FrameRateProperty* property);

  /**
   * @brief Set the stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Set(
    const std::string& key,
    const senscord::FrameRateProperty* property);

/// Original properties.

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Get(
    const std::string& key,
    senscord::PlayProperty* property);

  /**
   * @brief Set the stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Set(
    const std::string& key,
    const senscord::PlayProperty* property);

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Get(
    const std::string& key,
    senscord::PlayModeProperty* property);

  /**
   * @brief Set the stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Set(
    const std::string& key,
    const senscord::PlayModeProperty* property);

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Get(
    const std::string& key,
    senscord::BinaryProperty* property);

  /**
   * @brief Set the stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Set(
    const std::string& key,
    const senscord::BinaryProperty* property);

 private:
  /**
   * @brief parse arguments from senscord.xml
   * @return Status object
   */
  senscord::Status ParseArgument();

  /**
   * @brief Open skv file.
   * @param [out] (skv_play_library) library manager class
   * @param [in] (filename) Filename to be opened.
   * @return Status object.
   */
  senscord::Status OpenSkvFile(
    SkvPlayLibrary* skv_play_library, const std::string& filename);

  /**
   * @brief Close skv file.
   * @param [in] (skv_play_library) Filename to be opened.
   * @return Status object.
   */
  senscord::Status CloseSkvFile(SkvPlayLibrary* skv_play_library);

  /**
   * @brief Check if the file is created by SensCord.
   * @param [in] (skv_play_library) library manager class
   * @param [out] (is_senscord_format) true if the file is created by senscord.
   * @return Status object.
   */
  senscord::Status IsSensCordFormat(
    SkvPlayLibrary* skv_play_library,
    bool* is_senscord_format);

  /**
   * @brief parse the streams in the skv file.
   * @param [in] (skv_play_library) library manager class
   * @param [out] (skv_stream_map) map of link between stream_id and stream_info
   * @return Status object.
   */
  senscord::Status CreateStreamMap(
    SkvPlayLibrary* skv_play_library,
    std::map <std::string, SkvStreamInfo>* skv_stream_map);

  /**
   * @brief Create file manager
   * @param [in] (is_senscord_format) true if the file is created by senscord.
   * @param [out] (file_manager) file manager class
   * @param [in] (skv_play_library) library manager class
   * @param [in] (skv_stream_map) map of link between stream_id and stream_info
   * @param [out] (stream_properties) serialized stream properties
   * @return Status object.
   */
  senscord::Status CreateFileManager(
    bool is_senscord_format,
    SkvPlayBaseFileManager** file_manager,
    SkvPlayLibrary* skv_play_library,
    std::map <std::string, SkvStreamInfo>* skv_stream_map,
    skv_player::SerializedStreamProperties* stream_properties);

  /**
   * @brief Setup channel Accessor
   * @param [in](file_manager) file manager class
   * @return Status object.
   */
  senscord::Status SetupAccessor(SkvPlayBaseFileManager* file_manager);

  /**
   * @brief Setup channel Accessor
   * @param [in] (file_manager) file manager class
   * @return Status object.
   */
  senscord::Status DeleteAccessor(SkvPlayBaseFileManager* file_manager);

  /**
   * @brief Set Frame Interval in nanosecond
   * @param [in] (frame_interval) framerate fo player
   * @param [in] (file_manager) file manager class
   * @param [in] (skv_play_stream_property) structure of stream properties
   * @return Status object.
   */
  senscord::Status SetFrameInterval(
    uint64_t* frame_interval,
    SkvPlayBaseFileManager* file_manager,
    skv_player::SerializedStreamProperties* skv_play_stream_property);

  /**
   * @brief Reset frame index of channel accessor
   * @param [in] (file_manager) file manager class
   * @return Status object.
   */
  senscord::Status ResetFrameIndex(SkvPlayBaseFileManager* file_manager);

  /**
   * @brief Get all timestamps.
   * @param [in] (file_manager) file manager class
   * @param [out] (time_stamps) series of timestamp of frame
   * @param [in] (play_property) play_property
   * @return Status object.
   */
  senscord::Status GetAllFrameTimestamp(
    SkvPlayBaseFileManager* file_manager,
    std::vector<uint64_t>* time_stamps,
    const senscord::PlayProperty& play_property);

  /**
   * @brief Load stream data from disk to memory.
   * @return Status object.
   */
  senscord::Status CacheRawData();

  /**
   * @brief Find serialized stream property.
   * @param[in] Property key.
   * @param[in] Serialized property list.
   * @return Property pointer.
   */
  const senscord::BinaryProperty* FindSerializedStreamProperty(
      const std::string& key,
      const skv_player::SerializedStreamProperties& property_list) const;

 private:
  senscord::StreamSourceUtility* util_;
  senscord::MemoryAllocator* allocator_;
  uint64_t frame_seq_num_;
  uint64_t start_frame_num_;
  uint64_t stop_frame_num_;
  uint64_t current_frame_num_;
  uint64_t frame_interval_;
  uint64_t time_stamp_;
  std::vector<uint64_t> ordered_time_stamps_;
  std::string target_path_;
  bool is_senscord_format_;
  bool is_ready_to_start_;
  bool is_file_opened_;
  bool started_;
  SkvPlayLibrary skv_play_library_;
  std::map <std::string, SkvStreamInfo> skv_stream_map_;
  // properties
  skv_player::SerializedStreamProperties stream_properties_;
  senscord::PlayProperty play_property_;
  // file manager
  SkvPlayBaseFileManager* file_manager_;
};

#endif    // LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAYER_SOURCE_H_
