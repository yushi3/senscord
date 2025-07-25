/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_COMPOSITE_RAW_RECORDER_H_
#define LIB_CORE_RECORD_COMPOSITE_RAW_RECORDER_H_

#include <string>
#include "senscord/develop/channel_recorder.h"
#include "senscord/develop/composite_raw_file_writer.h"
#include "senscord/osal.h"
#include "senscord/status.h"

namespace senscord {

/**
 * @brief Recorder for composite-raw format.
 */
class CompositeRawRecorder : public ChannelRecorder {
 public:
  /**
   * @brief Start to record.
   * @param[in] (path) Top directory path of recording.
   * @param[in] (format) Target format name.
   * @param[in] (stream) Recording stream.
   * @return Status object.
   */
  virtual Status Start(
    const std::string& path,
    const std::string& format,
    Stream* stream);

  /**
   * @brief Stop to record.
   */
  virtual void Stop();

  /**
   * @brief Write a channel.
   * @param[in] (sequence_number) Frame sequence number.
   * @param[in] (sent_time) Time when frame was sent.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  virtual Status Write(
    uint64_t sequence_number,
    uint64_t sent_time,
    const SerializedChannel& channel);

  /**
   * @brief Constructor
   */
  CompositeRawRecorder();

  /**
   * @brief Destructor
   */
  ~CompositeRawRecorder();

 private:
  // output directory
  std::string output_dir_path_;

  // composite raw writer
  CompositeRawFileWriter* composite_raw_file_writer_;
};

}   // namespace senscord
#endif  // LIB_CORE_RECORD_COMPOSITE_RAW_RECORDER_H_
