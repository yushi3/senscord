/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/composite_raw_recorder.h"
#include <string>
#include "senscord/osal.h"
#include "senscord/status.h"
#include "logger/logger.h"

namespace senscord {

/**
 * @brief Start to record.
 * @param[in] (path) Top directory path of recording.
 * @param[in] (format) Target format name.
 * @param[in] (stream) Recording stream.
 * @return Status object.
 */
Status CompositeRawRecorder::Start(const std::string& path,
    const std::string& format, Stream* stream) {
  output_dir_path_ = path;
  composite_raw_file_writer_ = new CompositeRawFileWriter();
  Status status = composite_raw_file_writer_->Open(output_dir_path_);
  if (!status.ok()) {
    delete composite_raw_file_writer_;
    composite_raw_file_writer_ = NULL;
    return SENSCORD_STATUS_TRACE(status);
  }
  return Status::OK();
}

/**
 * @brief Stop to record.
 */
void CompositeRawRecorder::Stop() {
  if (composite_raw_file_writer_) {
    composite_raw_file_writer_->Close();
    delete composite_raw_file_writer_;
    composite_raw_file_writer_ = NULL;
  }
}

/**
 * @brief Write a channel.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status CompositeRawRecorder::Write(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel) {
  // write
  Status status = composite_raw_file_writer_->Write(
      sequence_number, sent_time, channel);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Constructor
 */
CompositeRawRecorder::CompositeRawRecorder() : composite_raw_file_writer_() {}

/**
 * @brief Destructor
 */
CompositeRawRecorder::~CompositeRawRecorder() {}

}   // namespace senscord
