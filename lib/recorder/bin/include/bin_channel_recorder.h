/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_RECORDER_BIN_INCLUDE_BIN_CHANNEL_RECORDER_H_
#define LIB_RECORDER_BIN_INCLUDE_BIN_CHANNEL_RECORDER_H_

#include <string>
#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/osal.h"
#include "senscord/property_types.h"
#include "senscord/develop/channel_recorder.h"

/**
 * @brief Channel recorder for bin type.
 *  - Target input bit wides: 16-bit image data.
 */
class BinChannelRecorder : private senscord::util::Noncopyable {
 public:
  /**
   * @brief Initialize for writing the channel.
   * @param[in] (channel_id) Channel ID.
   * @param[in] (output_dir_path) Output directory.
   * @return Status object.
   */
  senscord::Status Init(
    uint32_t channel_id, const std::string& output_dir_path);

  /**
   * @brief Write the channel.
   * @param[in] (sequence_number) Frame sequence number.
   * @param[in] (property) Image property of channel.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  senscord::Status Write(
    uint64_t sequence_number,
    const senscord::ImageProperty& property,
    const senscord::SerializedChannel& channel);

  /**
   * @brief Constructor
   */
  BinChannelRecorder();

  /**
   * @brief Destructor
   */
  virtual ~BinChannelRecorder();

 protected:
  /**
   * @brief Write the channel data to file.
   * @param[in] (file) Destination file pointer.
   * @param[in] (property) Image property of channel.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  virtual senscord::Status WritePayload(
    senscord::osal::OSFile* file,
    const senscord::ImageProperty& property,
    const senscord::SerializedChannel& channel);

 private:
  /**
   * @brief Create the output file name.
   * @param[out] (file_name) Output file name.
   * @param[in] (sequence_number) Frame sequence number.
   * @param[in] (channel) Serialized channel data.
   */
  void CreateFileName(
    std::string* file_name,
    uint64_t sequence_number,
    const senscord::SerializedChannel& channel) const;

  /**
   * @brief Write the file header.
   * @param[out] (file_name) Output file name.
   * @param[in] (property) Image property of channel.
   * @return Status object.
   */
  senscord::Status WriteHeader(
    senscord::osal::OSFile* file,
    const senscord::ImageProperty& property) const;

  std::string dir_path_;
};

/**
 * @brief Channel recorder for bin type.
 *  - Target input bit wides: 8-bit image data.
 */
class BinChannelRecorder8bit : public BinChannelRecorder {
 public:
  /**
   * @brief Constructor
   */
  BinChannelRecorder8bit();

  /**
   * @brief Destructor
   */
  ~BinChannelRecorder8bit();

 protected:
  /**
   * @brief Write the channel data to file.
   * @param[in] (file) Destination file pointer.
   * @param[in] (property) Image property of channel.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  virtual senscord::Status WritePayload(
    senscord::osal::OSFile* file,
    const senscord::ImageProperty& property,
    const senscord::SerializedChannel& channel);
};

#endif    // LIB_RECORDER_BIN_INCLUDE_BIN_CHANNEL_RECORDER_H_
