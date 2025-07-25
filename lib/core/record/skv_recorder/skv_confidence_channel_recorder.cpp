/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/skv_recorder/skv_confidence_channel_recorder.h"

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <map>

#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"
#include "senscord/status.h"
#include "senscord/logger.h"

namespace senscord {

/**
 * @brief Constructor
 */
SkvConfidenceChannelRecorder::SkvConfidenceChannelRecorder()
    : rawdata_stream_info_(), property_stream_info_() {}

/**
 * @brief Destructor
 */
SkvConfidenceChannelRecorder::~SkvConfidenceChannelRecorder() {}

/**
 * @brief Initialize for writing the channel.
 * @param[in] (channel) record channel.
 * @param[in] (library) skv library.
 * @return Status object.
 */
Status SkvConfidenceChannelRecorder::Init(
    const SerializedChannel& channel, SkvRecordLibrary* library) {
  if (library == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // get image property
  ConfidenceProperty property = {};
  Status status = GetChannelPropertyFromSerializedChannel(
      kConfidencePropertyKey, channel, &property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Add ImageStream for rawdata record
  SkvImageStreamInfo rawdata_info = {};
  {
    // set ImageStreamInfo
    status = SetRawDataStreamInfo(property, &rawdata_info);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    // add ImageStream to skv file
    uint32_t stream_id;
    status = library->AddImageStream(rawdata_info, &stream_id);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    rawdata_info.stream_id = stream_id;
  }

  // Add CustomStream for property record
  SkvCustomStreamInfo property_info = {};
  {
    // Get channel property size
    size_t buffer_size = GetChannelPropertyBufferSize(channel);

    // set CustomStreamInfo
    status = SetPropertyStreamInfo(
        rawdata_info.name, buffer_size, &property_info);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    // add CustomStream to skv file
    uint32_t stream_id;
    status = library->AddCustomStream(property_info, &stream_id);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    property_info.stream_id = stream_id;
  }

  // Set IntrinsicModel (Optional Parameter)
  {
    // Get camera calibration parameter.
    CameraCalibrationParameters parameter = {};
    status = GetCameraCalibrationParameter(library, &parameter);
    if (status.ok()) {
      // Set intrinsics model to ImageStream.
      status = library->SetIntrinsicsModel(rawdata_info.stream_id,
          rawdata_info.width, rawdata_info.height, parameter);
    }

    if (!status.ok()) {
      SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
      // This parameter is not mandatory,
      // so the recording process will continue.
    }
  }

  // keep record information
  rawdata_stream_info_  = rawdata_info;
  property_stream_info_ = property_info;
  skv_record_library_   = library;

  return Status::OK();
}

/**
 * @brief Write the channel data to file.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status SkvConfidenceChannelRecorder::WriteRawData(
    uint64_t sent_time, const SerializedChannel& channel) {
  // write rawdata
  Status status = skv_record_library_->AddFrame(
      rawdata_stream_info_.stream_id, sent_time,
      &channel.rawdata[0], channel.rawdata.size());
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return Status::OK();
}

/**
 * @brief Write the channel property to file.
 * @param[in] (sequence_number) sequence number of record frame.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status SkvConfidenceChannelRecorder::WriteProperty(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel) {
  // set write data
  ChannelPropertiesForRecord rec = {};
  rec.sequence_number = sequence_number;
  rec.properties = channel.properties;

  // serialize
  serialize::SerializedBuffer buf;
  serialize::Encoder enc(&buf);
  enc.Push(rec);

  // Convert to fix size
  std::vector<uint8_t> buf_resize;
  buf_resize.resize(property_stream_info_.size);
  osal::OSMemcpy(
      &buf_resize[0], buf_resize.size(), buf.data(), buf.size());

  // write channel property
  Status status = skv_record_library_->AddFrame(
      property_stream_info_.stream_id,
      sent_time, &buf_resize[0], buf_resize.size());
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return Status::OK();
}

/**
 * @brief set stream info for rawdata stream.
 * @param[in] (property) property of record channel.
 * @param[out] (info) stream info of rawdata written stream.
 * @return Status object.
 */
Status SkvConfidenceChannelRecorder::SetRawDataStreamInfo(
    const ConfidenceProperty& property, SkvImageStreamInfo* info) {
  if (info == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  uint32_t pixel_size;

  // set name and type each pixel format
  if (property.pixel_format == kPixelFormatC16) {
    info->name = kSkvStreamConfidence;
    info->type = SKV_IMAGE_TYPE_INT16;
    pixel_size = sizeof(int16_t);
  } else if (property.pixel_format == kPixelFormatC32F) {
    info->name = kSkvStreamFloatConfidence;
    info->type = SKV_IMAGE_TYPE_FLOAT;
    pixel_size = sizeof(float);
  } else {
    return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
        Status::kCauseNotSupported,
        "unsupported pixel format: \"%s\"", property.pixel_format.c_str());
  }

  // The width of resolution is calculated from stride.
  // Because rawdata with stride is written to the skv file.
  info->width  = property.stride_bytes / pixel_size;
  info->height = property.height;

  return Status::OK();
}

/**
 * @brief set stream info for property stream.
 * @param[in] (stream_name) name of rawdata  written stream.
 * @param[in] (buffer_size) size of serialized channel properties.
 * @param[in] (info) stream info of property written stream.
 * @return Status object.
 */
Status SkvConfidenceChannelRecorder::SetPropertyStreamInfo(
    const std::string& stream_name, size_t buffer_size,
    SkvCustomStreamInfo* info) {
  if (info == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // name : "senscord_channel_property_<StreamName>"
  info->name = kSkvStreamChannelPropertyPrefix + stream_name;
  info->size = buffer_size;
  return Status::OK();
}

}   // namespace senscord
