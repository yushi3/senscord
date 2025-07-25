/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_CHANNEL_PROPERTIES_FILE_WRITER_H_
#define LIB_CORE_RECORD_CHANNEL_PROPERTIES_FILE_WRITER_H_

#include <string>
#include <vector>
#include <map>
#include "senscord/osal.h"
#include "senscord/develop/channel_recorder.h"
#include "record/raw_index_file_writer.h"

namespace senscord {

/**
 * @brief File writer for channel properties files.
 */
class ChannelPropertiesFileWriter : private util::Noncopyable {
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
   * @brief Write the channel properties.
   * @param[in] (sequence_number) Frame sequence number.
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  Status Write(uint64_t sequence_number, const SerializedChannel& channel);

  /**
   * @brief Constructor
   */
  ChannelPropertiesFileWriter();

  /**
   * @brief Destructor
   */
  ~ChannelPropertiesFileWriter();

 private:
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

  /**
   * @brief Write the binary to the file.
   * @param[in] (file) Target file.
   * @param[in] (buffer) Write data.
   * @param[in] (buffer_size) Write data size.
   * @return Status object.
   */
  Status WriteBinaryFile(
    osal::OSFile* file, const void* buffer, size_t buffer_size) const;

  std::string output_dir_path_;

  // file pointer for each channels.
  osal::OSFile* channel_property_file_;
};

}   // namespace senscord
#endif  // LIB_CORE_RECORD_CHANNEL_PROPERTIES_FILE_WRITER_H_
