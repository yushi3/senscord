/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_RAW_FILE_WRITER_H_
#define LIB_CORE_RECORD_RAW_FILE_WRITER_H_

#include <string>
#include <vector>
#include <map>
#include "senscord/osal.h"
#include "senscord/develop/channel_recorder.h"
#include "record/channel_properties_file_writer.h"
#include "record/raw_index_file_writer.h"

namespace senscord {

/**
 * @brief File writer for raw data.
 */
class RawFileWriter : private util::Noncopyable {
 public:
  /**
   * @brief Open the raw file.
   * @param[in] (output_dir_path) Recording target directory.
   * @return Status object.
   */
  Status Open(const std::string& output_dir_path);

  /**
   * @brief Close the raw file
   */
  void Close();

  /**
   * @brief Write as the raw data.
   * @param[in] (sequence_number) Frame sequence number.
   * @param[in] (sent_time) Time when frame was sent,
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  Status Write(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel);

  /**
   * @brief Constructor
   */
  RawFileWriter();

  /**
   * @brief Destructor
   */
  ~RawFileWriter();

 private:
  /**
   * @brief Write a channel.
   * @param[in] (file) Opened file pointer.
   * @param[in] (sequence_number) Frame sequence number.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  Status WriteFile(
    osal::OSFile* file, uint64_t sequence_number,
    const SerializedChannel& channel);

  /**
   * @brief Write the channel properties.
   * @param[in] (sequence_number) Frame sequence number.
   * @param[in] (channel) Serializeed channel data.
   * @return Status object.
   */
  Status WriteProperties(
    uint64_t sequence_number,
    const SerializedChannel& channel);

  /**
   * @brief Open the target file.
   * @param[in] (filepath) File path.
   * @param[out] (file) File handler.
   * @return Status object.
   */
  Status OpenBinaryFile(
    const std::string& filepath, osal::OSFile** file) const;

  /**
   * @brief Close the file.
   * @param[in] (file) Closing file.
   */
  void CloseBinaryFile(osal::OSFile* file) const;

  std::string output_dir_path_;

  // file pointer for each channels.
  osal::OSFile* channel_property_file_;
  // raw index file writer.
  RawIndexFileWriter* raw_index_file_writer_;
  // channel properties file writer.
  ChannelPropertiesFileWriter* channel_property_writer_;
};

}   // namespace senscord
#endif  // LIB_CORE_RECORD_RAW_FILE_WRITER_H_
