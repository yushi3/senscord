/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_RAW_INDEX_FILE_WRITER_H_
#define LIB_CORE_RECORD_RAW_INDEX_FILE_WRITER_H_

#include <string>
#include <vector>
#include <map>
#include "senscord/osal.h"
#include "senscord/develop/channel_recorder.h"

namespace senscord {

/**
 * @brief File writer for raw related files.
 */
class RawIndexFileWriter : private util::Noncopyable {
 public:
  /**
   * @brief Open the index file.
   * @param[in] (output_dir_path) Recording target directory.
   * @return Status object.
   */
  Status Open(const std::string& output_dir_path);

  /**
   * @brief Close the index file
   */
  void Close();

  /**
   * @brief Write as the raw data.
   * @param[in] (sequence_number) Frame sequence number.
   * @param[in] (sent_time) Time when frame was sent,
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  Status WriteRaw(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel);

  /**
   * @brief Write as the comopsite raw data.
   * @param[in] (sequence_number) Frame sequence number.
   * @param[in] (sent_time) Time when frame was sent,
   * @param[in] (channel) Serialized channel data.
   * @return Status object.
   */
  Status WriteCompositeRaw(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel);

  /**
   * @brief Add reference.
   */
  void AddReference();

  /**
   * @brief Release reference.
   */
  void ReleaseReference();

  /**
   * @brief Get reference count.
   */
  uint32_t GetReferenceCount();

  /**
   * @brief Constructor
   */
  RawIndexFileWriter();

  /**
   * @brief Destructor
   */
  ~RawIndexFileWriter();

 private:
  /**
   * @brief Open the index file.
   * @return Status object.
   */
  Status OpenIndexFile();

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
  osal::OSFile* index_file_;
  uint32_t reference_count_;
};

}   // namespace senscord
#endif  // LIB_CORE_RECORD_RAW_INDEX_FILE_WRITER_H_
