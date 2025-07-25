/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/raw_index_file_writer.h"
#include "senscord/develop/recorder_common.h"
#include "logger/logger.h"

namespace senscord {

/**
 * @brief Open the index file.
 * @param[in] (output_dir_path) Recording target directory.
 * @return Status object.
 */
Status RawIndexFileWriter::Open(const std::string& output_dir_path) {
  if (!output_dir_path_.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseAlreadyExists,
        "already opened");
  }
  output_dir_path_ = output_dir_path;
  return Status::OK();
}

/**
 * @brief Close the index file
 */
void RawIndexFileWriter::Close() {
  if (!output_dir_path_.empty()) {
    output_dir_path_.clear();

    if (index_file_) {
      CloseBinaryFile(index_file_);
      index_file_ = NULL;
    }
  }
}

/**
 * @brief Write as the raw data.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (sent_time) Time when frame was sent,
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status RawIndexFileWriter::WriteRaw(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel) {
  Status status;

  // open file only when first write
  if (index_file_ == NULL) {
    status = OpenIndexFile();
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }

  // create the header information (no copy raw data)
  ChannelRawDataForRawIndex rec = {};
  rec.sequence_number = sequence_number;
  rec.channel_id = channel.id;
  rec.caputured_timestamp = channel.timestamp;
  rec.sent_time = sent_time;
  rec.record_type = kRecordDataTypeRaw;

  serialize::SerializedBuffer buf;
  serialize::Encoder enc(&buf);
  enc.Push(rec);

  status = WriteBinaryFile(index_file_, buf.data(), buf.size());
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Write as the comopsite raw data.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (sent_time) Time when frame was sent,
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status RawIndexFileWriter::WriteCompositeRaw(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel) {
  Status status;

  // open file only when first write
  if (index_file_ == NULL) {
    status = OpenIndexFile();
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }

  // create the header information and copy raw data
  ChannelRawDataForRawIndex rec = {};
  rec.sequence_number = sequence_number;
  rec.channel_id = channel.id;
  rec.caputured_timestamp = channel.timestamp;
  rec.sent_time = sent_time;
  rec.record_type = kRecordDataTypeCompositeRaw;
  rec.rawdata.assign(channel.rawdata.begin(), channel.rawdata.end());

  serialize::SerializedBuffer buf;
  serialize::Encoder enc(&buf);
  enc.Push(rec);

  // write
  status = WriteBinaryFile(index_file_, buf.data(), buf.size());
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Add reference.
 */
void RawIndexFileWriter::AddReference() {
  ++reference_count_;
}

/**
 * @brief Release reference.
 */
void RawIndexFileWriter::ReleaseReference() {
  --reference_count_;
}

/**
 * @brief Get reference count.
 */
uint32_t RawIndexFileWriter::GetReferenceCount() {
  return reference_count_;
}

/**
 * @brief Open the index file.
 * @return Status object.
 */
Status RawIndexFileWriter::OpenIndexFile() {
  // file path
  std::string filepath;
  RecordUtility::GetRawIndexFilePath(&filepath);
  filepath = output_dir_path_ + osal::kDirectoryDelimiter + filepath;

  // open
  Status status = OpenBinaryFile(filepath, &index_file_);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Open the target file.
 * @param[in] (filepath) File path.
 * @param[out] (file) File handler.
 * @return Status object.
 */
Status RawIndexFileWriter::OpenBinaryFile(
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
void RawIndexFileWriter::CloseBinaryFile(osal::OSFile* file) const {
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
Status RawIndexFileWriter::WriteBinaryFile(
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
RawIndexFileWriter::RawIndexFileWriter()
  : output_dir_path_(), index_file_(), reference_count_() {}

/**
 * @brief Destructor
 */
RawIndexFileWriter::~RawIndexFileWriter() {}

}   // namespace senscord
