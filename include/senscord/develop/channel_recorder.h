/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_CHANNEL_RECORDER_H_
#define SENSCORD_DEVELOP_CHANNEL_RECORDER_H_

#include "senscord/config.h"

#ifdef SENSCORD_RECORDER

#include <string>
#include <vector>
#include <map>

#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/stream.h"

namespace senscord {

/**
 * @brief Serialized and copied channel data.
 */
struct SerializedChannel {
  uint32_t id;                      /**< Channel ID */
  uint64_t timestamp;               /**< Timestamp of captured raw data */
  std::string type;                 /**< RawData type */
  std::vector<uint8_t> rawdata;     /**< Copied Raw Data */

  /** Contained properties each property key. */
  std::map<std::string, BinaryProperty> properties;

  /** Keys of updated properties. */
  std::vector<std::string> updated_property_keys;
};

/**
 * @brief Recorder for channel.
 */
class ChannelRecorder : private util::Noncopyable {
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
    Stream* stream) = 0;

  /**
   * @brief Stop to record.
   */
  virtual void Stop() = 0;

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
    const SerializedChannel& channel) = 0;

  /**
   * @brief Destructor
   */
  virtual ~ChannelRecorder() {}
};

}   // namespace senscord

/**
 * @def Macro for the new recorder registration.
 */
#define SENSCORD_REGISTER_RECORDER(RECORDER_CLASS_NAME) \
  extern "C" void* CreateRecorder() {    \
    return new RECORDER_CLASS_NAME();      \
  }     \
  extern "C" void DestroyRecorder(void* recorder) {   \
    delete reinterpret_cast<senscord::ChannelRecorder*>(recorder);    \
  }

#endif  // SENSCORD_RECORDER
#endif  // SENSCORD_DEVELOP_CHANNEL_RECORDER_H_
