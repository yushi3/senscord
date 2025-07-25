/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_CHANNEL_RECORDER_ADAPTER_H_
#define LIB_CORE_RECORD_CHANNEL_RECORDER_ADAPTER_H_

#include <stdint.h>
#include <string>
#include "senscord/develop/channel_recorder.h"

namespace senscord {

/**
 * @brief Adapter for implemented channel recorder.
 */
class ChannelRecorderAdapter : public ChannelRecorder {
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
   * @brief Check whether write error was occured.
   * @return true means error occured.
   */
  bool IsOccuredWriteError() const;

  /**
   * @brief Get the implemented channel recorder.
   * @return Implemented recorder.
   */
  ChannelRecorder* GetOrigin() const;

  /**
   * @brief Constructor
   * @param[in] (origin) Implemented recorder.
   */
  explicit ChannelRecorderAdapter(ChannelRecorder* origin);

  /**
   * @brief Destructor
   */
  ~ChannelRecorderAdapter();

 private:
  /**
   * @brief Create the output directory at once.
   * @param[in] (path) Top directory path of recording.
   * @return Status object.
   */
  Status CreateOutputDirectory(const std::string& path);

  // implemented recorder.
  ChannelRecorder* origin_;

  // whether the file-write error was occured.
  bool error_occured_;
};

}   // namespace senscord
#endif  // LIB_CORE_RECORD_CHANNEL_RECORDER_ADAPTER_H_
