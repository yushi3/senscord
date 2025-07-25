/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/develop/composite_raw_file_writer.h"

#include <string>

#include "record/channel_properties_file_writer.h"
#include "record/recorder_manager.h"
#include "logger/logger.h"
#include "senscord/status.h"

namespace senscord {

/**
 * @brief Open the index file.
 * @param[in] (output_dir_path) Recording target directory.
 * @return Status object.
 */
Status CompositeRawFileWriter::Open(const std::string& output_dir_path) {
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
void CompositeRawFileWriter::Close() {
  if (!output_dir_path_.empty()) {
    size_t length = output_dir_path_.find_last_of(osal::kDirectoryDelimiter);
    std::string root_record_path = output_dir_path_.substr(0, length);
    RecorderManager* manager = RecorderManager::GetInstance();
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
Status CompositeRawFileWriter::Write(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel) {
  if (raw_index_file_writer_ == NULL ||
      channel_property_writer_ ==  NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation,
        "not opened");
  }

  // write to raw index file
  Status status = raw_index_file_writer_->WriteCompositeRaw(
      sequence_number, sent_time, channel);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    // write properties
    status = channel_property_writer_->Write(sequence_number, channel);
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Constructor
 */
CompositeRawFileWriter::CompositeRawFileWriter()
  : output_dir_path_(), channel_property_file_(), raw_index_file_writer_(),
    channel_property_writer_() {}

/**
 * @brief Destructor
 */
CompositeRawFileWriter::~CompositeRawFileWriter() {}

}   // namespace senscord
