/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "include/pnm_channel_recorder.h"

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <limits>  // std::numeric_limits

#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"
#include "senscord/status.h"
#include "senscord/logger.h"

// PNM information.
static const char kPortableGrayMap[]   = "P5";
static const char kPortablePixMap[]    = "P6";
static const char kLuminanceFor8bit[]  = "255";
static const char kLuminanceFor16bit[] = "65535";
static const char kLineFeed[]   = "\n";
static const char kWhiteSpace[] = " ";
static const char kPgmExtension[] = ".pgm";
static const char kPpmExtension[] = ".ppm";

/**
 * @brief Constructor
 */
PnmChannelRecorder::PnmChannelRecorder() : format_info_(), dir_path_() {}

/**
 * @brief Destructor
 */
PnmChannelRecorder::~PnmChannelRecorder() {}

/**
 * @brief Initialize for writing the channel.
 * @param[in] (channel_id) Channel ID.
 * @param[in] (output_dir_path) Output directory.
 * @return Status object.
 */
senscord::Status PnmChannelRecorder::Init(
    uint32_t channel_id, const std::string& output_dir_path) {
  dir_path_ = output_dir_path;
  return senscord::Status::OK();
}

/**
 * @brief Write the channel.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (property) Image property of channel.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
senscord::Status PnmChannelRecorder::Write(
    uint64_t sequence_number,
    const senscord::ImageProperty& property,
    const senscord::SerializedChannel& channel) {
  // invalid data (skip)
  if (channel.rawdata.size() == 0) {
    return senscord::Status::OK();
  }

  // Check rawdata size.
  uint32_t expected_size = property.stride_bytes * property.height;
  if (expected_size != channel.rawdata.size()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
        senscord::Status::kCauseAborted,
        "Different rawdata size than expected: "
        "rawsize=%" PRIuS ", expected size=%" PRIuS,
        channel.rawdata.size(), expected_size);
  }

  // create file name
  std::string full_path = dir_path_;
  {
    std::string file_name;
    CreateFileName(&file_name, sequence_number, channel);
    full_path += senscord::osal::kDirectoryDelimiter;
    full_path += file_name;
  }

  // target file open
  senscord::osal::OSFile* file = NULL;
  int32_t ret = senscord::osal::OSFopen(full_path.c_str(), "wb", &file);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
        senscord::Status::kCauseAborted,
        "failed to open recording file: 0x%" PRIx32);
  }

  // write header
  senscord::Status status = WriteHeader(file, property);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    // write process core
    status = WritePayload(file, property, channel);
    SENSCORD_STATUS_TRACE(status);
  }

  // target file close
  senscord::osal::OSFclose(file);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Create the image header.
 * @param[out] (header) image header.
 * @param[in] (property) Image property of channel.
 * @return Status object.
 */
void PnmChannelRecorder::CreateHeader(
      std::string* header,
      const senscord::ImageProperty& property) const {
  // Create the header
  std::stringstream ss;
  ss << format_info_.magic_number << kLineFeed;
  ss << property.width << kWhiteSpace << property.height << kLineFeed;
  ss << format_info_.luminance << kLineFeed;

  *header = ss.str();
}

/**
 * @brief Write the file header.
 * @param[out] (file_name) Output file name.
 * @param[in] (property) Image property of channel.
 * @return Status object.
 */
senscord::Status PnmChannelRecorder::WriteHeader(
    senscord::osal::OSFile* file,
    const senscord::ImageProperty& property) const {

  // create header
  std::string header;
  CreateHeader(&header, property);

  // write header
  int32_t ret = senscord::osal::OSFwrite(
      header.c_str(), sizeof(uint8_t), header.size(), file, NULL);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
        senscord::Status::kCauseAborted,
        "failed to write file header: 0x%" PRIx32, ret);
  }

  return senscord::Status::OK();
}

/**
 * @brief Write the channel data to file.
 * @param[in] (file) Destination file pointer.
 * @param[in] (property) Image property of channel.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
senscord::Status PnmChannelRecorder::WritePayload(
    senscord::osal::OSFile* file,
    const senscord::ImageProperty& property,
    const senscord::SerializedChannel& channel) {
  // Check invalid width bytes(BytesPerPixel=1).
  size_t width_bytes = property.width;
  if (width_bytes > property.stride_bytes) {
    return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
        senscord::Status::kCauseAborted,
        "Width_bytes is greater than stride_bytes: "
        "stride_bytes=%" PRIuS ", width_bytes=%" PRIuS,
        property.stride_bytes, width_bytes);
  }

  for (uint32_t height = 0; height < property.height; ++height) {
    size_t read_addr = height * property.stride_bytes;
    // write to file
    int32_t ret = senscord::osal::OSFwrite(
        &channel.rawdata[read_addr], sizeof(uint8_t), width_bytes, file, NULL);
    if (ret != 0) {
      return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
          senscord::Status::kCauseAborted,
          "failed to write file: 0x%" PRIx32, ret);
    }
  }
  return senscord::Status::OK();
}

/**
 * @brief Create the output file name.
 * @param[out] (file_name) Output file name.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (channel) Serialized channel data.
 */
void PnmChannelRecorder::CreateFileName(
    std::string* file_name,
    uint64_t sequence_number,
    const senscord::SerializedChannel& channel) const {
  std::stringstream ss;
  ss << "data_" << std::setw(10) << std::setfill('0') << sequence_number;
  ss << '_' << channel.timestamp << format_info_.extension;

  *file_name = ss.str();
}

/**
 * @brief Constructor
 */
PgmChannelRecorder8bit::PgmChannelRecorder8bit() {
  format_info_.magic_number = kPortableGrayMap;
  format_info_.luminance    = kLuminanceFor8bit;
  format_info_.extension    = kPgmExtension;
}

/**
 * @brief Destructor
 */
PgmChannelRecorder8bit::~PgmChannelRecorder8bit() {}

/**
 * @brief Constructor
 */
PgmChannelRecorder16bit::PgmChannelRecorder16bit() {
  format_info_.magic_number = kPortableGrayMap;
  format_info_.luminance    = kLuminanceFor16bit;
  format_info_.extension    = kPgmExtension;
}

/**
 * @brief Destructor
 */
PgmChannelRecorder16bit::~PgmChannelRecorder16bit() {}

/**
 * @brief Write the channel data to file.
 * @param[in] (file) Destination file pointer.
 * @param[in] (property) Image property of channel.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
senscord::Status PgmChannelRecorder16bit::WritePayload(
    senscord::osal::OSFile* file,
    const senscord::ImageProperty& property,
    const senscord::SerializedChannel& channel) {
  const size_t& size_t_max = std::numeric_limits<size_t>::max();
  if ((size_t_max / 2) < property.width) {
    // not record (width*2 is overflow)
    return senscord::Status::OK();
  }

  // Check invalid width bytes(BytesPerPixel=2).
  size_t width_bytes = property.width * 2;
  if (width_bytes > property.stride_bytes) {
    return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
        senscord::Status::kCauseAborted,
        "Width_bytes is greater than stride_bytes: "
        "stride_bytes=%" PRIuS ", width_bytes=%" PRIuS,
        property.stride_bytes, width_bytes);
  }

  uint16_t* buffer = new uint16_t[property.width];
  for (uint32_t height = 0; height < property.height; ++height) {
    size_t read_addr = property.stride_bytes * height;
    const uint16_t* write_data =
        reinterpret_cast<const uint16_t*>(&channel.rawdata[read_addr]);

    for (uint32_t width = 0; width < property.width; ++width) {
      // Converting to big-endian and buffering
      buffer[width] = senscord::osal::OSHtons(write_data[width]);
    }

    // write to file
    int32_t ret = senscord::osal::OSFwrite(
        buffer, sizeof(uint16_t), property.width, file, NULL);
    if (ret != 0) {
      delete[] buffer;
      return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
          senscord::Status::kCauseAborted,
          "failed to write file: 0x%" PRIx32, ret);
    }
  }
  delete[] buffer;

  return senscord::Status::OK();
}

/**
 * @brief Constructor
 */
PpmChannelRecorder::PpmChannelRecorder() {
  format_info_.magic_number = kPortablePixMap;
  format_info_.luminance    = kLuminanceFor8bit;
  format_info_.extension    = kPpmExtension;
}

/**
 * @brief Destructor
 */
PpmChannelRecorder::~PpmChannelRecorder() {}

/**
 * @brief Write the channel data to file.
 * @param[in] (file) Destination file pointer.
 * @param[in] (property) Image property of channel.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
senscord::Status PpmChannelRecorder::WritePayload(
    senscord::osal::OSFile* file,
    const senscord::ImageProperty& property,
    const senscord::SerializedChannel& channel) {
  // Check invalid width bytes(BytesPerPixel=3).
  size_t width_bytes = property.width * 3;
  if (width_bytes > property.stride_bytes) {
    return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
        senscord::Status::kCauseAborted,
        "Width_bytes is greater than stride_bytes: "
        "stride_bytes=%" PRIuS ", width_bytes=%" PRIuS,
        property.stride_bytes, width_bytes);
  }

  for (uint32_t height = 0; height < property.height; ++height) {
    size_t read_addr = height * property.stride_bytes;
    // write to file
    int32_t ret = senscord::osal::OSFwrite(
        &channel.rawdata[read_addr], sizeof(uint8_t), width_bytes, file, NULL);
    if (ret != 0) {
      return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
          senscord::Status::kCauseAborted,
          "failed to write file: 0x%" PRIx32, ret);
    }
  }
  return senscord::Status::OK();
}
