/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_RECORDER_BIN_INCLUDE_BIN_RECORDER_H_
#define LIB_RECORDER_BIN_INCLUDE_BIN_RECORDER_H_

#include <inttypes.h>
#include <string>
#include "senscord/status.h"
#include "senscord/stream.h"
#include "senscord/develop/channel_recorder.h"
#include "include/bin_channel_recorder.h"

/**
 * @brief Recorder for bin format.
 */
class BinRecorder : public senscord::ChannelRecorder {
 public:
  /**
   * @brief Start to record.
   * @param[in] (path) Top directory path of recording.
   * @param[in] (format) Target format name.
   * @param[in] (stream) Recording stream.
   * @return Status object.
   */
  virtual senscord::Status Start(
    const std::string& path,
    const std::string& format,
    senscord::Stream* stream);

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
  virtual senscord::Status Write(
    uint64_t sequence_number, uint64_t sent_time,
    const senscord::SerializedChannel& channel);

  /**
   * @brief Constructor
   */
  BinRecorder();

  /**
   * @brief Destructor
   */
  virtual ~BinRecorder();

 protected:
  /**
   * @brief Check whether the channel data is image data.
   * @param[in] (channel) Serialized channel data.
   * @return True means image channel.
   */
  bool IsImageChannel(const senscord::SerializedChannel& channel) const;

  /**
   * @brief Get the image property from channel.
   * @param[in] (channel) Serialized channel data.
   * @param[out] (property) Image property of channel.
   * @return Status object.
   */
  senscord::Status GetImageProperty(
    const senscord::SerializedChannel& channel,
    senscord::ImageProperty* property) const;

  /**
   * @brief Create new channel recorder.
   * @param[in] (pixel_format) Pixel format of channel.
   * @return New channel recorder.
   */
  virtual BinChannelRecorder* CreateChannelRecorder(
    const std::string& pixel_format) const;

 private:
  // detected whether channel will be recorded.
  bool is_detected_;

  // output directory
  std::string output_dir_path_;

  // recorder
  BinChannelRecorder* recorder_;
};

#endif  // LIB_RECORDER_BIN_INCLUDE_BIN_RECORDER_H_
