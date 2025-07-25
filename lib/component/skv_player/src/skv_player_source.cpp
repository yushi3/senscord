/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/skv_player_source.h"
#include <map>
#include <string>
#include <vector>
#include <utility>
#include "senscord/senscord.h"
#include "senscord/environment.h"
#include "senscord/logger.h"
#include "senscord/osal.h"
#include "src/skv_player_common.h"
#include "src/skv_player_senscord_file_manager.h"
#include "src/skv_play_library.h"

/**
 * @brief Open the stream source.
 * @param[in] (core) The core instance.
 * @param[in] (util) The utility accessor to core.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Open(
    senscord::Core* core,
    senscord::StreamSourceUtility* util) {
  SENSCORD_LOG_DEBUG("SkvPlayer: Open");

  if (core == NULL || util == NULL) {
    return SENSCORD_STATUS_FAIL(kBlockName,
                                senscord::Status::kCauseInvalidArgument,
                                "Open: Null pointer");
  }

  util_ = util;

  // get allocator (use default)
  senscord::Status status = util_->GetAllocator(
      senscord::kAllocatorNameDefault, &allocator_);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // regiseter property
  SENSCORD_REGISTER_PROPERTY(
    util_, senscord::kPlayPropertyKey, senscord::PlayProperty);
  SENSCORD_REGISTER_PROPERTY(
    util_, senscord::kPlayModePropertyKey, senscord::PlayModeProperty);

  // parse arguments
  status = ParseArgument();

  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}

/**
 * @brief Close the stream source.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Close() {
  SENSCORD_LOG_DEBUG("SkvPlayer: Close");
  is_ready_to_start_ = false;
  senscord::Status status = CloseSkvFile(&skv_play_library_);
  if (!status.ok()) {
    SENSCORD_LOG_ERROR("Fail to close the stream source status = %s",
    status.ToString().c_str());
  }

  status = DeleteAccessor(file_manager_);
  if (!status.ok()) {
    SENSCORD_LOG_ERROR("Fail to close the stream source status = %s",
    status.ToString().c_str());
  }
  delete file_manager_;
  file_manager_ = NULL;

  SENSCORD_LOG_DEBUG("[skvplay] close");
  return senscord::Status::OK();
}

/**
 * @brief Start the stream source.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Start() {
  SENSCORD_LOG_DEBUG("SkvPlayer: Start");
  if (!is_ready_to_start_) {
    return SENSCORD_STATUS_FAIL(kBlockName,
                              senscord::Status::kCauseInvalidOperation,
                              "The file is not ready to start.");
  }
  senscord::Status status = ResetFrameIndex(file_manager_);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  status = CacheRawData();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  // Initialize framecount.
  start_frame_num_   = play_property_.start_offset;
  stop_frame_num_    = play_property_.start_offset + play_property_.count;
  current_frame_num_ = start_frame_num_;
  started_ = true;
  return senscord::Status::OK();
}

/**
 * @brief Stop the stream source.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Stop() {
  SENSCORD_LOG_DEBUG("SkvPlayer: Stop");
  started_ = false;
  return senscord::Status::OK();
}

/**
 * @brief Pull up the new frames.
 * @param[out] (frames) The information about new frames.
 */
void SkvPlayerSource::GetFrames(std::vector<senscord::FrameInfo>* frames) {
  senscord::osal::OSSleep(frame_interval_);

  if (current_frame_num_ >= stop_frame_num_) {
    if (play_property_.mode.repeat) {
      ResetFrameIndex(file_manager_);
      current_frame_num_ = start_frame_num_;
    } else {
      return;
    }
  }

  // get timestamp of playback frame
  time_stamp_ = ordered_time_stamps_[current_frame_num_];
  ++current_frame_num_;

  // get time for log.
  uint64_t before_time = 0;
  senscord::osal::OSGetTime(&before_time);

  senscord::FrameInfo frameinfo = {};
  senscord::Status status = file_manager_->GetFrame(time_stamp_, &frameinfo);

  // get time for log.
  uint64_t after_time = 0;
  senscord::osal::OSGetTime(&after_time);
  // Check if elapsed time is more than 1 second
  if (after_time > (before_time + (1ULL * 1000 * 1000 * 1000))) {
    SENSCORD_LOG_WARNING("Getframe() : More than 1 second has passed");
  }

  if (!status.ok()) {
    SENSCORD_LOG_WARNING(
        "Getframe(" PRIu64 ") failed: ret=%s",
        frameinfo.sequence_number, status.ToString().c_str());
    util_->SendEventFrameDropped(frameinfo.sequence_number);
    ReleaseFrame(frameinfo, NULL);
    return;
  }
  if (frameinfo.channels.empty()) {
    ReleaseFrame(frameinfo, NULL);
    SENSCORD_LOG_DEBUG("frameinfo.channels is empty");
    return;
  }

  frameinfo.sequence_number = frame_seq_num_++;
  frames->push_back(frameinfo);
}

/**
 * @brief Release the used frame.
 * @param[in] (frameinfo) The information about used frame.
 * @param[in] (referenced_channel_ids) List of referenced channel IDs.
 *                                     (NULL is the same as empty)
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::ReleaseFrame(
    const senscord::FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids) {
  // free raw data
  typedef std::vector<senscord::ChannelRawData> ChannelRawList;
  ChannelRawList::const_iterator itr = frameinfo.channels.begin();
  ChannelRawList::const_iterator end = frameinfo.channels.end();
  for (; itr != end; ++itr) {
    if (itr->data_memory != NULL) {
      allocator_->Free(itr->data_memory);
    }
  }
  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Get(
    const std::string& key,
    senscord::DepthProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: GetProperty(%s)", key.c_str());
  }

  // Find property
  const senscord::BinaryProperty* serialized =
      FindSerializedStreamProperty(key, stream_properties_);
  if (serialized == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseNotFound,
        "Not found property: key=%s",
        senscord::PropertyUtils::GetKey(key).c_str());
  }

  // Deserialize property
  senscord::Status status =
      skv_player::DecodeSerializedProperty(*serialized, property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}

/**
 * @brief Set the stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Set(
    const std::string& key,
    const senscord::DepthProperty* property) {
  // Serialize and retain property
  senscord::BinaryProperty& binary =
      stream_properties_[senscord::PropertyUtils::GetKey(key)];
  senscord::Status status =
      skv_player::EncodeDeserializedProperty(*property, &binary);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Get(
    const std::string& key,
    senscord::ImageProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer:GetProperty(%s)",
      senscord::PropertyUtils::GetKey(key).c_str());
  }

  // Find property
  const senscord::BinaryProperty* serialized =
      FindSerializedStreamProperty(key, stream_properties_);
  if (serialized == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseNotFound,
        "Not found property: key=%s",
        senscord::PropertyUtils::GetKey(key).c_str());
  }

  // Deserialize property
  senscord::Status status =
      skv_player::DecodeSerializedProperty(*serialized, property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}

/**
 * @brief Set the stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Set(
    const std::string& key,
    const senscord::ImageProperty* property) {
  // Serialize and retain property
  senscord::BinaryProperty& binary =
      stream_properties_[senscord::PropertyUtils::GetKey(key)];
  senscord::Status status =
      skv_player::EncodeDeserializedProperty(*property, &binary);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Get(
    const std::string& key,
    senscord::ConfidenceProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName, senscord::Status::kCauseInvalidArgument,
      "Null pointer: GetProperty(%s)",
      senscord::PropertyUtils::GetKey(key).c_str());
  }

  // Find property
  const senscord::BinaryProperty* serialized =
      FindSerializedStreamProperty(key, stream_properties_);
  if (serialized == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseNotFound,
        "Not found property: key=%s",
        senscord::PropertyUtils::GetKey(key).c_str());
  }

  // Deserialize property
  senscord::Status status =
      skv_player::DecodeSerializedProperty(*serialized, property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}

/**
 * @brief Set the stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Set(
    const std::string& key,
    const senscord::ConfidenceProperty* property) {
  // Serialize and retain property
  senscord::BinaryProperty& binary =
      stream_properties_[senscord::PropertyUtils::GetKey(key)];
  senscord::Status status =
      skv_player::EncodeDeserializedProperty(*property, &binary);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Get(
    const std::string& key,
    senscord::ChannelInfoProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName, senscord::Status::kCauseInvalidArgument,
      "Null pointer: GetProperty(%s)",
      senscord::PropertyUtils::GetKey(key).c_str());
  }

  // Find property
  const senscord::BinaryProperty* serialized =
      FindSerializedStreamProperty(key, stream_properties_);
  if (serialized == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseNotFound,
        "Not found property: key=%s",
        senscord::PropertyUtils::GetKey(key).c_str());
  }

  // Deserialize property
  senscord::Status status =
      skv_player::DecodeSerializedProperty(*serialized, property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}

/**
 * @brief Set the stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Set(
    const std::string& key,
    const senscord::ChannelInfoProperty* property) {
  // Serialize and retain property
  senscord::BinaryProperty& binary =
      stream_properties_[senscord::PropertyUtils::GetKey(key)];
  senscord::Status status =
      skv_player::EncodeDeserializedProperty(*property, &binary);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Get(
    const std::string& key,
    senscord::FrameRateProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: GetProperty(%s)",
      senscord::PropertyUtils::GetKey(key).c_str());
  }

  // Find property
  const senscord::BinaryProperty* serialized =
      FindSerializedStreamProperty(key, stream_properties_);
  if (serialized == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseNotFound,
        "Not found property: key=%s",
        senscord::PropertyUtils::GetKey(key).c_str());
  }

  // Deserialize property
  senscord::Status status =
      skv_player::DecodeSerializedProperty(*serialized, property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}

/**
 * @brief Set the new stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Set(
    const std::string& key,
    const senscord::FrameRateProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: GetProperty(%s)",
      senscord::PropertyUtils::GetKey(key).c_str());
  }

  if (property->denom == 0 || property->num == 0) {
    return SENSCORD_STATUS_FAIL(
      kBlockName, senscord::Status::kCauseInvalidArgument, "0 value");
  }

  uint64_t new_frame_interval =
      ((1000ULL * 1000 * 1000) * property->denom) / property->num;

  if (frame_interval_ != new_frame_interval) {
    // Serialize and retain property
    senscord::BinaryProperty& binary =
        stream_properties_[senscord::PropertyUtils::GetKey(key)];
    senscord::Status status =
        skv_player::EncodeDeserializedProperty(*property, &binary);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    // Update frame rate
    status = SetFrameInterval(
        &frame_interval_, file_manager_, &stream_properties_);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    SENSCORD_LOG_INFO("change framerate to %" PRIu32 " / %" PRIu32,
                      property->num, property->denom);
    // notify to streams
    util_->SendEvent(senscord::kEventPropertyUpdated, NULL);
  }
  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Get(
    const std::string& key,
    senscord::PlayProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: GetProperty(%s)",
      senscord::PropertyUtils::GetKey(key).c_str());
  }
  *property = play_property_;

  return senscord::Status::OK();
}

/**
 * @brief Set the stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Set(
    const std::string& key,
    const senscord::PlayProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer");
  }
  if (started_) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidOperation,
      "Playproperty cannot be set when the stream is started");
  }

  if (play_property_.target_path != property->target_path) {
    std::string target_path =  property->target_path;
    uint64_t frame_interval = 0;
    std::vector<uint64_t> ordered_time_stamps;
    bool is_senscord_format = false;
    bool is_ready_to_start  = false;
    SkvPlayLibrary skv_play_library;
    std::map<std::string, SkvStreamInfo> skv_stream_map;
    skv_player::SerializedStreamProperties stream_properties;
    SkvPlayBaseFileManager* file_manager = NULL;

    // close current file
    if (is_file_opened_) {
      senscord::Status status = CloseSkvFile(&skv_play_library_);
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
    }
    // open new file
    senscord::Status status = OpenSkvFile(&skv_play_library_, target_path);
    if (!status.ok()) {
      // roll back if fail to open new file
      OpenSkvFile(&skv_play_library_, play_property_.target_path);
      return SENSCORD_STATUS_TRACE(status);
    }

    do {
      status = IsSensCordFormat(&skv_play_library_, &is_senscord_format);
      if (!status.ok()) {
        break;
      }

      status = CreateStreamMap(&skv_play_library_, &skv_stream_map);
      if (!status.ok()) {
        break;
      }

      status = CreateFileManager(is_senscord_format, &file_manager,
          &skv_play_library_, &skv_stream_map, &stream_properties);
      if (!status.ok()) {
        if (file_manager != NULL) {
          delete file_manager;
        }
        break;
      }

      do {
        status = SetupAccessor(file_manager);
        if (!status.ok()) {
          break;
        }
        status = GetAllFrameTimestamp(
            file_manager, &ordered_time_stamps, *property);
        if (!status.ok()) {
          break;
        }
        status = SetFrameInterval(
          &frame_interval, file_manager, &stream_properties);
        if (!status.ok()) {
          break;
        }
        status = ResetFrameIndex(file_manager);
        if (!status.ok()) {
          break;
        }
      } while (0);

      if (!status.ok()) {
        senscord::Status status_tmp = DeleteAccessor(file_manager);
        if (!status_tmp.ok()) {
          SENSCORD_LOG_ERROR("Fail to DeleteAccessor = %s",
          status_tmp.ToString().c_str());
        }
        delete file_manager;
        break;
      }
    } while (0);

    // if fail to open another file.
    if (!status.ok()) {
      senscord::Status status_tmp = CloseSkvFile(&skv_play_library_);
      if (!status_tmp.ok()) {
         SENSCORD_LOG_WARNING("%s", status_tmp.ToString().c_str());
      }
      OpenSkvFile(&skv_play_library_, play_property_.target_path);
      return SENSCORD_STATUS_TRACE(status);
    }

    is_ready_to_start = true;
    if (play_property_.start_offset != property->start_offset) {
      std::vector<uint64_t> dummy_time_stamps;
      // check if the start_offset is less than the file end time.
      status = GetAllFrameTimestamp(
          file_manager, &dummy_time_stamps, *property);
      if (!status.ok()) {
        senscord::Status status_tmp = DeleteAccessor(file_manager);
        if (!status_tmp.ok()) {
          SENSCORD_LOG_ERROR("Fail to DeleteAccessor = %s",
          status_tmp.ToString().c_str());
        }
        delete file_manager;
        return SENSCORD_STATUS_TRACE(status);
      }
    }

    if (file_manager_ != NULL) {
      status = DeleteAccessor(file_manager_);
      if (!status.ok()) {
        senscord::Status status_tmp = DeleteAccessor(file_manager);
        if (!status_tmp.ok()) {
          SENSCORD_LOG_WARNING("Fail to DeleteAccessor = %s",
              status_tmp.ToString().c_str());
        }
        delete file_manager;
        return SENSCORD_STATUS_TRACE(status);
      }
    }

    delete file_manager_;
    file_manager_ = file_manager;
    is_ready_to_start_  = is_ready_to_start;
    frame_interval_ = frame_interval;
    ordered_time_stamps_ = ordered_time_stamps;
    is_senscord_format_ = is_senscord_format;
    skv_stream_map_     = skv_stream_map;
    stream_properties_ = stream_properties;
  } else if (is_file_opened_) {
    if (play_property_.start_offset != property->start_offset) {
      std::vector<uint64_t> dummy_time_stamps;
      // check if the start_offset is less than the file end time.
      senscord::Status status = GetAllFrameTimestamp(
        file_manager_, &dummy_time_stamps, *property);
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
    }
  }
  play_property_ = *property;
  play_property_.speed = senscord::kPlaySpeedBasedOnFramerate;
  //  count is fitted into the real playback time.
  if (!play_property_.target_path.empty()) {
    uint32_t total_frame = static_cast<uint32_t>(ordered_time_stamps_.size());
    uint32_t playback_last_frame =
        play_property_.start_offset + play_property_.count;
    if ((total_frame < playback_last_frame) || (play_property_.count == 0)) {
      play_property_.count = total_frame - play_property_.start_offset;
    }
  }
  if (is_file_opened_) {
    senscord::Status status = SetFrameInterval(
        &frame_interval_, file_manager_, &stream_properties_);
    if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
    }
  }
  // notify to streams
  util_->SendEvent(senscord::kEventPropertyUpdated, NULL);
  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Get(
    const std::string& key,
    senscord::PlayModeProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: GetProperty(%s)",
      senscord::PropertyUtils::GetKey(key).c_str());
  }
  *property = play_property_.mode;

  return senscord::Status::OK();
}

/**
 * @brief Set the stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Set(
    const std::string& key,
    const senscord::PlayModeProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: SetProperty(%s)",
      senscord::PropertyUtils::GetKey(key).c_str());
  }
  play_property_.mode = *property;

  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Get(const std::string& key,
    senscord::BinaryProperty* property) {
  skv_player::SerializedStreamProperties::const_iterator found;
  found = stream_properties_.find(senscord::PropertyUtils::GetKey(key));
  if (found != stream_properties_.end()) {
    *property = found->second;
  } else {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseNotSupported,
        "Not found property: key=%s",
        senscord::PropertyUtils::GetKey(key).c_str());
  }

  return senscord::Status::OK();
}

/**
 * @brief Set the stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status SkvPlayerSource::Set(const std::string& key,
    const senscord::BinaryProperty* property) {
  skv_player::SerializedStreamProperties::iterator found;
  found = stream_properties_.find(senscord::PropertyUtils::GetKey(key));
  if (found != stream_properties_.end()) {
    found->second = *property;
  } else {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseNotSupported,
        "Not found property: key=%s",
        senscord::PropertyUtils::GetKey(key).c_str());
  }

  return senscord::Status::OK();
}

/**
 * @brief parse arguments from senscord.xml
 * @return Status object
 */
senscord::Status SkvPlayerSource::ParseArgument() {
  // target_path
  std::string target_path;
  senscord::Status status =
    util_->GetStreamArgument(kSkvArgTargetPath, &target_path);
  if (status.ok()) {
    target_path_ = target_path;
    play_property_.target_path = target_path;
  } else if (status.cause() != senscord::Status::kCauseNotFound) {
    return SENSCORD_STATUS_TRACE(status);
  } else {
    SENSCORD_LOG_INFO("target_path is not found");
  }

  // start_offset, count, repeat
  if (!target_path_.empty()) {
    SENSCORD_LOG_INFO("[skvplay] %s = %s",
        kSkvArgTargetPath, target_path_.c_str());

    std::string arg_start_offset;
    status = util_->GetStreamArgument(kSkvArgStartOffset, &arg_start_offset);
    if (status.cause() != senscord::Status::kCauseNotFound) {
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
      if (arg_start_offset.find("-") == std::string::npos) {
        uint64_t num = 0;
        int result = senscord::osal::OSStrtoull(
          arg_start_offset.c_str(), NULL, 0, &num);
        if ((result == 0) && (num <= UINT32_MAX)) {
          play_property_.start_offset = static_cast<uint32_t>(num);
        } else {
          return SENSCORD_STATUS_FAIL(
              kBlockName, senscord::Status::kCauseInvalidArgument,
              "Invalid argument '%s'.", kSkvArgStartOffset);
        }
      } else {
        return SENSCORD_STATUS_FAIL(
            kBlockName, senscord::Status::kCauseInvalidArgument,
          "Invalid argument '%s'.", kSkvArgStartOffset);
      }
    }
    SENSCORD_LOG_INFO("[skvplay] %s = %" PRIu32,
        kSkvArgStartOffset, play_property_.start_offset);

    std::string count;
    status = util_->GetStreamArgument(kSkvArgCount, &count);
    if (status.ok()) {
      if (count == "all" || count == "0") {
        play_property_.count = 0;
      } else {
        //  check negative value
        if (count.find("-") == std::string::npos) {
          uint64_t num = 0;
          int result = senscord::osal::OSStrtoull(count.c_str(), NULL, 0, &num);
          if ((result == 0) && (num <= UINT32_MAX)) {
            play_property_.count = static_cast<uint32_t>(num);
          } else {
            return SENSCORD_STATUS_FAIL(
                kBlockName, senscord::Status::kCauseInvalidArgument,
                "Invalid argument '%s'.", kSkvArgCount);
          }
        } else {
          return SENSCORD_STATUS_FAIL(
              kBlockName, senscord::Status::kCauseInvalidArgument,
              "Invalid argument '%s'.", kSkvArgCount);
        }
      }
    } else if (status.cause() != senscord::Status::kCauseNotFound) {
      return SENSCORD_STATUS_TRACE(status);
    }
    SENSCORD_LOG_INFO("[skvplay] %s = %" PRIu32, kSkvArgCount,
      play_property_.count);

    std::string repeat;
    status = util_->GetStreamArgument(kSkvArgRepeat, &repeat);
    if (status.ok()) {
      if (repeat == "true") {
        play_property_.mode.repeat = true;
      } else if (repeat == "false") {
        play_property_.mode.repeat = false;
      }  else {
        return SENSCORD_STATUS_FAIL(
            kBlockName, senscord::Status::kCauseInvalidArgument,
            "specify true or false for the argument 'repeat'.");
      }
    } else if (status.cause() != senscord::Status::kCauseNotFound) {
      return SENSCORD_STATUS_TRACE(status);
    }
    SENSCORD_LOG_INFO("[skvplay] %s = %" PRIu32, kSkvArgRepeat,
      play_property_.mode.repeat);

    // SkvPlayLibrary OpenSKVFile;
    status = OpenSkvFile(&skv_play_library_, target_path);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    // SkvPlayLibrary IsSenscordFormat;
    status = IsSensCordFormat(&skv_play_library_, &is_senscord_format_);
    if (!status.ok()) {
      CloseSkvFile(&skv_play_library_);
      return SENSCORD_STATUS_TRACE(status);
    }

    // SkvPlayLibrary CreateStreamMap;
    status = CreateStreamMap(&skv_play_library_, &skv_stream_map_);
    if (!status.ok()) {
      CloseSkvFile(&skv_play_library_);
      return SENSCORD_STATUS_TRACE(status);
    }

    SkvPlayBaseFileManager* file_manager = NULL;
    do {  // while(0)
      // Create file manager
      status = CreateFileManager(is_senscord_format_, &file_manager,
          &skv_play_library_, &skv_stream_map_, &stream_properties_);
      if (!status.ok()) {
        SENSCORD_STATUS_TRACE(status);
        break;
      }

      // Setup channel accessor
      status = SetupAccessor(file_manager);
      if (!status.ok()) {
        SENSCORD_STATUS_TRACE(status);
        break;
      }

      // Get the timestamp of the Frame in the skv file
      status = GetAllFrameTimestamp(
          file_manager, &ordered_time_stamps_, play_property_);
      if (!status.ok()) {
        SENSCORD_STATUS_TRACE(status);
        break;
      }

      //  count is fitted into the real playback time.
      uint32_t total_frame = static_cast<uint32_t>(ordered_time_stamps_.size());
      uint32_t play_last_frame =
          play_property_.start_offset + play_property_.count;
      if ((total_frame < play_last_frame) || (play_property_.count == 0)) {
        play_property_.count = total_frame - play_property_.start_offset;
      }

      // Set the send frame interval
      status = SetFrameInterval(
        &frame_interval_, file_manager, &stream_properties_);
      if (!status.ok()) {
        SENSCORD_STATUS_TRACE(status);
        break;
      }

      // Reset frame index
      status = ResetFrameIndex(file_manager);
      if (!status.ok()) {
        SENSCORD_STATUS_TRACE(status);
        break;
      }
    } while (0);

    if (!status.ok()) {
      // Do nothing if no accessor is generated
      DeleteAccessor(file_manager);

      delete file_manager;
      CloseSkvFile(&skv_play_library_);
      return SENSCORD_STATUS_TRACE(status);
    }

    is_ready_to_start_ = true;

    // apply
    file_manager_ = file_manager;
  }
  return senscord::Status::OK();
}

/**
 * @brief Open skv file.
 * @param [out] (skv_play_library) library manager class
 * @param [in] (filename) Filename to be opened.
 * @return Status object.
 */
senscord::Status SkvPlayerSource::OpenSkvFile(
    SkvPlayLibrary* skv_play_library,
    const std::string& filename) {
  if (skv_play_library == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: OpenSkvFile");
  }
  if (is_file_opened_) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidOperation,
      "File is already opened: OpenSkvFile");
  }
  senscord::Status status = skv_play_library->OpenFile(filename);

  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  is_file_opened_ = true;
  return senscord::Status::OK();
}

/**
 * @brief Close skv file.
 * @param [in] (skv_play_library) Filename to be opened.
 * @return Status object.
 */
senscord::Status SkvPlayerSource::CloseSkvFile(
    SkvPlayLibrary* skv_play_library) {
  if (skv_play_library == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: CloseSkvFile");
  }
  if (!is_file_opened_) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidOperation,
      "opened file is not found: CloseSkvFile");
  }
  senscord::Status status = skv_play_library->CloseFile();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  is_file_opened_ = false;
  return senscord::Status::OK();
}

/**
 * @brief Check if the file is created by SensCord.
 * @param [in] (skv_play_library) library manager class
 * @param [out] (is_senscord_format) true if the file is created by senscord.
 * @return Status object.
 */
senscord::Status SkvPlayerSource::IsSensCordFormat(
    SkvPlayLibrary* skv_play_library,
    bool* is_senscord_format) {
  if (skv_play_library == NULL || is_senscord_format == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: IsSensCordFormat");
  }
  senscord::Status status =
      skv_play_library->IsSensCordFormat(is_senscord_format);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}

/**
 * @brief parse the streams in the skv file.
 * @param [in] (skv_play_library) library manager class
 * @param [out] (skv_stream_map) map of link between stream_id and stream_info
 * @return Status object.
 */
senscord::Status SkvPlayerSource::CreateStreamMap(
    SkvPlayLibrary* skv_play_library,
    std::map <std::string, SkvStreamInfo>* skv_stream_map) {
  if (skv_play_library == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: CreateStreamMap");
  }
  senscord::Status status = skv_play_library->CreateStreamMap(skv_stream_map);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  // display stream map
  std::map<std::string, SkvStreamInfo>::iterator itr;
  for (itr = skv_stream_map->begin(); itr != skv_stream_map->end(); ++itr) {
    SENSCORD_LOG_INFO(
        "%s:id = %d ,size= %d", itr->first.c_str(),
        itr->second.id, itr->second.frame_size);
  }

  return senscord::Status::OK();
}

/**
 * @brief Create file manager
 * @param [in] (is_senscord_format) true if the file is created by senscord.
 * @param [out] (file_manager) file manager class
 * @param [in] (skv_play_library) library manager class
 * @param [out] (skv_play_stream_property) structure of stream properties
 * @param [in] (skv_stream_map) map of link between stream_id and stream_info
 * @param [in] (stream_properties) serialized stream properties
 * @return Status object.
 */
senscord::Status SkvPlayerSource::CreateFileManager(
    bool is_senscord_format, SkvPlayBaseFileManager** file_manager,
    SkvPlayLibrary* skv_play_library,
    std::map <std::string, SkvStreamInfo>* skv_stream_map,
    skv_player::SerializedStreamProperties* stream_properties) {
  if (*file_manager != NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "File manager already exists.");
  }
  if (skv_play_library == NULL || stream_properties == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: CreateFileManager");
  }
  if (is_senscord_format) {
    *file_manager = new SkvPlaySensCordFileManager();
  } else {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseNotSupported,
        "It is not a SensCord recorded file");
  }

  (*file_manager)->Init(stream_properties, skv_play_library,
    util_, skv_stream_map, allocator_);
  senscord::Status status =
    (*file_manager)->SetupStreamProperty(stream_properties);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  skv_player::SerializedStreamProperties::iterator itr =
      stream_properties->begin();
  skv_player::SerializedStreamProperties::iterator end =
      stream_properties->end();
  for (; itr != end; ++itr) {
    if (itr->first == senscord::kDepthPropertyKey) {
      SENSCORD_REGISTER_PROPERTY(util_, senscord::kDepthPropertyKey,
          senscord::DepthProperty);
    } else if (itr->first == senscord::kImagePropertyKey) {
      SENSCORD_REGISTER_PROPERTY(util_, senscord::kImagePropertyKey,
        senscord::ImageProperty);
    } else if (itr->first == senscord::kConfidencePropertyKey) {
      SENSCORD_REGISTER_PROPERTY(util_, senscord::kConfidencePropertyKey,
        senscord::ConfidenceProperty);
    } else if (itr->first == senscord::kChannelInfoPropertyKey) {
      SENSCORD_REGISTER_PROPERTY(util_, senscord::kChannelInfoPropertyKey,
        senscord::ChannelInfoProperty);
    } else if (itr->first == senscord::kFrameRatePropertyKey) {
      SENSCORD_REGISTER_PROPERTY(util_, senscord::kFrameRatePropertyKey,
        senscord::FrameRateProperty);
    } else {
      // Register serialized property
      SENSCORD_REGISTER_SERIALIZED_PROPERTY(util_, itr->first);
    }
  }

  return senscord::Status::OK();
}

/**
 * @brief Setup channel Accessor
 * @param [in](file_manager) file manager class
 * @return Status object.
 */
senscord::Status SkvPlayerSource::SetupAccessor(
    SkvPlayBaseFileManager* file_manager) {
  if (file_manager == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidArgument,
        "Null pointer: SetupAccessor");
  }

  senscord::Status status = file_manager->SetupChannelAccessor();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  return senscord::Status::OK();
}

/**
 * @brief Setup channel Accessor
 * @param [in] (file_manager) file manager class
 * @return Status object.
 */
senscord::Status SkvPlayerSource::DeleteAccessor(
    SkvPlayBaseFileManager* file_manager) {
  if (file_manager == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidArgument,
        "Null pointer: DeleteAccessor");
  }

  senscord::Status status = file_manager->DeleteChannelAccessor();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}

/**
 * @brief Set Frame Interval in nanosecond
 * @param [in] (frame_interval) framerate fo player
 * @param [out] (file_manager) file manager class
 * @param [in] (skv_play_stream_property) structure of stream properties
 * @return Status object.
 */
senscord::Status SkvPlayerSource::SetFrameInterval(
    uint64_t* frame_interval, SkvPlayBaseFileManager* file_manager,
    skv_player::SerializedStreamProperties* stream_properties) {
  if (frame_interval == NULL || stream_properties == NULL ||
    file_manager == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: SetFraemInterval");
  }

  // Find property
  const senscord::BinaryProperty* serialized = FindSerializedStreamProperty(
      senscord::kFrameRatePropertyKey, *stream_properties);
  if (serialized == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseNotFound,
        "Not found property: key=%s", senscord::kFrameRatePropertyKey);
  }

  // Deserialize property
  senscord::FrameRateProperty property = {};
  senscord::Status status =
      skv_player::DecodeSerializedProperty(*serialized, &property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  uint64_t interval;
  if (property.num != 0 && property.denom != 0) {
    interval =
      ((1000ULL * 1000 * 1000) * property.denom) / property.num;
  } else {
    interval = kDefaultFrameRate;
  }
  status = file_manager->SetFrameInterval(interval);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  *frame_interval = interval;
  return senscord::Status::OK();
}

/**
 * @brief Reset frame index of channel accessor
 * @param [out] (file_manager) file manager class
 * @return Status object.
 */
senscord::Status SkvPlayerSource::ResetFrameIndex(
    SkvPlayBaseFileManager* file_manager) {
  if (file_manager == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName, senscord::Status::kCauseInvalidArgument,
      "Null pointer: ResetFrameIndex");
  }
  file_manager->ResetFrameIndex();

  return senscord::Status::OK();
}

/**
 * @brief Get all timestamps.
 * @param [in] (file_manager) file manager class
 * @param [out] (time_stamps) series of timestamp of frame
 * @param [in] (play_property) play_property
 * @return Status object.
 */
senscord::Status SkvPlayerSource::GetAllFrameTimestamp(
    SkvPlayBaseFileManager* file_manager,
    std::vector<uint64_t>* time_stamps,
    const senscord::PlayProperty& play_property) {
  if (file_manager == NULL || time_stamps == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseInvalidArgument,
      "Null pointer: GetAllFrameTimestamp");
  }

  std::vector<uint64_t> times;
  senscord::Status status = file_manager->GetAllFrameTimestamp(&times);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  if (times.size() <= play_property.start_offset) {
    return SENSCORD_STATUS_FAIL(
      kBlockName, senscord::Status::kCauseInvalidArgument,
      "The start_offset is more than the count of frames:"
      "offset(%" PRIu32 ")/total_frame(%" PRIuS ")",
      play_property.start_offset, times.size());
  }

  // apply time stamp
  *time_stamps = times;

  return senscord::Status::OK();
}

/**
 * @brief Load stream data from disk to memory.
 * @return Status object.
 */
senscord::Status SkvPlayerSource::CacheRawData() {
  // this is trial code. need to be implement independent function.(by Yuki)
  senscord::Status status = file_manager_->CacheRawData();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  return senscord::Status::OK();
}

/**
 * @brief Find serialized stream property.
 * @param[in] Property key.
 * @param[in] Serialized property list.
 * @return Property pointer.
 */
const senscord::BinaryProperty* SkvPlayerSource::FindSerializedStreamProperty(
    const std::string& key,
    const skv_player::SerializedStreamProperties& property_list) const {
  // Find StreamProperty
  skv_player::SerializedStreamProperties::const_iterator found;
  found = property_list.find(senscord::PropertyUtils::GetKey(key));
  if (found == property_list.end()) {
    return NULL;
  }

  return &(found->second);
}

/**
 * @brief Constructor
 * @param[in] (allocator) Memory allocator for this source.
 */
SkvPlayerSource::SkvPlayerSource()
    : util_(),
      allocator_(),
      frame_seq_num_(0),
      start_frame_num_(0),
      stop_frame_num_(0),
      current_frame_num_(0),
      time_stamp_(0),
      ordered_time_stamps_(),
      target_path_(),
      is_senscord_format_(true),
      is_ready_to_start_(false),
      is_file_opened_(false),
      started_(false),
      skv_play_library_(),
      skv_stream_map_(),
      stream_properties_(),
      play_property_(),
      file_manager_(NULL) {
  SENSCORD_LOG_DEBUG("[skvplayer] constructor");
  // setup properties to default value.
  frame_interval_ = (1000ULL * 1000 * 1000) / kDefaultFrameRateNum;
  play_property_.start_offset = 0;
  play_property_.count = 0;
  play_property_.speed = senscord::kPlaySpeedBasedOnFramerate;

  play_property_.mode.repeat = false;
}

/**
 * @brief Destructor
 */
SkvPlayerSource::~SkvPlayerSource() {
  util_ = NULL;
  allocator_ = NULL;
}
