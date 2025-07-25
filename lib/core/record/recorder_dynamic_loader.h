/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_RECORDER_DYNAMIC_LOADER_H_
#define LIB_CORE_RECORD_RECORDER_DYNAMIC_LOADER_H_

#include <string>
#include "loader/class_dynamic_loader.h"
#include "senscord/develop/channel_recorder.h"

namespace senscord {

/**
 * @brief Component dynamic loader.
 */
class RecorderDynamicLoader : public ClassDynamicLoader {
 public:
  /**
   * @brief Generate an instance based on the recorder name of the argument.
   * @param[in] (name) Name of recorder.
   * @param[out] (recorder) Where to store the created recorder.
   * @return Status object.
   */
  Status Create(const std::string& name, ChannelRecorder** recorder);

  /**
   * @brief Delete the recorder passed in the argument.
   * @param[in] (name) Name of recorder.
   * @param[in] (recorder) Recorder to delete.
   * @return Status object.
   */
  Status Destroy(const std::string& name, ChannelRecorder* recorder);

  /**
   * @brief Constructor.
   */
  RecorderDynamicLoader();

  /**
   * @brief Destructor.
   */
  ~RecorderDynamicLoader();

 protected:
  /**
   * @brief A function that loads a library based on the argument name.
   * @param[in] (name) Key name of library.
   * @return Status object.
   */
  virtual Status Load(const std::string& name);
};

}   // namespace senscord

#endif  // LIB_CORE_RECORD_RECORDER_DYNAMIC_LOADER_H_
