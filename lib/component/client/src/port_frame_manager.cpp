/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "port_frame_manager.h"

#include <inttypes.h>
#include <set>
#include <utility>
#include "senscord/osal_inttypes.h"
#include "./client_log.h"

namespace client {

/**
 * @brief Parameters of each port.
 */
struct PortParameter {
  /** Set to true when stream stopped. */
  bool stopped;
  /** Manages the frame sequence number.*/
  std::set<uint64_t> frames;
};

/**
 * @brief Constructor.
 * @param[in] (listener) Frame event listener.
 */
PortFrameManager::PortFrameManager(PortFrameEventListener* listener)
    : listener_(listener), list_(), mutex_() {
  senscord::osal::OSCreateMutex(&mutex_);
}

/**
 * @brief Destructor.
 */
PortFrameManager::~PortFrameManager() {
  senscord::osal::OSLockMutex(mutex_);
  for (std::map<int32_t, PortParameter*>::const_iterator itr = list_.begin(),
      end = list_.end(); itr != end; ++itr) {
    SENSCORD_CLIENT_LOG_INFO(
        "leak: port[%" PRId32 "]: stopped=%d, frames=%" PRIuS,
        itr->first, itr->second->stopped, itr->second->frames.size());
    delete itr->second;
  }
  list_.clear();
  senscord::osal::OSUnlockMutex(mutex_);

  senscord::osal::OSDestroyMutex(mutex_);
  mutex_ = NULL;
}

/**
 * @brief Set the specified port to the stream start state.
 * @param[in] (port_id) Port ID.
 * @return Status object.
 */
senscord::Status PortFrameManager::Start(int32_t port_id) {
  if (listener_ == NULL) {
    return SENSCORD_STATUS_FAIL("client",
        senscord::Status::kCauseInvalidArgument,
        "port[%" PRId32 "]: listener is null", port_id);
  }

  senscord::Status status;

  senscord::osal::OSLockMutex(mutex_);
  std::map<int32_t, PortParameter*>::const_iterator itr = list_.find(port_id);
  if (itr == list_.end()) {
    PortParameter* param = new PortParameter;
    param->stopped = false;
    list_.insert(std::make_pair(port_id, param));
  } else {
    status = SENSCORD_STATUS_FAIL("client",
        senscord::Status::kCauseAlreadyExists,
        "port[%" PRId32 "]: already exists", port_id);
  }
  senscord::osal::OSUnlockMutex(mutex_);

  return status;
}

/**
 * @brief Set the specified port to the stream stop state.
 * @param[in] (port_id) Port ID.
 * @return Status object.
 */
senscord::Status PortFrameManager::Stop(int32_t port_id) {
  senscord::Status status;
  bool release = false;

  senscord::osal::OSLockMutex(mutex_);
  std::map<int32_t, PortParameter*>::iterator itr = list_.find(port_id);
  if (itr != list_.end()) {
    PortParameter* param = itr->second;
    param->stopped = true;
    release = param->frames.empty();
    if (release) {
      delete param;
      list_.erase(itr);
    }
  } else {
    status = SENSCORD_STATUS_FAIL("client",
        senscord::Status::kCauseNotFound,
        "port[%" PRId32 "]: not found", port_id);
  }
  senscord::osal::OSUnlockMutex(mutex_);

  if (release) {
    listener_->OnReleaseAllFrames(port_id);
  }

  return status;
}

/**
 * @brief Add the frame to the management target.
 * @param[in] (port_id) Port ID.
 * @param[in] (sequence_number) Frame sequence number.
 * @return Status object.
 */
senscord::Status PortFrameManager::AddFrame(
    int32_t port_id, uint64_t sequence_number) {
  senscord::Status status;

  senscord::osal::OSLockMutex(mutex_);
  std::map<int32_t, PortParameter*>::const_iterator itr = list_.find(port_id);
  if (itr != list_.end()) {
    PortParameter* param = itr->second;
    if (!param->frames.insert(sequence_number).second) {
      status = SENSCORD_STATUS_FAIL("client",
          senscord::Status::kCauseAlreadyExists,
          "port[%" PRId32 "]: sequence number[%" PRIu64 "]: already exists",
          port_id, sequence_number);
    }
  } else {
    status = SENSCORD_STATUS_FAIL("client",
        senscord::Status::kCauseNotFound,
        "port[%" PRId32 "]: not found (sequence number=%" PRIu64 ")",
        port_id, sequence_number);
  }
  senscord::osal::OSUnlockMutex(mutex_);

  return status;
}

/**
 * @brief Remove the frame from the management target.
 * @param[in] (port_id) Port ID.
 * @param[in] (sequence_number) Frame sequence number.
 * @return Status object.
 */
senscord::Status PortFrameManager::RemoveFrame(
    int32_t port_id, uint64_t sequence_number) {
  senscord::Status status;
  bool release = false;

  senscord::osal::OSLockMutex(mutex_);
  std::map<int32_t, PortParameter*>::iterator itr = list_.find(port_id);
  if (itr != list_.end()) {
    PortParameter* param = itr->second;
    param->frames.erase(sequence_number);
    release = param->stopped && param->frames.empty();
    if (release) {
      delete param;
      list_.erase(itr);
    }
  } else {
    status = SENSCORD_STATUS_FAIL("client",
        senscord::Status::kCauseNotFound,
        "port[%" PRId32 "]: not found (sequence number=%" PRIu64 ")",
        port_id, sequence_number);
  }
  senscord::osal::OSUnlockMutex(mutex_);

  if (release) {
    listener_->OnReleaseAllFrames(port_id);
  }

  return status;
}

}  // namespace client
