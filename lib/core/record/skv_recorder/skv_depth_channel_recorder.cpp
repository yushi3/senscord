/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/skv_recorder/skv_depth_channel_recorder.h"

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
SkvDepthChannelRecorder::SkvDepthChannelRecorder()
    : rawdata_stream_info_(), property_stream_info_(),
      temperature_property_stream_info_(),
      frame_extension_property_stream_info_(),
      exposure_property_stream_info_(),
      is_recordable_temperature_property_(false),
      is_recordable_frame_extension_property_(false),
      is_recordable_exposure_property_(false),
      is_recordable_skv_write_property_(false) {}

/**
 * @brief Destructor
 */
SkvDepthChannelRecorder::~SkvDepthChannelRecorder() {}

/**
 * @brief Initialize for writing the channel.
 * @param[in] (channel) record channel.
 * @param[in] (library) skv library.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::Init(
    const SerializedChannel& channel, SkvRecordLibrary* library) {
  if (library == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // Set StreamInfo and add ImageStream for rawdata stream.
  SkvImageStreamInfo rawdata_info = {};
  Status status = SetupRawDataStream(library, channel, &rawdata_info);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Set StreamInfo and add CustomStream for channel properties.
  SkvCustomStreamInfo property_info = {};
  status = SetupPropertyStream(
      channel, library, rawdata_info.name, &property_info);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Set StreamInfo and add CustomStream for temperature property.
  std::vector<SkvCustomStreamInfo> temperature_info;
  status = SetupTemperaturePropertyStream(library, channel, &temperature_info);
  if (status.ok()) {
    is_recordable_temperature_property_ = true;
  } else {
    SENSCORD_LOG_DEBUG("%s", status.ToString().c_str());
    // This Property is not mandatory,
    // so the recording process will continue.
  }

  // Set StreamInfo and add CustomStream for exposure property.
  SkvCustomStreamInfo exposure_info = {};
  status = SetupExposurePropertyStream(library, channel, &exposure_info);
  if (status.ok()) {
    is_recordable_exposure_property_ = true;
  } else {
    SENSCORD_LOG_DEBUG("%s", status.ToString().c_str());
    // This Property is not mandatory,
    // so the recording process will continue.
  }

  // Set StreamInfo and add CustomStream for skv write property.
  std::vector<SkvCustomStreamInfo> skv_write_info;
  status = SetupSkvWritePropertyStream(
      library, channel, &skv_write_info);
  if (status.ok()) {
    is_recordable_skv_write_property_ = true;
  } else {
    SENSCORD_LOG_DEBUG("%s", status.ToString().c_str());
    // This Property is not mandatory,
    // so the recording process will continue.
  }

  // keep record information
  rawdata_stream_info_  = rawdata_info;
  property_stream_info_ = property_info;
  skv_record_library_   = library;
  if (is_recordable_temperature_property_) {
    temperature_property_stream_info_ = temperature_info;
  }
  if (is_recordable_exposure_property_) {
    exposure_property_stream_info_ = exposure_info;
  }
  if (is_recordable_skv_write_property_) {
    skv_write_property_stream_info_ = skv_write_info;
  }
  return Status::OK();
}

/**
 * @brief Write the channel data to file.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::WriteRawData(
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
 * @brief Write the channel property.
 * @param[in] (sequence_number) sequence number of record frame.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::WriteProperty(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel) {
  // Write channel property.
  Status status = WriteChannelProperty(sequence_number, sent_time, channel);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Write temperature property members.
  if (is_recordable_temperature_property_) {
    status = WriteTemperatureProperty(sent_time, channel);
    if (!status.ok()) {
      SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
      // This Property is not mandatory,
      // so the recording process will continue.
      is_recordable_temperature_property_ = false;
    }
  }

  // Write exposure property member.
  if (is_recordable_exposure_property_) {
    status = WriteExposureProperty(sent_time, channel);
    if (!status.ok()) {
      SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
      // This Property is not mandatory,
      // so the recording process will continue.
      is_recordable_exposure_property_ = false;
    }
  }

  // Write skv wirte property members.
  if (is_recordable_skv_write_property_) {
    status = WriteSkvWriteProperty(sent_time, channel);
    if (!status.ok()) {
      SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
      // This Property is not mandatory,
      // so the recording process will continue.
      is_recordable_skv_write_property_ = false;
    }
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
Status SkvDepthChannelRecorder::WriteChannelProperty(
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
 * @brief Write the temperature property to file.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::WriteTemperatureProperty(
  uint64_t sent_time, const SerializedChannel& channel) {
  // Get temperature property.
  TemperatureProperty property = {};
  Status status = GetChannelPropertyFromSerializedChannel(
      kTemperaturePropertyKey, channel, &property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  std::vector<SkvCustomStreamInfo>::iterator itr =
      temperature_property_stream_info_.begin();
  for (; itr != temperature_property_stream_info_.end(); ++itr) {
    void* member_value = NULL;
    if (itr->name == kSkvStreamLaserTemperature) {
      member_value = &(property.temperatures[0].temperature);
    } else if (itr->name == kSkvStreamSensorTemperature) {
      member_value = &(property.temperatures[1].temperature);
    } else {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseAborted, "Unexpected name: \"%s\"",
          itr->name.c_str());
    }

    status = skv_record_library_->AddFrame(
        itr->stream_id, sent_time, member_value, itr->size);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }

  return Status::OK();
}

/**
 * @brief Write the exposure property to file.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::WriteExposureProperty(
  uint64_t sent_time, const SerializedChannel& channel) {
  // Get exposure property.
  ExposureProperty property = {};
  Status status = GetChannelPropertyFromSerializedChannel(
      kExposurePropertyKey, channel, &property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  status = skv_record_library_->AddFrame(
      exposure_property_stream_info_.stream_id, sent_time,
      &property.exposure_time, exposure_property_stream_info_.size);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return Status::OK();
}

/**
 * @brief Write the skv write property to file.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::WriteSkvWriteProperty(
  uint64_t sent_time, const SerializedChannel& channel) {
  // Get frame extension property.
  SkvWriteProperty property = {};
  Status status = GetChannelPropertyFromSerializedChannel(
      kSkvWritePropertyKey, channel, &property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Write skv write property member.
  for (std::vector<SkvCustomStreamInfo>::iterator
      itr = skv_write_property_stream_info_.begin(),
      end = skv_write_property_stream_info_.end();
      itr != end; ++itr) {
    std::map<std::string, SkvWriteData>::iterator found;
    found = property.write_list.find(itr->name);
    if (found == property.write_list.end()) {
      continue;
    }

    void* member_value = &(found->second.data[0]);

    status = skv_record_library_->AddFrame(
        itr->stream_id, sent_time, member_value, itr->size);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }

  return Status::OK();
}

/**
 * @brief Set StreamInfo and add ImageStream for raw data.
 * @param[in] (library) skv library.
 * @param[in] (channel) record channel.
 * @param[out] (stream_info) stream info of rawdata written stream.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::SetupRawDataStream(
    SkvRecordLibrary* library, const SerializedChannel& channel,
    SkvImageStreamInfo* stream_info) {
  if (library == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  if (stream_info == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // Get image property.
  ImageProperty property = {};
  Status status = GetChannelPropertyFromSerializedChannel(
      kImagePropertyKey, channel, &property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Set ImageStreamInfo
  status = SetRawDataStreamInfo(property, stream_info);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Add ImageStream to skv file
  uint32_t stream_id;
  status = library->AddImageStream(*stream_info, &stream_id);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  stream_info->stream_id = stream_id;

  // Set IntrinsicModel (Optional Parameter)
  {
    // Get camera calibration parameter.
    CameraCalibrationParameters parameter = {};
    status = GetCameraCalibrationParameter(library, &parameter);
    if (status.ok()) {
      // Set intrinsics model to ImageStream.
      status = library->SetIntrinsicsModel(stream_info->stream_id,
          stream_info->width, stream_info->height, parameter);
    }

    if (!status.ok()) {
      SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
      // This parameter is not mandatory,
      // so the recording process will continue.
    }
  }

  return Status::OK();
}

/**
 * @brief set stream info for rawdata stream.
 * @param[in] (property) property of record channel.
 * @param[out] (info) stream info of rawdata written stream.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::SetRawDataStreamInfo(
    const ImageProperty& property, SkvImageStreamInfo* info) {
  if (info == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  uint32_t pixel_size;

  // set name and type each pixel format
  if (property.pixel_format == kPixelFormatZ16) {
    info->name = kSkvStreamDepth;
    info->type = SKV_IMAGE_TYPE_INT16;
    pixel_size = sizeof(int16_t);
  } else if (property.pixel_format == kPixelFormatZ32F) {
    info->name = kSkvStreamDepthFloat;
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
 * @brief Set StreamInfo and add CustomStream for channel properties.
 * @param[in] (channel) Serialized channel data.
 * @param[in] (library) skv library.
 * @param[in] (stream_name) name of rawdata  written stream.
 * @param[out] (stream_info) stream info of property written stream.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::SetupPropertyStream(
    const SerializedChannel& channel, SkvRecordLibrary* library,
    const std::string& stream_name, SkvCustomStreamInfo* info) {
  if (library == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  if (info == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // Get channel property size
  size_t buffer_size = GetChannelPropertyBufferSize(channel);

  // Set CustomStreamInfo for channel property.
  Status status = SetPropertyStreamInfo(stream_name, buffer_size, info);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Add CustomStream to skv file
  uint32_t stream_id;
  status = library->AddCustomStream(*info, &stream_id);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  info->stream_id = stream_id;

  return Status::OK();
}

/**
 * @brief set stream info for property stream.
 * @param[in] (stream_name) name of rawdata written stream.
 * @param[in] (buffer_size) size of serialized channel properties.
 * @param[out] (info) stream info of property written stream.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::SetPropertyStreamInfo(
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

/**
 * @brief Set StreamInfo and add CustomStream for TemperatureProperty.
 * @param[in] (library) skv library.
 * @param[in] (channel) record channel.
 * @param[out] (info_list) Stream info of TemperatureProperty members.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::SetupTemperaturePropertyStream(
    SkvRecordLibrary* library, const SerializedChannel& channel,
    std::vector<SkvCustomStreamInfo>* info_list) {
  if (library == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  if (info_list == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // Get temperature property.
  TemperatureProperty temperature_property = {};
  Status status = GetChannelPropertyFromSerializedChannel(
      kTemperaturePropertyKey, channel, &temperature_property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Add CustomStream for TemperatureProperty members record.
  status = SetTemperaturePropertyStreamInfo(
      temperature_property, info_list);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Add CustomStream to skv file
  for (std::vector<SkvCustomStreamInfo>::iterator
      itr = info_list->begin(), end = info_list->end();
      itr != end; ++itr) {
    uint32_t stream_id;
    status = library->AddCustomStream(*itr, &stream_id);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    itr->stream_id = stream_id;
  }

  return Status::OK();
}

/**
 * @brief Set stream info for TemperatureProperty.
 * @param[in] (property) TemperatureProperty of parent stream.
 * @param[out] (info_list) Stream info of TemperatureProperty members.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::SetTemperaturePropertyStreamInfo(
    const TemperatureProperty& property,
    std::vector<SkvCustomStreamInfo>* info_list) {
  if (info_list == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // temperatures[0] = LaserTemperature
  std::map<uint32_t, TemperatureInfo>::const_iterator found =
      property.temperatures.find(0);
  if (found == property.temperatures.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "%s not found.", kSkvStreamLaserTemperature);
  }
  SkvCustomStreamInfo info = {};
  info.name = kSkvStreamLaserTemperature;
  info.size = sizeof(found->second.temperature);
  info_list->push_back(info);

  // temperatures[1] = SensorTemperature
  found = property.temperatures.find(1);
  if (found == property.temperatures.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "%s not found.", kSkvStreamSensorTemperature);
  }
  info.name = kSkvStreamSensorTemperature;
  info.size = sizeof(found->second.temperature);
  info_list->push_back(info);

  return Status::OK();
}

/**
 * @brief Set StreamInfo and add CustomStream for SkvWriteProperty.
 * @param[in] (library) skv library.
 * @param[in] (channel) record channel.
 * @param[out] (info_list) Stream info of SkvWriteProperty members.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::SetupSkvWritePropertyStream(
    SkvRecordLibrary* library, const SerializedChannel& channel,
    std::vector<SkvCustomStreamInfo>* info_list) {
  if (library == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  if (info_list == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // Get skv write property from channel.
  SkvWriteProperty property = {};
  Status status = GetChannelPropertyFromSerializedChannel(
      kSkvWritePropertyKey, channel, &property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  for (std::map<std::string, SkvWriteData>::iterator
      itr = property.write_list.begin(), end = property.write_list.end();
      itr != end; ++itr) {
    const std::string& name = itr->first;
    const SkvWriteData& write = itr->second;
    if (name.empty() || (name.size() > 255)) {
      continue;  // not record
    }

    if (write.type == kSkvRecordTypeCustomStream) {
      // Set custom stream information.
      SkvCustomStreamInfo info = {};
      info.name = itr->first;
      info.size = write.data.size();

      info_list->push_back(info);
    } else if (write.type == kSkvRecordTypeCustomBuffer) {
      // Write custom buffer.
      status = library->AddCustomBuffer(name,
          &write.data[0], write.data.size());
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
    }
  }

  // Add SkvWriteProperty members to CustomStream.
  std::vector<SkvCustomStreamInfo>::iterator itr = info_list->begin();
  for (; itr != info_list->end(); ++itr) {
    uint32_t stream_id;
    status = library->AddCustomStream(*itr, &stream_id);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    itr->stream_id = stream_id;
  }

  return Status::OK();
}

/**
 * @brief Set StreamInfo and add CustomStream for ExposureProperty.
 * @param[in] (library) skv library.
 * @param[in] (channel) record channel.
 * @param[out] (info) Stream info of ExposureProperty members.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::SetupExposurePropertyStream(
    SkvRecordLibrary* library, const SerializedChannel& channel,
    SkvCustomStreamInfo* info) {
  if (library == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  if (info == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // Get exposure property.
  ExposureProperty exposure_property = {};
  Status status = GetChannelPropertyFromSerializedChannel(
      kExposurePropertyKey, channel, &exposure_property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Add CustomStream for ExposureProperty member record.
  status = SetExposurePropertyStreamInfo(
      exposure_property, info);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Add CustomStream to skv file.
  uint32_t stream_id;
  status = library->AddCustomStream(*info, &stream_id);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  info->stream_id = stream_id;

  return Status::OK();
}

/**
 * @brief Set stream info for ExposureProperty.
 * @param[in] (property) ExposureProperty of parent stream.
 * @param[out] (info) Stream info of ExposureProperty members.
 * @return Status object.
 */
Status SkvDepthChannelRecorder::SetExposurePropertyStreamInfo(
    const ExposureProperty& property,
    SkvCustomStreamInfo* info) {
  if (info == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  info->name = kSkvStreamExposure;
  info->size = sizeof(property.exposure_time);

  return Status::OK();
}

}   // namespace senscord
