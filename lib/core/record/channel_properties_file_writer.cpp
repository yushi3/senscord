/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/channel_properties_file_writer.h"
#include "senscord/develop/recorder_common.h"
#include "logger/logger.h"

namespace senscord {

/**
 * @brief Open the index file.
 * @param[in] (output_dir_path) Recording target directory.
 * @return Status object.
 */
Status ChannelPropertiesFileWriter::Open(const std::string& output_dir_path) {
  if (!output_dir_path_.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseAlreadyExists,
        "already started recording");
  }
  output_dir_path_ = output_dir_path;
  return Status::OK();
}

/**
 * @brief Close the index file
 */
void ChannelPropertiesFileWriter::Close() {
  if (!output_dir_path_.empty()) {
    output_dir_path_.clear();

    if (channel_property_file_) {
      CloseBinaryFile(channel_property_file_);
      channel_property_file_ = NULL;
    }
  }
}

/**
 * @brief Write the channel properties.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (channel) Serializeed channel data.
 * @return Status object.
 */
Status ChannelPropertiesFileWriter::Write(
    uint64_t sequence_number, const SerializedChannel& channel) {
  Status status;

  // open file only when first write
  if (channel_property_file_ == NULL) {
    std::string filepath;
    std::string filename;
    RecordUtility::GetChannelPropertiesFileName(&filename);
    filepath = output_dir_path_ + osal::kDirectoryDelimiter + filename;

    status = OpenBinaryFile(filepath, &channel_property_file_);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }

  // create writing data
  ChannelPropertiesForRecord rec = {};
  rec.sequence_number = sequence_number;
  rec.properties = channel.properties;

  // serialize
  serialize::SerializedBuffer buf;
  serialize::Encoder enc(&buf);
  enc.Push(rec);

  // write
  status = WriteBinaryFile(channel_property_file_, buf.data(), buf.size());
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Open the target file.
 * @param[in] (filepath) File path.
 * @param[out] (file) File handler.
 * @return Status object.
 */
Status ChannelPropertiesFileWriter::OpenBinaryFile(
    const std::string& filepath, osal::OSFile** file) const {
  int32_t ret = osal::OSFopen(filepath.c_str(), "wb", file);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseAborted,
        "failed to open file: path=%s, ret=0x%" PRIx32,
        filepath.c_str(), ret);
  }
  return Status::OK();
}

/**
 * @brief Close the file.
 * @param[in] (file) Closing file.
 */
void ChannelPropertiesFileWriter::CloseBinaryFile(osal::OSFile* file) const {
  int32_t ret = osal::OSFclose(file);
  if (ret != 0) {
    // print only
    SENSCORD_LOG_WARNING("failed to close file: 0x%" PRIx32, ret);
  }
}

/**
 * @brief Write the binary to the file.
 * @param[in] (file) Target file.
 * @param[in] (buffer) Write data.
 * @param[in] (buffer_size) Write data size.
 * @return Status object.
 */
Status ChannelPropertiesFileWriter::WriteBinaryFile(
    osal::OSFile* file, const void* buffer, size_t buffer_size) const {
  int32_t ret = osal::OSFwrite(buffer, sizeof(uint8_t),
      buffer_size, file, NULL);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseAborted,
        "failed to write file: 0x%" PRIx32, ret);
  }
  return Status::OK();
}

/**
 * @brief Constructor
 */
ChannelPropertiesFileWriter::ChannelPropertiesFileWriter()
  : output_dir_path_(), channel_property_file_() {}

/**
 * @brief Destructor
 */
ChannelPropertiesFileWriter::~ChannelPropertiesFileWriter() {}

}   // namespace senscord
