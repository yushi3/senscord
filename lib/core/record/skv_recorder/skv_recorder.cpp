/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/skv_recorder/skv_recorder.h"
#include <string>
#include <vector>
#include <sstream>
#include "senscord/osal.h"
#include "senscord/status.h"
#include "senscord/develop/recorder_common.h"
#include "logger/logger.h"

namespace senscord {

/**
 * @brief Start to record.
 * @param[in] (path) Top directory path of recording.
 * @param[in] (format) Target format name.
 * @param[in] (stream) Recording stream.
 * @return Status object.
 */
Status SkvRecorder::Start(const std::string& path,
    const std::string& format, Stream* stream) {
  if (skv_record_library_ == NULL) {
    // impossible
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation,
        "Not found record library");
  }
  return Status::OK();
}

/**
 * @brief Stop to record.
 */
void SkvRecorder::Stop()  {
  ReleaseChannelRecorder();
  is_detected_ = false;
}

/**
 * @brief Write a channel.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status SkvRecorder::Write(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel) {
  Status status;
  if (!is_detected_) {
    // first received
    if (!IsRecordableChannel(channel)) {
      // do nothing (unsupported)
      is_detected_ = true;
      return senscord::Status::OK();
    }
    // create new channel recorder
    SkvChannelRecorder* recorder;
    recorder = CreateChannelRecorder(channel.id, channel.type);
    if (recorder == NULL) {
      // do nothing (unsupported rawdata type)
      is_detected_ = true;
      return SENSCORD_STATUS_FAIL(kStatusBlockRecorder,
          Status::kCauseNotSupported,
          "unsupported rawdata type: \"%s\"", channel.type.c_str());
    }

    // initialize
    status = recorder->Init(channel, skv_record_library_);
    if (!status.ok()) {
      // do nothing (failed to initialize)
      is_detected_ = true;
      delete recorder;
      return SENSCORD_STATUS_TRACE(status);
    }

    // register
    recorder_ = recorder;
    is_detected_ = true;
  }

  if (is_detected_) {
    if (recorder_ == NULL) {
      // do nothing (unsupported)
    } else {
      // write
      status = recorder_->Write(sequence_number, sent_time, channel);
      SENSCORD_STATUS_TRACE(status);
    }
  }
  return status;
}

/**
 * @brief Create new channel recorder.
 * @param[in] (channel_id) ID of the channel to record.
 * @param[in] (rawdata_type) Raw data type of the channel to record.
 * @return New channel recorder.
 */
SkvChannelRecorder* SkvRecorder::CreateChannelRecorder(
    const uint32_t channel_id, const std::string& rawdata_type) {
  // create recorder each rawdata type of channel
  if (rawdata_type == kRawDataTypeDepth) {
    // depth channel recorder
    return new SkvDepthChannelRecorder();
  } else if (rawdata_type == kRawDataTypeConfidence) {
    // confidence channel recorder
    return new SkvConfidenceChannelRecorder();
  } else if (rawdata_type == kRawDataTypePointCloud) {
    // point-cloud channel recorder
    return new SkvPointCloudChannelRecorder();
  } else if (rawdata_type == kRawDataTypeImage) {
    // rawdata channel recorder
    return new SkvRawDataChannelRecorder(channel_id);
  }

  return NULL;
}

/**
 * @brief Create new channel recorder.
 * @param[in] (pixel_format) Pixel format of channel.
 * @return New channel recorder.
 */
void SkvRecorder::ReleaseChannelRecorder() {
  delete recorder_;
  recorder_ = NULL;
}

/**
 * @brief Check whether the data type of channel.
 * @param[in] (channel) Serialized channel data.
 * @return True means recordable channel.
 */
bool SkvRecorder::IsRecordableChannel(
    const SerializedChannel& channel) const {
  if ((channel.type == kRawDataTypeDepth) ||
      (channel.type == kRawDataTypeConfidence) ||
      (channel.type == kRawDataTypePointCloud) ||
      (channel.type == kRawDataTypeImage)) {
    return true;
  }
  SENSCORD_LOG_WARNING("Unsupported rawdata_type : id=%" PRIu32 ", type=%s",
      channel.id, channel.type.c_str());

  return false;
}

/**
 * @brief Constructor
 * @param[in] (skv_record_library) Skv record library
 */
SkvRecorder::SkvRecorder(SkvRecordLibrary* skv_record_library)
    : is_detected_(false), recorder_(),
      skv_record_library_(skv_record_library) {}

/**
 * @brief Destructor
 */
SkvRecorder::~SkvRecorder() {}

}   // namespace senscord
