/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/channel_recorder_adapter.h"
#include <string>
#include "senscord/status.h"
#include "senscord/osal.h"

namespace senscord {

/**
 * @brief Start to record.
 * @param[in] (path) Top directory path of recording.
 * @param[in] (format) Target format name.
 * @param[in] (stream) Recording stream.
 * @return Status object.
 */
Status ChannelRecorderAdapter::Start(
    const std::string& path, const std::string& format, Stream* stream) {
  error_occured_ = false;

  Status status = CreateOutputDirectory(path);
  SENSCORD_STATUS_TRACE(status);

  if (status.ok()) {
    status = origin_->Start(path, format, stream);
    SENSCORD_STATUS_TRACE(status);
  }
  if (!status.ok()) {
    error_occured_ = true;
    osal::OSRemoveDirectory(path.c_str());
  }
  return status;
}

/**
 * @brief Stop to record.
 */
void ChannelRecorderAdapter::Stop() {
  origin_->Stop();
}

/**
 * @brief Write a channel.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status ChannelRecorderAdapter::Write(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel) {
  Status status = origin_->Write(sequence_number, sent_time, channel);
  if (!status.ok()) {
    error_occured_ = true;
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Check whether write error was occured.
 * @return true means error occured.
 */
bool ChannelRecorderAdapter::IsOccuredWriteError() const {
  return error_occured_;
}

/**
 * @brief Create the output directory at once.
 * @param[in] (path) Top directory path of recording.
 * @return Status object.
 */
Status ChannelRecorderAdapter::CreateOutputDirectory(const std::string& path) {
    // skv record is not create channel directory
  if (!path.empty()) {
    int32_t ret = osal::OSMakeDirectory(path.c_str());
    if (ret != 0) {
      error_occured_ = true;
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseAborted,
          "failed to create directory: path=%s, ret=0x%" PRIx32,
          path.c_str(), ret);
    }
  }
  return Status::OK();
}

/**
 * @brief Get the implemented channel recorder.
 * @return Implemented recorder.
 */
ChannelRecorder* ChannelRecorderAdapter::GetOrigin() const {
  return origin_;
}

/**
 * @brief Constructor
 * @param[in] (origin) Implemented recorder.
 */
ChannelRecorderAdapter::ChannelRecorderAdapter(ChannelRecorder* origin)
    : origin_(origin), error_occured_(false) {}

/**
 * @brief Destructor
 */
ChannelRecorderAdapter::~ChannelRecorderAdapter() {}

}   // namespace senscord
