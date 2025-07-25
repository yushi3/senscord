/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "include/pnm_recorder.h"

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
senscord::Status PnmRecorder::Start(
    const std::string& path,
    const std::string& format,
    senscord::Stream* stream) {
  output_dir_path_ = path;
  return senscord::Status::OK();
}

/**
 * @brief Stop to record.
 */
void PnmRecorder::Stop() {
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
senscord::Status PnmRecorder::Write(
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
    PnmChannelRecorder* recorder;
    recorder = CreateChannelRecorder(property.pixel_format);
    if (recorder == NULL) {
      // do nothing (unsupported pixel format)
      is_detected_ = true;
      return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
          senscord::Status::kCauseNotSupported,
          "unsupported pixel format: \"%s\"", property.pixel_format.c_str());
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
bool PnmRecorder::IsImageChannel(
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
senscord::Status PnmRecorder::GetImageProperty(
    const senscord::SerializedChannel& channel,
    senscord::ImageProperty* property) const {
  // search property
  std::map<std::string, senscord::BinaryProperty>::const_iterator found =
      channel.properties.find(senscord::kImagePropertyKey);
  if (found == channel.properties.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
        senscord::Status::kCauseNotFound, "property not found.");
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
PnmChannelRecorder* PnmRecorder::CreateChannelRecorder(
    const std::string& pixel_format) const {
  if (pixel_format == senscord::kPixelFormatGREY) {
    // 8-bit input
    return new PgmChannelRecorder8bit();
  } else if ((pixel_format == senscord::kPixelFormatY10) ||
             (pixel_format == senscord::kPixelFormatY12) ||
             (pixel_format == senscord::kPixelFormatY14) ||
             (pixel_format == senscord::kPixelFormatY16)) {
    // 16-bit input
    return new PgmChannelRecorder16bit();
  } else if (pixel_format == senscord::kPixelFormatRGB24) {
    // 8-bit input
    return new PpmChannelRecorder();
  }

  return NULL;
}

/**
 * @brief Constructor
 */
PnmRecorder::PnmRecorder()
    : is_detected_(false), output_dir_path_(), recorder_() {}

/**
 * @brief Destructor
 */
PnmRecorder::~PnmRecorder() {
  Stop();
}
