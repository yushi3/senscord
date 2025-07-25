/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/recorder_manager.h"
#include <string>
#include <vector>
#include "core/internal_types.h"
#include "util/autolock.h"
#include "util/senscord_utils.h"
#include "util/singleton.h"
#include "logger/logger.h"

namespace senscord {

/**
 * @brief Get the manager instance.
 * @return Manager instance.
 */
RecorderManager* RecorderManager::GetInstance() {
  // for private constructor / destructor
  struct InnerRecorderManager : public RecorderManager {
  };
  return util::Singleton<InnerRecorderManager>::GetInstance();
}

/**
 * @brief Initialize and read config file.
 * @return Status object.
 */
Status RecorderManager::Init() {
#ifdef SENSCORD_RECORDER_LOADER
  util::AutoLock autolock(&mutex_);
  if (initialized_) {
    return Status::OK();
  }

  // get recorder config path
  std::string path;
  if (util::SearchFileFromEnv(kRecorderConfigFile, &path)) {
    // read configure
    Status status = config_manager_.ReadConfig(path);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  } else {
    // if no recorder config, through initialize.
    SENSCORD_LOG_DEBUG("%s not found.", kRecorderConfigFile);
  }
  initialized_ = true;
#endif  // SENSCORD_RECORDER_LOADER
  return Status::OK();
}

#ifdef SENSCORD_RECORDER_LOADER
/**
 * @brief Create the new recorder.
 * @param[in] (format_name) Recording format type.
 * @param[out] (recorder) New recorder.
 * @return Status object.
 */
Status RecorderManager::CreateRecorder(
    const std::string& format_name, ChannelRecorder** recorder) {
  if (recorder == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // get the recorder name
  std::string recorder_name;
  Status status = config_manager_.GetRecorderType(
      format_name, &recorder_name);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // load recorder
  util::AutoLock autolock(&mutex_);
  ChannelRecorder* implemented = NULL;
  status = loader_.Create(recorder_name, &implemented);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  *recorder = implemented;
  return Status::OK();
}

/**
 * @brief Release the recorder.
 * @param[in] (format_name) Recording format type.
 * @param[in] (recorder) Created recorder.
 * @return Status object.
 */
Status RecorderManager::ReleaseRecorder(
    const std::string& format_name, ChannelRecorder* recorder) {
  if (recorder == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // get the recorder name
  std::string recorder_name;
  Status status = config_manager_.GetRecorderType(
      format_name, &recorder_name);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  util::AutoLock autolock(&mutex_);
  status = loader_.Destroy(recorder_name, recorder);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  return Status::OK();
}

/**
 * @brief Get the recordable format list.
 * @param[out] (formats) List of formats.
 * @return Status object.
 */
Status RecorderManager::GetRecordableFormats(
    std::vector<std::string>* formats) const {
  if (formats == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  Status status = config_manager_.GetRecordableFormats(formats);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_RECORDER_LOADER

#ifdef SENSCORD_RECORDER_RAW
/**
 * @brief Attach to raw index file writer.
 * @param[in] (output_dir_path) Recording target directory.
 * @param[out] (writer) Attached raw index file writer.
 * @return Status object.
 */
Status RecorderManager::AttachRawIndexFileWriter(
    const std::string& output_dir_path, RawIndexFileWriter** writer) {
  if (writer == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  util::AutoLock autolock(&mutex_writer_);
  RawIndexFileWriter* tmp_writer = GetRawIndexFileWriter(output_dir_path);
  if (tmp_writer == NULL) {
    tmp_writer = new RawIndexFileWriter();
    Status status = tmp_writer->Open(output_dir_path);
    if (!status.ok()) {
      delete tmp_writer;
      return SENSCORD_STATUS_TRACE(status);
    }
    raw_index_file_writers_[output_dir_path] = tmp_writer;
  }
  tmp_writer->AddReference();
  *writer = tmp_writer;
  return Status::OK();
}

/**
 * @brief Detach the raw index file writer.
 * @param[in] (output_dir_path) Recording target directory.
 * @return Status object.
 */
Status RecorderManager::DetachRawIndexFileWriter(
    const std::string& output_dir_path) {
  util::AutoLock autolock(&mutex_writer_);
  RawIndexFileWriter* writer = GetRawIndexFileWriter(output_dir_path);
  if (writer == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound,
        "instance not found : output_dir_path=%s",
        output_dir_path.c_str());
  }
  writer->ReleaseReference();
  if (writer->GetReferenceCount() == 0) {
    writer->Close();
    raw_index_file_writers_.erase(output_dir_path);
    delete writer;
  }
  return Status::OK();
}

/**
 * @brief Get the raw index file writer.
 * @param[in] (output_dir_path) Recording target directory.
 * @return Raw index file writer.
 */
RawIndexFileWriter* RecorderManager::GetRawIndexFileWriter(
    const std::string& output_dir_path) const {
  std::map<std::string, RawIndexFileWriter*>::const_iterator itr =
      raw_index_file_writers_.find(output_dir_path);
  if (itr == raw_index_file_writers_.end()) {
    return NULL;
  }
  return itr->second;
}
#endif  // SENSCORD_RECORDER_RAW

/**
 * @brief Constructor.
 */
RecorderManager::RecorderManager() : initialized_(false) {}

/**
 * @brief Destructor.
 */
RecorderManager::~RecorderManager() {}

}   // namespace senscord
