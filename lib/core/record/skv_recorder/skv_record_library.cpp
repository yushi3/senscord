/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/skv_recorder/skv_record_library.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <utility>
#include "senscord/serialize.h"
#include "senscord/logger.h"
#include "senscord/osal.h"

namespace senscord {
/**
 * @brief Create skv file.
 * @param[in] (path) skv file output path.
 * @return Status object.
 */
Status SkvRecordLibrary::CreateFile(const std::string& path) {
  if (file_handle_ != NULL) {
    // Created file
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseAlreadyExists,
        "existed skv file handle");
  }

  skv_error_code ec = skv_create_file(&file_handle_, path.c_str(), NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "SkvIF Error(create_file): %s", skv_error_message(ec));
  } else if (file_handle_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "SkvIF Error(create_file): create file failure");
  }

  return Status::OK();
}

/**
 * @brief Close skv file.
 * @return Status object.
 */
Status SkvRecordLibrary::CloseFile() {
  if (file_handle_ == NULL) {
    // closed
    return Status::OK();
  }

  skv_close_file(file_handle_);
  file_handle_ = NULL;
  return Status::OK();
}

/**
 * @brief Add ImageStream to skv file.
 * @param[in] (stream_name) Name of ImageStream.
 * @param[in] (pixel_type) pixel type of frame.
 * @param[in] (width) width of frame.
 * @param[in] (height) height of frame.
 * @param[out] (stream_id) id of added ImageStream.
 * @return Status object.
 */
Status SkvRecordLibrary::AddImageStream(
    const SkvImageStreamInfo& info, uint32_t *stream_id) {
  // set stream info
  skv_image_stream_info image_stream_info = {};
  skv_assign_image_stream_info(
      &image_stream_info,
      info.name.c_str(),
      static_cast<skv_image_type>(info.type),
      static_cast<skv_compression>(kSkvDefaultCompressionType),
      info.width,
      info.height);

  // add stream
  skv_error_code ec = skv_add_image_stream(
      file_handle_, &image_stream_info, stream_id, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "SkvIF Error(add_image_stream): %s", skv_error_message(ec));
  }

  return Status::OK();
}

/**
 * @brief Set intrinsics to skv file.
 * @param[in] (stream_id) stream id.
 * @param[in] (width) width of frame.
 * @param[in] (height) height of frame.
 * @param[out] (calibration) calibration of record stream.
 * @return Status object.
 */
Status SkvRecordLibrary::SetIntrinsicsModel(
    uint32_t stream_id, uint32_t width, uint32_t height,
    const CameraCalibrationParameters& calibration) {
  // set distortion model
  skv_distortion_model distortion = {};
  distortion.fx = calibration.intrinsic.fx;
  distortion.fy = calibration.intrinsic.fy;
  distortion.k1 = calibration.distortion.k1;
  distortion.k2 = calibration.distortion.k2;
  distortion.k3 = calibration.distortion.k3;
  distortion.k4 = calibration.distortion.k4;
  distortion.p1 = calibration.distortion.p1;
  distortion.p2 = calibration.distortion.p2;
  skv_error_code ec =
      skv_set_distortion_model(file_handle_, stream_id, &distortion, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "SkvIF Error(set_distortion_model): %s", skv_error_message(ec));
  }

  // set pinhole model
  skv_pinhole_model pinhole = {};
  pinhole.fovx = static_cast<float>(
      2.0 * atan(width / (2.0 * calibration.intrinsic.fx)) * 180.0 / M_PI);
  pinhole.fovy = static_cast<float>(
      2.0 * atan(height / (2.0 * calibration.intrinsic.fy)) * 180.0 / M_PI);
  pinhole.cx = calibration.intrinsic.cx;
  pinhole.cy = calibration.intrinsic.cy;
  ec = skv_set_pinhole_model(file_handle_, stream_id, &pinhole, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "SkvIF Error(set_pinhole_model): %s", skv_error_message(ec));
  }

  return Status::OK();
}
/**
 * @brief Add CustomStream to skv file.
 * @param[in] (stream_name) Name of CustomStream.
 * @param[in] (frame_size) size of frame.
 * @param[out] (stream_id) id of added CustomStream.
 * @return Status object.
 */
Status SkvRecordLibrary::AddCustomStream(
    const SkvCustomStreamInfo& info, uint32_t *stream_id) {
  skv_custom_stream_info custom_stream_info = {};
  skv_assign_custom_stream_info(
      &custom_stream_info, info.name.c_str(),
      static_cast<skv_compression>(kSkvDefaultCompressionType),
      info.size);
  skv_error_code ec =
    skv_add_custom_stream(file_handle_, &custom_stream_info, stream_id, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "SkvIF Error(add_custom_stream): %s", skv_error_message(ec));
  }

  return Status::OK();
}

/**
 * @brief Add frame to ImageStream or CustomStream.
 * @param[in] (stream_id) id of add stream.
 * @param[in] (sent_time) frame sent time.
 * @param[in] (data) write data.
 * @return Status object.
 */
Status SkvRecordLibrary::AddFrame(
    const uint32_t stream_id, const uint64_t sent_time,
    const void* buffer, const size_t size) {
  // Convert from nanoseconds to microseconds
  uint64_t time_stamp = static_cast<uint64_t>(sent_time / 1000);

  std::map<uint32_t, uint64_t>::iterator found =
      last_add_frame_time_.find(stream_id);
  if (found != last_add_frame_time_.end()) {
    // If the time stamps are the same, do not write
    if (found->second == time_stamp) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
          "AddFrame Error: Frame of the same time already exists. "
          "stream_id=%" PRIu32 ", sent_time=%" PRIu64 ", time_stamp=%" PRIu64,
          stream_id, sent_time, time_stamp);
    }
  }

  skv_error_code ec = skv_add_frame(
      file_handle_, stream_id, time_stamp, buffer, size, NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "SkvIF Error(add_frame): %s", skv_error_message(ec));
  }

  // Keep time stamp of write frame
  if (found == last_add_frame_time_.end()) {
    // First write
    last_add_frame_time_.insert(std::make_pair(stream_id, time_stamp));
  } else {
    // Subsequent writing
    found->second = time_stamp;
  }

  return Status::OK();
}

/**
 * @brief Add CustomBuffer.
 * @param[in] (buffer_name) name of buffer.
 * @param[in] (data) write data.
 * @return Status object.
 */
Status SkvRecordLibrary::AddCustomBuffer(
    const std::string& buffer_name,
    const void* buffer, const size_t size) {
  skv_error_code ec = skv_add_custom_buffer(
      file_handle_, buffer_name.c_str(),
      buffer, size,
      static_cast<skv_compression>(kSkvDefaultCompressionType), NULL);
  if (ec != skv_error_code_success) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "SkvIF Error(add_custom_buffer): %s", skv_error_message(ec));
  }
  return Status::OK();
}

/**
 * @brief Constructor
 */
SkvRecordLibrary::SkvRecordLibrary() :
    file_handle_(NULL), last_add_frame_time_() {}

/**
 * @brief Destructor
 */
SkvRecordLibrary::~SkvRecordLibrary() {}

}   // namespace senscord
