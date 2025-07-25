/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_RECORDER_PNM_INCLUDE_PNM_CHANNEL_RECORDER_H_
#define LIB_RECORDER_PNM_INCLUDE_PNM_CHANNEL_RECORDER_H_

#include <string>
#include <vector>

#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/osal.h"
#include "senscord/property_types.h"
#include "senscord/develop/channel_recorder.h"

// blockname for status
static const char kStatusBlockRecorder[] = "recorder";

/**
 * @brief Channel recorder for pnm type.
 */
class PnmChannelRecorder : private senscord::util::Noncopyable {
 public:
  /**
   * @brief Initialize for writing the channel.
   * @param[in] (channel_id) Channel ID.
   * @param[in] (output_dir_path) Output directory.
   * @return Status object.
   */
  senscord::Status Init(
    uint32_t channel_id,
    const std::string& output_dir_path);

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
  PnmChannelRecorder();

  /**
   * @brief Destructor
   */
  virtual ~PnmChannelRecorder();

 protected:
  /**
   * @brief Pnm format information.
   */
  struct PnmFormatInformation {
    const char* magic_number;  /**< Magic number */
    const char* luminance;     /**< Maximum luminance */
    const char* extension;     /**< File extension */
  };

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

  PnmFormatInformation format_info_;

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
   * @brief Create the image header.
   * @param[out] (header) image header.
   * @param[in] (property) Image property of channel.
   * @return Status object.
   */
  void CreateHeader(
    std::string* header,
    const senscord::ImageProperty& property) const;

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
 * @brief Channel recorder for pgm type.
 */
class PgmChannelRecorder8bit : public PnmChannelRecorder {
 public:
  /**
   * @brief Constructor
   */
  PgmChannelRecorder8bit();

  /**
   * @brief Destructor
   */
  ~PgmChannelRecorder8bit();
};

/**
 * @brief Channel recorder for pgm type.
 */
class PgmChannelRecorder16bit : public PnmChannelRecorder {
 public:
  /**
   * @brief Constructor
   */
  PgmChannelRecorder16bit();

  /**
   * @brief Destructor
   */
  ~PgmChannelRecorder16bit();

 private:
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

/**
 * @brief Channel recorder for ppm type.
 */
class PpmChannelRecorder : public PnmChannelRecorder {
 public:
  /**
   * @brief Constructor
   */
  PpmChannelRecorder();

  /**
   * @brief Destructor
   */
  ~PpmChannelRecorder();

 private:
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

#endif    // LIB_RECORDER_PNM_INCLUDE_PNM_CHANNEL_RECORDER_H_
