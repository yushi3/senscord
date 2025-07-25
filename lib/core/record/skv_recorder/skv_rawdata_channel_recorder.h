/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_SKV_RECORDER_SKV_RAWDATA_CHANNEL_RECORDER_H_
#define LIB_CORE_RECORD_SKV_RECORDER_SKV_RAWDATA_CHANNEL_RECORDER_H_

#include <string>
#include <vector>

#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/osal.h"
#include "senscord/property_types.h"
#include "senscord/develop/channel_recorder.h"
#include "record/skv_recorder/skv_channel_recorder.h"

namespace senscord {
/**
 * @brief Channel recorder for skv format.
 */
class SkvRawDataChannelRecorder : public SkvChannelRecorder {
 public:
  /**
   * @brief Constructor
   * @param[in] (channel_id) ID of the channel to record.
   */
  explicit SkvRawDataChannelRecorder(const uint32_t channel_id);

  /**
   * @brief Destructor
   */
  ~SkvRawDataChannelRecorder();

 private:
  /**
   * @brief Initialize for writing the channel.
   * @param[in] (channel) record channel.
   * @param[in] (library) skv library.
   * @return Status object.
   */
  virtual Status Init(
      const SerializedChannel& channel, SkvRecordLibrary* library);

  /**
   * @brief Write the channel data to file.
   * @param[in] (sent_time) Time when frame was sent.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  virtual Status WriteRawData(
    uint64_t sent_time,
    const SerializedChannel& channel);

  /**
   * @brief Write the channel property to file.
   * @param[in] (sequence_number) sequence number of record frame.
   * @param[in] (sent_time) Time when frame was sent.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  virtual Status WriteProperty(
    uint64_t sequence_number,
    uint64_t sent_time,
    const SerializedChannel& channel);

  /**
   * @brief set stream info for rawdata stream.
   * @param[in] (property) property of record channel.
   * @param[in] (info) stream info of rawdata written stream.
   * @return Status object.
   */
  Status SetRawDataStreamInfo(
    const ImageProperty& property,
    SkvImageStreamInfo* info);

  /**
   * @brief set stream info for property stream.
   * @param[in] (stream_name) name of rawdata  written stream.
   * @param[in] (buffer_size) size of serialized channel properties.
   * @param[in] (info) stream info of property written stream.
   * @return Status object.
   */
  Status SetPropertyStreamInfo(
    const std::string& stream_name,
    size_t buffer_size,
    SkvCustomStreamInfo* info);

  uint32_t channel_id_;
  SkvImageStreamInfo rawdata_stream_info_;
  SkvCustomStreamInfo property_stream_info_;
};

}   // namespace senscord
#endif    // LIB_CORE_RECORD_SKV_RECORDER_SKV_RAWDATA_CHANNEL_RECORDER_H_
