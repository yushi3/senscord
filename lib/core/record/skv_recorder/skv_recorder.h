/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_SKV_RECORDER_SKV_RECORDER_H_
#define LIB_CORE_RECORD_SKV_RECORDER_SKV_RECORDER_H_

#include <string>
#include "senscord/develop/channel_recorder.h"
#include "senscord/osal.h"
#include "record/skv_recorder/skv_record_library.h"
#include "record/skv_recorder/skv_channel_recorder.h"
#include "record/skv_recorder/skv_depth_channel_recorder.h"
#include "record/skv_recorder/skv_confidence_channel_recorder.h"
#include "record/skv_recorder/skv_pointcloud_channel_recorder.h"
#include "record/skv_recorder/skv_rawdata_channel_recorder.h"

namespace senscord {

/**
 * @brief Recorder for raw format.
 */
class SkvRecorder : public ChannelRecorder {
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
   * @param[in] (common_writer) Skv common writer
   */
  explicit SkvRecorder(SkvRecordLibrary* skv_record_library);

  /**
   * @brief Destructor
   */
  ~SkvRecorder();

 protected:
  /**
   * @brief Create new channel recorder.
   * @param[in] (channel_id) ID of the channel to record.
   * @param[in] (rawdata_type) Raw data type of the channel to record.
   * @return New channel recorder.
   */
  virtual SkvChannelRecorder* CreateChannelRecorder(
      const uint32_t channel_id, const std::string& rawdata_type);

 private:
  /**
   * @brief Release channel recorder.
   */
  void ReleaseChannelRecorder();

  /**
   * @brief Check whether the data type of channel.
   * @param[in] (channel) Serialized channel data.
   * @return True means recordable channel.
   */
  bool IsRecordableChannel(
      const SerializedChannel& channel) const;

  // detected whether channel will be recorded.
  bool is_detected_;

  // recorder
  SkvChannelRecorder* recorder_;

  // skv record library
  SkvRecordLibrary* skv_record_library_;
};

}   // namespace senscord
#endif  // LIB_CORE_RECORD_SKV_RECORDER_SKV_RECORDER_H_
