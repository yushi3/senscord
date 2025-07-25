/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "include/bin_channel_recorder.h"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "senscord/osal.h"
#include "senscord/status.h"
#include "senscord/logger.h"

// This parameter is the size of the write buffer.
// [Example of setting values]
//  0 : Allocate a buffer of rawdata size
//  8192  : Allocate a 8KB buffer
//  65536 : Allocate a 64KB buffer
static const size_t kWriteBufferSize = 65536;

/**
 * @brief Constructor
 */
BinChannelRecorder::BinChannelRecorder() : dir_path_() {}

/**
 * @brief Destructor
 */
BinChannelRecorder::~BinChannelRecorder() {}

/**
 * @brief Initialize for writing the channel.
 * @param[in] (channel_id) Channel ID.
 * @param[in] (output_dir_path) Output directory.
 * @return Status object.
 */
senscord::Status BinChannelRecorder::Init(
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
senscord::Status BinChannelRecorder::Write(
    uint64_t sequence_number,
    const senscord::ImageProperty& property,
    const senscord::SerializedChannel& channel) {
  // invalid data (skip)
  if (channel.rawdata.size() == 0) {
    return senscord::Status::OK();
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
    return SENSCORD_STATUS_FAIL("recorder",
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
 * @brief Write the file header.
 * @param[out] (file_name) Output file name.
 * @param[in] (property) Image property of channel.
 * @return Status object.
 */
senscord::Status BinChannelRecorder::WriteHeader(
    senscord::osal::OSFile* file,
    const senscord::ImageProperty& property) const {
  // header data (width, height)
  uint16_t header[] = {
    static_cast<uint16_t>(property.width),
    static_cast<uint16_t>(property.height)
  };
  // write
  int32_t ret = senscord::osal::OSFwrite(
      header, sizeof(uint16_t), 2, file, NULL);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL("recorder",
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
senscord::Status BinChannelRecorder::WritePayload(
    senscord::osal::OSFile* file,
    const senscord::ImageProperty& property,
    const senscord::SerializedChannel& channel) {
  // for 16-bit pixel format
  size_t count = property.width * property.height;

  // write to file
  int32_t ret = senscord::osal::OSFwrite(
      &channel.rawdata[0], sizeof(uint16_t), count, file, NULL);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL("recorder",
        senscord::Status::kCauseAborted,
        "failed to write file: 0x%" PRIx32, ret);
  }
  return senscord::Status::OK();
}

/**
 * @brief Create the output file name.
 * @param[out] (file_name) Output file name.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (channel) Serialized channel data.
 */
void BinChannelRecorder::CreateFileName(
    std::string* file_name,
    uint64_t sequence_number,
    const senscord::SerializedChannel& channel) const {
  std::stringstream ss;
  ss << "data_" << std::setw(10) << std::setfill('0') << sequence_number;
  ss << '_' << channel.timestamp << ".bin";
  *file_name = ss.str();
}

/**
 * @brief Constructor
 */
BinChannelRecorder8bit::BinChannelRecorder8bit() {}

/**
 * @brief Destructor
 */
BinChannelRecorder8bit::~BinChannelRecorder8bit() {}

/**
 * @brief Write the channel data to file.
 * @param[in] (file) Destination file pointer.
 * @param[in] (property) Image property of channel.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
senscord::Status BinChannelRecorder8bit::WritePayload(
    senscord::osal::OSFile* file,
    const senscord::ImageProperty& property,
    const senscord::SerializedChannel& channel) {
  // Set buffer size
  size_t buffer_size = kWriteBufferSize;
  if (buffer_size > channel.rawdata.size()) {
    buffer_size = channel.rawdata.size();
  }

  uint16_t* buffer = new uint16_t[buffer_size];
  const uint8_t* rawdata = &channel.rawdata[0];
  size_t raw_size = channel.rawdata.size();

  size_t i = 0;
  while (true) {
    // convert to 16-bit wides
    buffer[i % buffer_size] = static_cast<uint16_t>(rawdata[i] << 8);
    ++i;

    // Write to a file in the following cases
    // - When the buffer is maximum.
    // - When the last data.
    size_t write_size = 0;
    if ((i % buffer_size) == 0) {
      write_size = buffer_size;
    } else if (i == raw_size) {
      write_size = i % buffer_size;
    }

    if (write_size != 0) {
      // write to file
      int32_t ret = senscord::osal::OSFwrite(
          buffer, sizeof(uint16_t), write_size, file, NULL);
      if (ret != 0) {
        delete[] buffer;
        return SENSCORD_STATUS_FAIL("recorder",
            senscord::Status::kCauseAborted,
            "failed to write file: 0x%" PRIx32, ret);
      }
    }

    // Write completion
    if (i == raw_size) {
      break;
    }
  }

  delete[] buffer;

  return senscord::Status::OK();
}
