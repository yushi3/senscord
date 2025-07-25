/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/skv_recorder/skv_pointcloud_channel_recorder.h"

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

namespace {
  // Offset value to convert from planar to packed
  const size_t kOffsetAxisX = 0;
  const size_t kOffsetAxisY = 1;
  const size_t kOffsetAxisZ = 2;
}  // namespace

namespace senscord {

/**
 * @brief Constructor
 */
SkvPointCloudChannelRecorder::SkvPointCloudChannelRecorder()
    : rawdata_stream_info_(), property_stream_info_() {}

/**
 * @brief Destructor
 */
SkvPointCloudChannelRecorder::~SkvPointCloudChannelRecorder() {}

/**
 * @brief Initialize for writing the channel.
 * @param[in] (channel) record channel.
 * @param[in] (library) skv library.
 * @return Status object.
 */
Status SkvPointCloudChannelRecorder::Init(
    const SerializedChannel& channel, SkvRecordLibrary* library) {
  if (library == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // get pointcloud property
  PointCloudProperty property = {};
  Status status = GetChannelPropertyFromSerializedChannel(
      kPointCloudPropertyKey, channel, &property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Add CustomStream for rawdata record
  SkvCustomStreamInfo rawdata_info = {};
  {
    // set CustomStreamInfo
    status = SetRawDataStreamInfo(property, &rawdata_info);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    // add CustomStream to skv file
    uint32_t stream_id;
    status = library->AddCustomStream(rawdata_info, &stream_id);
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

  // keep record information
  rawdata_stream_info_  = rawdata_info;
  property_stream_info_ = property_info;
  skv_record_library_   = library;
  point_cloud_          = property;
  return Status::OK();
}

/**
 * @brief Write the channel data to file.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status SkvPointCloudChannelRecorder::WriteRawData(
    uint64_t sent_time, const SerializedChannel& channel) {
  if ((point_cloud_.pixel_format == kPixelFormatXYZ16Planar) ||
      (point_cloud_.pixel_format == kPixelFormatXYZ32FPlanar)) {
    // Convert to Packed
    std::vector<uint8_t> packed;
    if (point_cloud_.pixel_format == kPixelFormatXYZ16Planar) {
      ConvertChannelRawData(channel.rawdata, &packed, kPointCloudXyz16Bpp);
    } else {
      ConvertChannelRawData(channel.rawdata, &packed, kPointCloudXyz32Bpp);
    }

    // write rawdata
    Status status = skv_record_library_->AddFrame(
        rawdata_stream_info_.stream_id, sent_time,
        packed.data(), channel.rawdata.size());
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  } else {
    // write rawdata
    Status status = skv_record_library_->AddFrame(
        rawdata_stream_info_.stream_id, sent_time,
        &channel.rawdata[0], channel.rawdata.size());
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
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
Status SkvPointCloudChannelRecorder::WriteProperty(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel) {
  // set write data
  ChannelPropertiesForRecord rec = {};
  rec.sequence_number = sequence_number;
  rec.properties = channel.properties;

  // Convert pixel format from planar to packed
  if ((point_cloud_.pixel_format == kPixelFormatXYZ16Planar) ||
      (point_cloud_.pixel_format == kPixelFormatXYZ32FPlanar)) {
    std::map<std::string, BinaryProperty>::iterator found =
        rec.properties.find(senscord::kPointCloudPropertyKey);
    if (found != rec.properties.end()) {
      ConvertPixelFormat(&(found->second));
    }
  }

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
Status SkvPointCloudChannelRecorder::SetRawDataStreamInfo(
    const PointCloudProperty& property, SkvCustomStreamInfo* info) {
  if (info == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // set name and type each pixel format
  uint32_t pixel_size;
  if ((property.pixel_format == kPixelFormatXYZ32F) ||
      (property.pixel_format == kPixelFormatXYZ32FPlanar)) {
    info->name = kSkvStreamPointcloudFloat;
    pixel_size = kPointCloudXyzPlane * kPointCloudXyz32Bpp;
  } else if ((property.pixel_format == kPixelFormatXYZ16) ||
             (property.pixel_format == kPixelFormatXYZ16Planar)) {
    info->name = kSkvStreamPointcloud;
    pixel_size = kPointCloudXyzPlane * kPointCloudXyz16Bpp;
  } else {
    return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
        Status::kCauseNotSupported,
        "unsupported pixel format: \"%s\"", property.pixel_format.c_str());
  }

  // set frame size
  info->size = property.width * property.height * pixel_size;

  return Status::OK();
}

/**
 * @brief set stream info for property stream.
 * @param[in] (stream_name) name of rawdata  written stream.
 * @param[in] (buffer_size) size of serialized channel properties.
 * @param[out] (info) stream info of property written stream.
 * @return Status object.
 */
Status SkvPointCloudChannelRecorder::SetPropertyStreamInfo(
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
 * @brief Convert channel rawdata from planar to packed.
 * @param[in] (planar) Byte array of planar rawdata.
 * @param[out] (packed) Byte array of packed rawdata.
 * @param[in] (pixel_bytes) Bytes of pixel.
 * @return Status object.
 */
Status SkvPointCloudChannelRecorder::ConvertChannelRawData(
    const std::vector<uint8_t>& planar,
    std::vector<uint8_t>* packed,
    const size_t pixel_bytes) {
  // Set plane offset
  size_t plane_size = pixel_bytes * point_cloud_.width * point_cloud_.height;
  size_t plane_x = kOffsetAxisX * plane_size;
  size_t plane_y = kOffsetAxisY * plane_size;
  size_t plane_z = kOffsetAxisZ * plane_size;

  // Set dst buffer size
  packed->resize(planar.size());

  // Set the src and dst address
  uint8_t* dst = &(*packed)[0];
  const uint8_t* src_x = &(planar[plane_x]);
  const uint8_t* src_y = &(planar[plane_y]);
  const uint8_t* src_z = &(planar[plane_z]);

  size_t size = packed->size();
  if (pixel_bytes == kPointCloudXyz16Bpp) {
    while (size != 0) {
      // Copying the X-axis rawdata
      dst[0] = src_x[0];
      dst[1] = src_x[1];
      src_x += pixel_bytes;

      // Copying the Y-axis rawdata
      dst[2] = src_y[0];
      dst[3] = src_y[1];
      src_y += pixel_bytes;

      // Copying the Z-axis rawdata
      dst[4] = src_z[0];
      dst[5] = src_z[1];
      src_z += pixel_bytes;

      dst += pixel_bytes * kPointCloudXyzPlane;
      size -= pixel_bytes * kPointCloudXyzPlane;
    }
  } else {
    while (size != 0) {
      // Copying the X-axis rawdata
      dst[0] = src_x[0];
      dst[1] = src_x[1];
      dst[2] = src_x[2];
      dst[3] = src_x[3];
      src_x += pixel_bytes;

      // Copying the Y-axis rawdata
      dst[4] = src_y[0];
      dst[5] = src_y[1];
      dst[6] = src_y[2];
      dst[7] = src_y[3];
      src_y += pixel_bytes;

      // Copying the Z-axis rawdata
      dst[8] = src_z[0];
      dst[9] = src_z[1];
      dst[10] = src_z[2];
      dst[11] = src_z[3];
      src_z += pixel_bytes;

      dst += pixel_bytes * kPointCloudXyzPlane;
      size -= pixel_bytes * kPointCloudXyzPlane;
    }
  }

  return Status::OK();
}

/**
 * @brief Convert pixel format from planar to packed.
 * @param[in/out] (binary_property) The binary data of PointCloudProperty.
 */
void SkvPointCloudChannelRecorder::ConvertPixelFormat(
    BinaryProperty* binary_property) {
  // Deserialize
  PointCloudProperty point_cloud = {};
  serialize::Decoder decoder(
      &binary_property->data[0], binary_property->data.size());
  decoder.Pop(point_cloud);

  // Convert the pixel format
  if (point_cloud.pixel_format == kPixelFormatXYZ16Planar) {
    point_cloud.pixel_format = kPixelFormatXYZ16;
  } else if (point_cloud.pixel_format == kPixelFormatXYZ32FPlanar) {
    point_cloud.pixel_format = kPixelFormatXYZ32F;
  }

  // Serialize
  serialize::SerializedBuffer serialized_property;
  serialize::Encoder enc(&serialized_property);
  enc.Push(point_cloud);

  // Update binary property
  binary_property->data.reserve(serialized_property.size());
  binary_property->data.assign(
      reinterpret_cast<const uint8_t*>(serialized_property.data()),
      reinterpret_cast<const uint8_t*>(serialized_property.data()) +
      serialized_property.size());
}
}   // namespace senscord
