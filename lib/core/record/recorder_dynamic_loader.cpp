/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/recorder_dynamic_loader.h"
#include "record/recorder_dynamic_factory.h"

namespace senscord {

// Name of the function to be obtained from the library.
static const char kCreateInstance[] = "CreateRecorder";
static const char kDestroyInstance[] = "DestroyRecorder";

/**
 * @brief Generate an instance based on the recorder name of the argument.
 * @param[in] (name) Name of recorder.
 * @param[out] (recorder) Where to store the created recorder.
 * @return Status object.
 */
Status RecorderDynamicLoader::Create(
    const std::string& name, ChannelRecorder** recorder) {
  Status ret = ClassDynamicLoader::Create(
      name, reinterpret_cast<void**>(recorder));
  return SENSCORD_STATUS_TRACE(ret);
}

/**
 * @brief Delete the recorder passed in the argument.
 * @param[in] (name) Name of recorder.
 * @param[in] (recorder) Recorder to delete.
 * @return Status object.
 */
Status RecorderDynamicLoader::Destroy(
    const std::string& name, ChannelRecorder* recorder) {
  Status ret = ClassDynamicLoader::Destroy(name, recorder);
  return SENSCORD_STATUS_TRACE(ret);
}

/**
 * @brief A function that loads a library based on the argument name.
 * @param[in] (name) Key name of library.
 * @return Status object.
 */
Status RecorderDynamicLoader::Load(const std::string& name) {
  std::string file_path;
  Status ret = GetLibraryPath(name, &file_path);
  if (!ret.ok()) {
    return ret;
  }

  // Register factory as loader.
  RecorderDynamicFactory* factory = new RecorderDynamicFactory();
  ret = LoadAndRegisterLibrary(file_path, kCreateInstance, kDestroyInstance,
      factory);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    delete factory;
    return ret;
  }

  SetFactory(name, factory);
  return Status::OK();
}

/**
 * @brief Constructor.
 */
RecorderDynamicLoader::RecorderDynamicLoader() {}

/**
 * @brief Destructor.
 */
RecorderDynamicLoader::~RecorderDynamicLoader() {}

}   // namespace senscord
