/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CORE_STREAM_MANAGER_H_
#define LIB_CORE_CORE_STREAM_MANAGER_H_

#include <stdint.h>
#include <vector>
#include <string>

#include "senscord/stream.h"
#include "stream/stream_core.h"
#include "core/internal_types.h"
#include "util/mutex.h"

namespace senscord {

/**
 * @brief Stream instance management class.
 */
class StreamManager {
 public:
  /**
   * @brief Get the new stream instance.
   * @param (config) Stream coniguration to get.
   * @param (stream_core) New stream pointer.
   * @return Status object.
   */
  Status GetStream(const StreamSetting& config, StreamCore** stream_core);

  /**
   * @brief Get the registered stream instance.
   * @return Acquired stream. null is empty.
   */
  StreamCore* GetRegisteredStream();

  /**
   * @brief Release the stream instance.
   * @param (stream_core) Acquired stream.
   * @return Status object.
   */
  Status ReleaseStream(StreamCore* stream_core);

  /**
   * @brief Release all stream instances.
   */
  void ReleaseStreamAll();

  /**
   * @brief Get the stream key.
   * @param[in] (stream_core) Stream instance.
   * @param[out] (stream_key) Stream key.
   * @return Status object.
   */
  Status GetStreamKey(StreamCore* stream_core, std::string* stream_key);

  /**
   * @brief Constructor.
   */
  StreamManager();

  /**
   * @brief Destructor.
   */
  ~StreamManager();

 private:
  /**
   * @brief Delete the released stream instance.
   */
  void DeleteReleasedStream();

  typedef std::vector<StreamCore*> StreamList;
  StreamList stream_list_;
  StreamList released_stream_list_;
  util::Mutex list_mutex_;
};

}   // namespace senscord
#endif  // LIB_CORE_CORE_STREAM_MANAGER_H_
