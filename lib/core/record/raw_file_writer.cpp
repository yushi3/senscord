/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/raw_file_writer.h"
#include "senscord/develop/recorder_common.h"
#include "logger/logger.h"
#include "record/recorder_manager.h"

namespace senscord {

/**
 * @brief Open the index file.
 * @param[in] (output_dir_path) Recording target directory.
 * @return Status object.
 */
Status RawFileWriter::Open(const std::string& output_dir_path) {
  if (!output_dir_path_.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseAlreadyExists,
        "already opened");
  }
  output_dir_path_ = output_dir_path;
  size_t length = output_dir_path.find_last_of(osal::kDirectoryDelimiter);
  std::string root_record_path = output_dir_path.substr(0, length);
  RecorderManager* manager = RecorderManager::GetInstance();
  Status status = manager->AttachRawIndexFileWriter(
      root_record_path, &raw_index_file_writer_);
  SENSCORD_STATUS_TRACE(status);

  if (status.ok()) {
    channel_property_writer_ = new ChannelPropertiesFileWriter();
    status = channel_property_writer_->Open(output_dir_path_);
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Close the index file
 */
void RawFileWriter::Close() {
  if (!output_dir_path_.empty()) {
    RecorderManager* manager = RecorderManager::GetInstance();
    size_t length = output_dir_path_.find_last_of(osal::kDirectoryDelimiter);
    std::string root_record_path = output_dir_path_.substr(0, length);
    Status status = manager->DetachRawIndexFileWriter(root_record_path);
    if (!status.ok()) {
      SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
    }
    raw_index_file_writer_ = NULL;
    channel_property_writer_->Close();
    delete channel_property_writer_;
    channel_property_writer_ = NULL;
    output_dir_path_.clear();
  }
}

/**
 * @brief Write as the raw data.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (sent_time) Time when frame was sent,
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status RawFileWriter::Write(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel) {
  // create file name and full path
  std::string full_path;
  RecordUtility::GetRawDataFileName(sequence_number, &full_path);
  full_path = output_dir_path_ + osal::kDirectoryDelimiter + full_path;

  // open
  osal::OSFile* file = NULL;
  Status status = OpenBinaryFile(full_path, &file);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    // write
    status = WriteFile(file, sequence_number, channel);
    SENSCORD_STATUS_TRACE(status);

    // close
    CloseBinaryFile(file);
  }

  if (status.ok()) {
    // write to raw index file
    status = raw_index_file_writer_->WriteRaw(
        sequence_number, sent_time, channel);
    SENSCORD_STATUS_TRACE(status);
  }

  if (status.ok()) {
    // write properties
    status = channel_property_writer_->Write(sequence_number, channel);
    SENSCORD_STATUS_TRACE(status);
  }
  return status;
}

/**
 * @brief Write a channel.
 * @param[in] (file) Opened file pointer.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status RawFileWriter::WriteFile(osal::OSFile* file, uint64_t sequence_number,
    const SerializedChannel& channel) {
  if (channel.rawdata.size() > 0) {
    int32_t ret = osal::OSFwrite(&channel.rawdata[0], sizeof(uint8_t),
        channel.rawdata.size(), file, NULL);
    if (ret != 0) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseAborted,
          "failed to write recording file: 0x%" PRIx32, ret);
    }
  } else {
    // create empty file
  }
  return Status::OK();
}

/**
 * @brief Open the target file.
 * @param[in] (filepath) File path.
 * @param[out] (file) File handler.
 * @return Status object.
 */
Status RawFileWriter::OpenBinaryFile(
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
void RawFileWriter::CloseBinaryFile(osal::OSFile* file) const {
  int32_t ret = osal::OSFclose(file);
  if (ret != 0) {
    // print only
    SENSCORD_LOG_WARNING("failed to close file: 0x%" PRIx32, ret);
  }
}

/**
 * @brief Constructor
 */
RawFileWriter::RawFileWriter()
  : output_dir_path_(), channel_property_file_(), raw_index_file_writer_(),
    channel_property_writer_() {}

/**
 * @brief Destructor
 */
RawFileWriter::~RawFileWriter() {}

}   // namespace senscord
