/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_RECORDER_MANAGER_H_
#define LIB_CORE_RECORD_RECORDER_MANAGER_H_

#include <string>
#include <map>
#include <vector>

#include "senscord/config.h"
#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "util/mutex.h"

#ifdef SENSCORD_RECORDER_RAW
#include "record/raw_index_file_writer.h"
#endif  // SENSCORD_RECORDER_RAW

#ifdef SENSCORD_RECORDER_LOADER
#include "record/recorder_config_manager.h"
#include "record/recorder_dynamic_loader.h"
#endif  // SENSCORD_RECORDER_LOADER

namespace senscord {

/**
 * @brief Manager of recorders (singleton).
 */
class RecorderManager : private util::Noncopyable {
 public:
  /**
   * @brief Get the manager instance.
   * @return Manager instance.
   */
  static RecorderManager* GetInstance();

  /**
   * @brief Initialize and read config file.
   * @return Status object.
   */
  Status Init();

#ifdef SENSCORD_RECORDER_LOADER
  /**
   * @brief Create the new recorder.
   * @param[in] (format_name) Recording format type.
   * @param[out] (recorder) New recorder.
   * @return Status object.
   */
  Status CreateRecorder(
    const std::string& format_name, ChannelRecorder** recorder);

  /**
   * @brief Release the recorder.
   * @param[in] (format_name) Recording format type.
   * @param[in] (recorder) Created recorder.
   * @return Status object.
   */
  Status ReleaseRecorder(
    const std::string& format_name, ChannelRecorder* recorder);

  /**
   * @brief Get the recordable format list.
   * @param[out] (formats) List of formats.
   * @return Status object.
   */
  Status GetRecordableFormats(std::vector<std::string>* formats) const;
#endif  // SENSCORD_RECORDER_LOADER

#ifdef SENSCORD_RECORDER_RAW
  /**
   * @brief Attach to raw index file writer.
   * @param[in] (output_dir_path) Recording target directory.
   * @param[out] (writer) Attached raw index file writer.
   * @return Status object.
   */
  Status AttachRawIndexFileWriter(
    const std::string& output_dir_path, RawIndexFileWriter** writer);

  /**
   * @brief Detach the raw index file writer.
   * @param[in] (output_dir_path) Recording target directory.
   * @return Status object.
   */
  Status DetachRawIndexFileWriter(const std::string& output_dir_path);
#endif  // SENSCORD_RECORDER_RAW

 private:
#ifdef SENSCORD_RECORDER_RAW
  /**
   * @brief Get the raw index file writer.
   * @param[in] (output_dir_path) Recording target directory.
   * @return Raw index file writer.
   */
  RawIndexFileWriter* GetRawIndexFileWriter(
    const std::string& output_dir_path) const;
#endif  // SENSCORD_RECORDER_RAW

  /**
   * @brief Constructor.
   */
  RecorderManager();

  /**
   * @brief Destructor.
   */
  ~RecorderManager();

  bool initialized_;
#ifdef SENSCORD_RECORDER_LOADER
  util::Mutex mutex_;
  RecorderConfigManager config_manager_;
  RecorderDynamicLoader loader_;
#endif  // SENSCORD_RECORDER_LOADER

#ifdef SENSCORD_RECORDER_RAW
  // raw index file writer
  util::Mutex mutex_writer_;
  std::map<std::string, RawIndexFileWriter*> raw_index_file_writers_;
#endif  // SENSCORD_RECORDER_RAW
};

}   // namespace senscord

#endif  // LIB_CORE_RECORD_RECORDER_MANAGER_H_
