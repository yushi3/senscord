/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_SKV_RECORDER_SKV_DEPTH_CHANNEL_RECORDER_H_
#define LIB_CORE_RECORD_SKV_RECORDER_SKV_DEPTH_CHANNEL_RECORDER_H_

#include <string>
#include <vector>

#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/osal.h"
#include "senscord/property_types.h"
#include "senscord/develop/property_types_rosemary.h"
#include "senscord/develop/channel_recorder.h"
#include "record/skv_recorder/skv_channel_recorder.h"

namespace senscord {

/**
 * @brief Channel recorder for skv format.
 */
class SkvDepthChannelRecorder : public SkvChannelRecorder {
 public:
  /**
   * @brief Constructor
   */
  SkvDepthChannelRecorder();

  /**
   * @brief Destructor
   */
  ~SkvDepthChannelRecorder();

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
   * @brief Write the channel property.
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
   * @brief Write the channel property to file.
   * @param[in] (sequence_number) sequence number of record frame.
   * @param[in] (sent_time) Time when frame was sent.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  Status WriteChannelProperty(
    uint64_t sequence_number,
    uint64_t sent_time,
    const SerializedChannel& channel);

  /**
   * @brief Write the temperature property to file.
   * @param[in] (sent_time) Time when frame was sent.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  Status WriteTemperatureProperty(
    uint64_t sent_time,
    const SerializedChannel& channel);

  /**
   * @brief Write the skv write property to file.
   * @param[in] (sent_time) Time when frame was sent.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  Status WriteSkvWriteProperty(
    uint64_t sent_time,
    const SerializedChannel& channel);

  /**
   * @brief Write the frame exposure property to file.
   * @param[in] (sent_time) Time when frame was sent.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  Status WriteExposureProperty(
    uint64_t sent_time,
    const SerializedChannel& channel);

  /**
   * @brief Set StreamInfo and add ImageStream for rawdata stream.
   * @param[in] (library) skv library.
   * @param[in] (channel) record channel.
   * @param[out] (info) stream info of rawdata written stream.
   * @return Status object.
   */
  Status SetupRawDataStream(
    SkvRecordLibrary* library,
    const SerializedChannel& channel,
    SkvImageStreamInfo* info);

  /**
   * @brief set stream info for rawdata stream.
   * @param[in] (property) property of record channel.
   * @param[out] (info) stream info of rawdata written stream.
   * @return Status object.
   */
  Status SetRawDataStreamInfo(
    const ImageProperty& property,
    SkvImageStreamInfo* info);

  /**
   * @brief Set StreamInfo and add CustomStream for property stream.
   * @param[in] (channel) Serialized channel data.
   * @param[in] (library) skv library.
   * @param[in] (stream_name) name of rawdata  written stream.
   * @param[out] (info) stream info of property written stream.
   * @return Status object.
   */
  Status SetupPropertyStream(
    const SerializedChannel& channel,
    SkvRecordLibrary* library,
    const std::string& stream_name,
    SkvCustomStreamInfo* info);

  /**
   * @brief set stream info for property stream.
   * @param[in] (stream_name) name of rawdata  written stream.
   * @param[in] (buffer_size) size of serialized channel properties.
   * @param[out] (info) stream info of property written stream.
   * @return Status object.
   */
  Status SetPropertyStreamInfo(
    const std::string& stream_name,
    size_t buffer_size,
    SkvCustomStreamInfo* info);

  /**
   * @brief Set StreamInfo and add CustomStream for TemperatureProperty.
   * @param[in] (library) skv library.
   * @param[in] (channel) record channel.
   * @param[out] (info_list) Stream info of TemperatureProperty members.
   * @return Status object.
   */
  Status SetupTemperaturePropertyStream(
    SkvRecordLibrary* library, const SerializedChannel& channel,
    std::vector<SkvCustomStreamInfo>* info_list);

  /**
   * @brief Set stream info for TemperatureProperty.
   * @param[in] (property) TemperatureProperty of parent stream.
   * @param[out] (info_list) stream info of property written stream.
   * @return Status object.
   */
  Status SetTemperaturePropertyStreamInfo(
    const TemperatureProperty& property,
    std::vector<SkvCustomStreamInfo>* info_list);

  /**
   * @brief Set StreamInfo and add CustomStream for ExposureProperty.
   * @param[in] (library) skv library.
   * @param[in] (channel) record channel.
   * @param[out] (info) Stream info of ExposureProperty member.
   * @return Status object.
   */
  Status SetupExposurePropertyStream(
    SkvRecordLibrary* library, const SerializedChannel& channel,
    SkvCustomStreamInfo* info);

  /**
   * @brief Set stream info for ExposureProperty.
   * @param[in] (property) ExposureProperty of parent stream.
   * @param[out] (info) Stream info of ExposureProperty member.
   * @return Status object.
   */
  Status SetExposurePropertyStreamInfo(
    const ExposureProperty& property,
    SkvCustomStreamInfo* info);

  /**
   * @brief Set StreamInfo and add CustomStream for SkvWriteProperty.
   * @param[in] (library) skv library.
   * @param[in] (channel) record channel.
   * @param[out] (info_list) Stream info of SkvWriteProperty members.
   * @return Status object.
   */
  Status SetupSkvWritePropertyStream(
    SkvRecordLibrary* library, const SerializedChannel& channel,
    std::vector<SkvCustomStreamInfo>* info_list);

  SkvImageStreamInfo rawdata_stream_info_;
  SkvCustomStreamInfo property_stream_info_;
  std::vector<SkvCustomStreamInfo> temperature_property_stream_info_;
  std::vector<SkvCustomStreamInfo> frame_extension_property_stream_info_;
  SkvCustomStreamInfo exposure_property_stream_info_;
  std::vector<SkvCustomStreamInfo> skv_write_property_stream_info_;

  bool is_recordable_temperature_property_;
  bool is_recordable_frame_extension_property_;
  bool is_recordable_exposure_property_;
  bool is_recordable_skv_write_property_;
};

}   // namespace senscord
#endif    // LIB_CORE_RECORD_SKV_RECORDER_SKV_DEPTH_CHANNEL_RECORDER_H_
