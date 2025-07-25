/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core/stream_manager.h"

#include <vector>
#include <algorithm>    // std::find
#include <string>

#include "logger/logger.h"
#include "util/autolock.h"

namespace senscord {

/**
 * @brief Constructor.
 */
StreamManager::StreamManager() {
}

/**
 * @brief Destructor.
 */
StreamManager::~StreamManager() {
  ReleaseStreamAll();
}

/**
 * @brief Get the new stream instance.
 * @param (config) Stream coniguration to get.
 * @param (stream_core) New stream pointer.
 * @return Status object.
 */
Status StreamManager::GetStream(const StreamSetting& config,
                                StreamCore** stream_core) {
  if (stream_core == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  // garbage collection
  DeleteReleasedStream();

  // new resource
  *stream_core = new StreamCore();

  Status status = (*stream_core)->Init(config);
  if (!status.ok()) {
    delete *stream_core;
    *stream_core = NULL;
    return SENSCORD_STATUS_TRACE(status);
  }

  util::AutoLock lock(&list_mutex_);
  stream_list_.push_back(*stream_core);
  return Status::OK();
}

/**
 * @brief Get the registered stream instance.
 * @return Acquired stream. null is empty.
 */
StreamCore* StreamManager::GetRegisteredStream() {
  util::AutoLock lock(&list_mutex_);
  if (!stream_list_.empty()) {
    return stream_list_.front();
  }
  return NULL;  // empty stream
}

/**
 * @brief Release the stream instance.
 * @param (stream_core) Acquired stream.
 * @return Status object.
 */
Status StreamManager::ReleaseStream(StreamCore* stream_core) {
  util::AutoLock lock(&list_mutex_);
  StreamList::iterator itr =
    std::find(stream_list_.begin(), stream_list_.end(), stream_core);

  if (itr != stream_list_.end()) {
    stream_list_.erase(itr);
    released_stream_list_.push_back(stream_core);
    return Status::OK();
  }
  return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
      "stream not found: %p", stream_core);
}

/**
 * @brief Release all stream instances.
 */
void StreamManager::ReleaseStreamAll() {
  util::AutoLock lock(&list_mutex_);
  // shift all stream to the released list.
  while (!stream_list_.empty()) {
    StreamCore* stream_core = stream_list_.back();
    ReleaseStream(stream_core);
  }
  // wait for the access to be completed and then delete it.
  while (!released_stream_list_.empty()) {
    StreamCore* stream_core = released_stream_list_.back();
    stream_core->WaitForReleasable();
    delete stream_core;
    released_stream_list_.pop_back();
  }
}

/**
 * @brief Get the stream key.
 * @param[in] (stream_core) Stream instance.
 * @param[out] (stream_key) Stream key.
 * @return Status object.
 */
Status StreamManager::GetStreamKey(StreamCore* stream_core,
                                   std::string* stream_key) {
  if (stream_key == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "stream_key is NULL");
  }
  util::AutoLock lock(&list_mutex_);
  StreamList::iterator itr =
      std::find(stream_list_.begin(), stream_list_.end(), stream_core);
  if (itr == stream_list_.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "stream not found: %p", stream_core);
  }
  *stream_key = stream_core->GetKey();
  return Status::OK();
}

/**
 * @brief Delete the released stream instance.
 */
void StreamManager::DeleteReleasedStream() {
  util::AutoLock lock(&list_mutex_);
  StreamList::iterator itr = released_stream_list_.begin();
  while (itr != released_stream_list_.end()) {
    StreamCore* stream_core = *itr;
    if (stream_core->IsReleasable()) {
      itr = released_stream_list_.erase(itr);
      delete stream_core;
    } else {
      ++itr;
    }
  }
}

}   // namespace senscord
