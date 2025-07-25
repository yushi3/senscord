/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/skv_play_library.h"
#include <string>
#include <vector>
#include "senscord/logger.h"
#include "senscord/osal.h"
#include "src/skv_player_common.h"

/**
 * @brief Open skv file.
 * @param[in] (target_path) skv file path to be opened.
 * @return Status object.
 */
senscord::Status SkvPlayLibrary::OpenFile(const std::string& target_path) {
  if (file_handle_ != NULL) {
    // A file opend already
    return SENSCORD_STATUS_FAIL(kBlockName,
      senscord::Status::kCauseAlreadyExists,
      "existed skv file handle");
  }
  skv_error_code  ec = skv_open_file(
    &file_handle_, target_path.c_str(), skv_read_only, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
    "SkvIF Error(open_file): %s", skv_error_message(ec));
  } else if (file_handle_ == NULL) {
    return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
    "SkvIF Error(open_file): open file failure");
  }
  return senscord::Status::OK();
}

/**
 * @brief Close skv file.
 * @return Status object.
 */
senscord::Status SkvPlayLibrary::CloseFile() {
  if (file_handle_ == NULL) {
    // closed
    return senscord::Status::OK();
  }
  skv_close_file(file_handle_);
  file_handle_ = NULL;
  return senscord::Status::OK();
}

/**
 * @brief Check if the file is created by SensCord.
 * @param[in] (is_senscord_format) file format flag
 * @return Status object.
 */
senscord::Status SkvPlayLibrary::IsSensCordFormat(bool* is_senscord_format) {
  // check file format
  skv_error_code  ec = skv_has_custom_buffer(
    file_handle_, kSkvStreamPropertyName, is_senscord_format, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
    "SkvIF Error(has_senscord_stream_property): %s", skv_error_message(ec));
  }
  return senscord::Status::OK();
}

/**
 * @brief Create a map of streams in a skv file
 * @param[in] (skv_stream_map) map of the stream
 * @return Status object.
 */
senscord::Status SkvPlayLibrary::CreateStreamMap(
     std::map <std::string, SkvStreamInfo>* skv_stream_map) {
  // Parse streams
  if (file_handle_ == NULL) {
    return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
      "SkvIF Error(file_handle is NULL)");
  }
  uint32_t stream_count;
  skv_error_code  ec = skv_get_stream_count(file_handle_, &stream_count, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
      "SkvIF Error(skv_get_stream_count): %s", skv_error_message(ec));
  }
  for (uint32_t stream_id=0; stream_id < stream_count; ++stream_id) {
    std::string name;
    SkvStreamInfo skv_stream_info = {};
    senscord::Status status =
      SetStreamPairedInfo(stream_id, &name, &skv_stream_info);
    if (status.ok()) {
      (*skv_stream_map)[name] = skv_stream_info;
    } else {
      return SENSCORD_STATUS_TRACE(status);
    }
  }
  return senscord::Status::OK();
}

/**
 * @brief get the size of a frame in the stream
 * @param[in] (stream_id) stream id
 * @param[in] (frame_size) frame size
 * @return Status object.
 */
senscord::Status SkvPlayLibrary::GetStreamFrameSize(
    uint32_t stream_id,
    size_t *frame_size) {
  skv_error_code ec = skv_get_frame_byte_count(
    file_handle_, stream_id, 0, frame_size, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(
      kBlockName,
      senscord::Status::kCauseAborted,
      "SkvIF Error(skv_get_stream_count): %s",
      skv_error_message(ec));
  }
  return senscord::Status::OK();
}

/**
 * @brief set name and stream info pair
 * @param[in] (stream_id) stream id
 * @param[in] (name) unique name of stream
 * @param[in] (skv_stream_info) frame_size and stream_xid
 * @return Status object.
 */
senscord::Status SkvPlayLibrary::SetStreamPairedInfo(
  uint32_t stream_id,  std::string *name, SkvStreamInfo *skv_stream_info) {
  skv_stream_type stream_type;
  skv_error_code ec = skv_get_stream_type(
    file_handle_, stream_id, &stream_type, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
      "SkvIF Error(skv_get_stream_type): %s", skv_error_message(ec));
  }
  if (stream_type == skv_stream_type_image) {
    skv_image_stream_info image_stream_info = {};
    ec = skv_get_image_stream_info(
      file_handle_, stream_id, &image_stream_info, NULL);
    if (ec != skv_error_code_success) {
      return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
        "SkvIF Error(skv_get_image_stream_info): %s", skv_error_message(ec));
    }
    senscord::Status status = GetStreamFrameSize(
      stream_id, &skv_stream_info->frame_size);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    skv_stream_info->width  = image_stream_info.width;
    skv_stream_info->height = image_stream_info.height;
    skv_stream_info->type   = image_stream_info.type;
    *name = image_stream_info.name;
  } else if (stream_type == skv_stream_type_custom) {
    skv_custom_stream_info custom_stream_info = {};
    ec = skv_get_custom_stream_info(
      file_handle_, stream_id, &custom_stream_info, NULL);
    if (ec != skv_error_code_success) {
      return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
        "SkvIF Error(skv_get_custom_stream_info): %s", skv_error_message(ec));
    }
    *name = custom_stream_info.name;
    skv_stream_info->frame_size = custom_stream_info.frame_size;
  }
  skv_stream_info->id = stream_id;
  return senscord::Status::OK();
}

/**
 * @brief get the buffer size and pointer to the data
 * @param[in] (buffer_name) name of custom buffer
 * @param[in] (buffer_size) size of custom buffer
 * @return Status object.
 */
senscord::Status SkvPlayLibrary::GetCustomBufferSize(
    const std::string& buffer_name, size_t* buffer_size) {
  skv_error_code ec = skv_get_custom_buffer_byte_count(
    file_handle_, buffer_name.c_str(), buffer_size, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
      "SkvIF Error(skv_get_custom_buffer_byte_count): %s",
      skv_error_message(ec));
  }
  return senscord::Status::OK();
}

/**
 * @brief get the buffer size and pointer to the data
 * @param[in] (buffer_name) name of custom buffer
 * @param[in] (buffer_data) buffer data
 * @return Status object.
 */
senscord::Status SkvPlayLibrary::GetCustomBufferData(
    const std::string& buffer_name, void* buffer_data) {
  skv_error_code ec = skv_get_custom_buffer_data(
    file_handle_, buffer_name.c_str(), buffer_data, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
      "SkvIF Error(skv_get_custom_buffer_data): %s", skv_error_message(ec));
  }
  return senscord::Status::OK();
  }

/**
 * @brief get stream frame data.
 * @param[in] (stream_id) stream_id
 * @param[in] (frame_inedx) frame index
 * @param[in] (frame_data) frame data
 * @return Status object.
 */
senscord::Status SkvPlayLibrary::GetFrameData(
  uint32_t stream_id, uint32_t frame_index, void* frame_data) {
    skv_error_code ec = skv_get_frame_data(
    file_handle_, stream_id, frame_index, frame_data, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
      "SkvIF Error(skv_get_custom_buffer_data): %s", skv_error_message(ec));
  }
  return senscord::Status::OK();
  }

/**
 * @brief Get all timestamps.
 * @param[in]  (stream_id) stream id
 * @param[out] (time_stamps) series of time stamp of frame
 * @return Status object.
 */
senscord::Status SkvPlayLibrary::GetAllFrameTimestamp(
    uint32_t stream_id,
    std::vector<uint64_t>* time_stamp) {
  if (time_stamp == NULL) {
    return SENSCORD_STATUS_FAIL(
      kBlockName, senscord::Status::kCauseInvalidArgument,
      "Null poineter: GetAllFrameTimestamp");
  }
  std::vector<uint64_t> timestamp_vec;
  // get frame count in the stream
  uint32_t max_frame_count = 0;
  skv_error_code ec = skv_get_stream_frame_count(
    file_handle_, stream_id, &max_frame_count, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
      "SkvIF Error(skv_get_frame_count): %s", skv_error_message(ec));
  }
  if (max_frame_count == 0) {
    return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
    "Error(max_frame_count = 0)");
  }
  for (uint32_t frame_count = 0; frame_count < max_frame_count; ++frame_count) {
    uint64_t timestamp = 0;
    ec = skv_get_frame_timestamp(file_handle_, stream_id, frame_count,
      &timestamp, NULL);
    if (ec != skv_error_code_success) {
      return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
        "SkvIF Error(skv_get_frame_timestamp): %s", skv_error_message(ec));
    }
    timestamp_vec.push_back(timestamp);
  }

  if (timestamp_vec[max_frame_count-1] < timestamp_vec[0]) {
    return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
      "GetAllFrameTimestamp: timestamp is broken");
  }
  *time_stamp = timestamp_vec;
  return senscord::Status::OK();
}
/**
 * @brief Get frame index and its timestamp closest to the given timestamp.
 * @param[in]  (stream_id) stream id
 * @param[in]  (specified_time) secified timestmap
 * @param[out] (frame_index) frame index closest to the given timestamp.
 * @param[out] (time_stamp) time stamp of the frame closest to the given timestamp.
 * @return Status object.
 */
senscord::Status SkvPlayLibrary::GetClosestFrameInfoByTimeStamp(
    uint32_t stream_id,
    uint64_t specified_time,
    uint32_t* frame_index,
    uint64_t* time_stamp) {
  if (frame_index == NULL || time_stamp == NULL) {
  return SENSCORD_STATUS_FAIL(kBlockName,
                            senscord::Status::kCauseInvalidArgument,
                            "Null poineter: GetClosestFrameInfoByTimeStamp");
  }
  uint32_t frame_index_ = 0;  // rollback
  uint64_t time_stamp_  = 0;
  skv_error_code ec = skv_get_frame_index(file_handle_, stream_id,
    specified_time, &frame_index_, NULL);
  if (ec != skv_error_code_success) {
  return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
    "SkvIF Error(skv_get_frame_index): %s", skv_error_message(ec));
  }
  ec = skv_get_frame_timestamp(file_handle_, stream_id,
    frame_index_, &time_stamp_, NULL);
  if (ec != skv_error_code_success) {
  return SENSCORD_STATUS_FAIL(kBlockName, senscord::Status::kCauseAborted,
    "SkvIF Error(skv_get_frame_timestamp): %s", skv_error_message(ec));
  }
  *frame_index = frame_index_;
  *time_stamp  = time_stamp_;
  return senscord::Status::OK();
}

/**
 * @brief Constructor
 */
SkvPlayLibrary::SkvPlayLibrary() : file_handle_(NULL) {}

/**
 * @brief Destructor
 */
SkvPlayLibrary::~SkvPlayLibrary() {}
