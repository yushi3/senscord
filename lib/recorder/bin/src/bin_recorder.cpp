/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "include/bin_recorder.h"
#include <string>
#include <map>
#include "senscord/serialize.h"
#include "senscord/logger.h"
#include "senscord/osal.h"

/**
 * @brief Start to record.
 * @param[in] (path) Top directory path of recording.
 * @param[in] (format) Target format name.
 * @param[in] (stream) Recording stream.
 * @return Status object.
 */
senscord::Status BinRecorder::Start(
    const std::string& path,
    const std::string& format,
    senscord::Stream* stream) {
  output_dir_path_ = path;
  return senscord::Status::OK();
}

/**
 * @brief Stop to record.
 */
void BinRecorder::Stop() {
  delete recorder_;
  recorder_ = NULL;
  is_detected_ = false;
}

/**
 * @brief Write the channel.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
senscord::Status BinRecorder::Write(
    uint64_t sequence_number, uint64_t sent_time,
    const senscord::SerializedChannel& channel) {
  senscord::Status status;
  if (!is_detected_) {
    // first received
    if (!IsImageChannel(channel)) {
      // do nothing (unsupported)
      is_detected_ = true;
      return senscord::Status::OK();
    }

    // get property
    senscord::ImageProperty property = {};
    status = GetImageProperty(channel, &property);
    if (!status.ok()) {
      // do nothing (unsupported channel)
      is_detected_ = true;
      return senscord::Status::OK();
    }

    // create new channel recorder
    BinChannelRecorder* recorder;
    recorder = CreateChannelRecorder(property.pixel_format);
    if (recorder == NULL) {
      // do nothing (unsupported pixel format)
      is_detected_ = true;
      return SENSCORD_STATUS_FAIL("recorder",
          senscord::Status::kCauseNotSupported,
          "unsupported pixel format");
    }

    // initialize
    status = recorder->Init(channel.id, output_dir_path_);
    if (!status.ok()) {
      // do nothing (failed to initialize)
      is_detected_ = true;
      delete recorder;
      return SENSCORD_STATUS_TRACE(status);
    }

    // register
    recorder_ = recorder;
    is_detected_ = true;
  }

  if (is_detected_) {
    if (recorder_ == NULL) {
      // do nothing (unsupported)
    } else {
      senscord::ImageProperty property = {};
      status = GetImageProperty(channel, &property);
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }

      // write
      status = recorder_->Write(sequence_number, property, channel);
      SENSCORD_STATUS_TRACE(status);
    }
  }
  return status;
}

/**
 * @brief Check whether the channel data is image data.
 * @param[in] (channel) Serialized channel data.
 * @return True means image channel.
 */
bool BinRecorder::IsImageChannel(
    const senscord::SerializedChannel& channel) const {
  if (channel.type == senscord::kRawDataTypeImage) {
    return true;
  }
  return false;
}

/**
 * @brief Get the image property from channel.
 * @param[in] (channel) Serialized channel data.
 * @param[out] (property) Image property of channel.
 * @return Status object.
 */
senscord::Status BinRecorder::GetImageProperty(
    const senscord::SerializedChannel& channel,
    senscord::ImageProperty* property) const {
  // search property
  std::map<std::string, senscord::BinaryProperty>::const_iterator found =
      channel.properties.find(senscord::kImagePropertyKey);
  if (found == channel.properties.end()) {
    return SENSCORD_STATUS_FAIL("recorder", senscord::Status::kCauseNotFound,
        "property not found.");
  }
  senscord::BinaryProperty binary = found->second;

  // convert
  senscord::serialize::Decoder dec(&binary.data[0], binary.data.size());
  senscord::Status status = dec.Pop(*property);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Create new channel recorder.
 * @param[in] (pixel_format) Pixel format of channel.
 * @return New channel recorder.
 */
BinChannelRecorder* BinRecorder::CreateChannelRecorder(
    const std::string& pixel_format) const {
  if ((pixel_format == senscord::kPixelFormatGREY) ||
      (pixel_format == senscord::kPixelFormatNV12) ||
      (pixel_format == senscord::kPixelFormatNV16)) {
    // 8-bit input (only Y plane)
    return new BinChannelRecorder8bit();
  } else if ((pixel_format == senscord::kPixelFormatY10) ||
             (pixel_format == senscord::kPixelFormatY12) ||
             (pixel_format == senscord::kPixelFormatY16)) {
    // 16-bit input
    return new BinChannelRecorder();
  }
  return NULL;
}

/**
 * @brief Constructor
 */
BinRecorder::BinRecorder()
    : is_detected_(false), output_dir_path_(), recorder_() {}

/**
 * @brief Destructor
 */
BinRecorder::~BinRecorder() {
  delete recorder_;
  recorder_ = NULL;
}
