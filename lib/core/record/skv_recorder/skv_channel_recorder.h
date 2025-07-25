/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_SKV_RECORDER_SKV_CHANNEL_RECORDER_H_
#define LIB_CORE_RECORD_SKV_RECORDER_SKV_CHANNEL_RECORDER_H_

#include <string>
#include <vector>
#include <map>

#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/osal.h"
#include "senscord/property_types.h"
#include "senscord/develop/channel_recorder.h"
#include "senscord/develop/recorder_common.h"
#include "record/skv_recorder/skv_record_library.h"

// blockname for status
static const char kStatusBlockRecorder[] = "recorder";

namespace senscord {

/**
 * @brief Channel recorder for skv type.
 */
class SkvChannelRecorder : private util::Noncopyable {
 public:
  /**
   * @brief Write the channel.
   * @param[in] (sequence_number) Frame sequence number.
   * @param[in] (sent_time) Time when frame was sent.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  Status Write(
    uint64_t sequence_number,
    uint64_t sent_time,
    const SerializedChannel& channel);

  /**
   * @brief Initialize for writing the channel.
   * @param[in] (channel) record channel.
   * @param[in] (library) skv library.
   * @return Status object.
   */
  virtual Status Init(
    const SerializedChannel& channel,
    SkvRecordLibrary* library) = 0;

  /**
   * @brief Constructor
   */
  SkvChannelRecorder();

  /**
   * @brief Destructor
   */
  virtual ~SkvChannelRecorder();

 protected:
  static const size_t kPropertySizeBase = 0x400;  // 1024 bytes.

  /**
   * @brief Write the channel data to file.
   * @param[in] (sent_time) Time when frame was sent.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  virtual Status WriteRawData(
    uint64_t sent_time,
    const SerializedChannel& channel) = 0;

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
    const SerializedChannel& channel) = 0;

  /**
   * @brief Get the ExposureProperty from channel.
   * @param[in] (channel) Serialized channel data.
   * @param[out] (property) ExposureProperty of channel.
   * @return Status object.
   */
  template <typename T>
  Status GetChannelPropertyFromSerializedChannel(
      const std::string& property_key,
      const SerializedChannel& channel,
      T* property) {
    if (property == NULL) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument, "property is null");
    }

    // search property
    std::map<std::string, BinaryProperty>::const_iterator found =
        channel.properties.find(property_key);
    if (found == channel.properties.end()) {
      return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
          Status::kCauseNotFound, "%s not found.",
          property_key.c_str());
    }
    const BinaryProperty& binary = found->second;

    // convert
    serialize::Decoder dec(&binary.data[0], binary.data.size());
    Status status = dec.Pop(*property);
    return SENSCORD_STATUS_TRACE(status);
  }

  /**
   * @brief Get the CameraCalibrationProperty from stream.
   * @param[in] (library) Skv record library.
   * @param[out] (parameter) CameraCalibrationParameters of stream.
   * @return Status object.
   */
  Status GetCameraCalibrationParameter(
    SkvRecordLibrary* library,
    CameraCalibrationParameters* parameter) const;

  /**
   * @brief Get the buffer size of serialized channel properties.
   * @param[in] (channel) Serialized channel data.
   * @return Buffer size. (1024Bytes * n)
   */
  size_t GetChannelPropertyBufferSize(const SerializedChannel& channel);

  // skv record library
  SkvRecordLibrary* skv_record_library_;
};

}   // namespace senscord

#endif    // LIB_CORE_RECORD_SKV_RECORDER_SKV_CHANNEL_RECORDER_H_
