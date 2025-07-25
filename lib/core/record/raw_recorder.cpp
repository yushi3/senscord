/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/raw_recorder.h"
#include <string>
#include "senscord/status.h"

namespace senscord {

/**
 * @brief Start to record.
 * @param[in] (path) Top directory path of recording.
 * @param[in] (format) Target format name.
 * @param[in] (stream) Recording stream.
 * @return Status object.
 */
Status RawRecorder::Start(const std::string& path,
    const std::string& format, Stream* stream) {
  output_dir_path_ = path;
  raw_file_writer_ = new RawFileWriter();
  Status status = raw_file_writer_->Open(output_dir_path_);
  if (!status.ok()) {
    delete raw_file_writer_;
    raw_file_writer_ = NULL;
    return SENSCORD_STATUS_TRACE(status);
  }
  return Status::OK();
}

/**
 * @brief Stop to record.
 */
void RawRecorder::Stop() {
  if (raw_file_writer_) {
    raw_file_writer_->Close();
    delete raw_file_writer_;
    raw_file_writer_ = NULL;
  }
}

/**
 * @brief Write a channel.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status RawRecorder::Write(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel) {
  // write
  Status status = raw_file_writer_->Write(sequence_number, sent_time, channel);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Constructor
 */
RawRecorder::RawRecorder() : raw_file_writer_() {}

/**
 * @brief Destructor
 */
RawRecorder::~RawRecorder() {}

}   // namespace senscord
