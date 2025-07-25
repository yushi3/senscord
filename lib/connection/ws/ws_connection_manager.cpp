/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ws_connection_manager.h"
#include <inttypes.h>
#include <utility>
#include "senscord/logger.h"

namespace {

/**
 * @brief Link the primary and secondary.
 */
void Link(const senscord::ws::ConnectionInfo& info) {
  if (info.primary != nullptr && info.secondary != nullptr) {
    info.primary->SetSecondary(info.secondary);
    info.secondary->SetPrimary(info.primary);
  }
}

/**
 * @brief Unlink the primary and secondary.
 */
void Unlink(const senscord::ws::ConnectionInfo& info) {
  if (info.primary != nullptr) {
    info.primary->SetSecondary(nullptr);
  }
  if (info.secondary != nullptr) {
    info.secondary->SetPrimary(nullptr);
  }
}

}  // namespace

namespace senscord {
namespace ws {

/**
 * @brief Get manager instance.
 * @return Manager instance.
 */
WsConnectionManager* WsConnectionManager::GetInstance() {
  static WsConnectionManager instance;
  return &instance;
}

/**
 * @brief Constructor
 */
WsConnectionManager::WsConnectionManager() :
    mutex_connections_(), mutex_stream_ids_() {
  osal::OSCreateMutex(&mutex_connections_);
  osal::OSCreateMutex(&mutex_stream_ids_);
}

/**
 * @brief Destructor.
 */
WsConnectionManager::~WsConnectionManager() {
  if (mutex_stream_ids_ != NULL) {
    osal::OSDestroyMutex(mutex_stream_ids_);
    mutex_stream_ids_ = NULL;
  }
  if (mutex_connections_ != NULL) {
    osal::OSDestroyMutex(mutex_connections_);
    mutex_connections_ = NULL;
  }
}

/**
 * @brief Register the primary connection.
 * @param[in] stream_id   The stream id that acts as the search key.
 * @param[in] connection  The primary connection instance to register.
 */
void WsConnectionManager::RegisterPrimaryConnection(
    uint64_t stream_id, WsConnection* connection) {
  AutoMutex lock(mutex_connections_);
  ConnectionInfo& info = connections_[stream_id];
  Unlink(info);
  info.primary = connection;
  Link(info);
}

/**
 * @brief Register the secondary connection.
 * @param[in] stream_id   The stream id that acts as the search key.
 * @param[in] connection  The secondary connection instance to register.
 */
void WsConnectionManager::RegisterSecondaryConnection(
    uint64_t stream_id, WsConnection* connection) {
  AutoMutex lock(mutex_connections_);
  ConnectionInfo& info = connections_[stream_id];
  Unlink(info);
  info.secondary = connection;
  Link(info);
}

/**
 * @brief Unregister the connection.
 * @param[in] stream_id   The stream id that acts as the search key.
 * @param[in] connection  The connection instance to unregister.
 */
void WsConnectionManager::UnregisterConnection(
    uint64_t stream_id, WsConnection* connection) {
  AutoMutex lock(mutex_connections_);

  std::map<uint64_t, ConnectionInfo>::iterator itr =
      connections_.find(stream_id);
  if (itr != connections_.end()) {
    ConnectionInfo& info = itr->second;
    if (info.primary == connection) {
      Unlink(info);
      info.primary = nullptr;
    } else if (info.secondary == connection) {
      Unlink(info);
      info.secondary = nullptr;
    }

    if (info.primary == nullptr && info.secondary == nullptr) {
      connections_.erase(itr);
    }
  }
}

/**
 * @brief Get the connection information.
 * @param[in]  stream_id  The stream id that acts as the search key.
 * @param[out] info       The registered connection information.
 * @return Status object.
 */
Status WsConnectionManager::GetConnection(
    uint64_t stream_id, ConnectionInfo* info) const {
  if (info == nullptr) {
    return SENSCORD_STATUS_FAIL(
        "ws", Status::kCauseInvalidArgument, "info == null");
  }

  AutoMutex lock(mutex_connections_);
  std::map<uint64_t, ConnectionInfo>::const_iterator itr =
      connections_.find(stream_id);
  if (itr == connections_.end()) {
    return SENSCORD_STATUS_FAIL(
        "ws", Status::kCauseInvalidArgument, "unmanaged stream id: %" PRIx64,
        stream_id);
  }

  *info = itr->second;
  return Status::OK();
}

/**
 * @brief Register the stream handle.
 * @param[in] handle     The handle that acts as the search key.
 * @param[in] stream_id  The stream id to register.
 */
void WsConnectionManager::RegisterHandle(
    const std::string& handle, uint64_t stream_id) {
  AutoMutex lock(mutex_stream_ids_);
  stream_ids_[handle] = stream_id;
}

/**
 * @brief Unregister the stream handle.
 * @param[in] handle     The handle that acts as the search key.
 * @param[in] stream_id  The stream id to unregister.
 */
void WsConnectionManager::UnregisterHandle(const std::string& handle) {
  AutoMutex lock(mutex_stream_ids_);
  stream_ids_.erase(handle);
}

/**
 * @brief Get the stream id.
 * @param[in]  handle     The handle that acts as the search key.
 * @param[out] stream_id  The registered stream id.
 * @return Status object.
 */
Status WsConnectionManager::GetStreamId(
    const std::string& handle, uint64_t* stream_id) const {
  if (stream_id == nullptr) {
    return SENSCORD_STATUS_FAIL(
        "ws", Status::kCauseInvalidArgument, "stream_id == null");
  }

  AutoMutex lock(mutex_stream_ids_);
  std::map<std::string, uint64_t>::const_iterator itr =
      stream_ids_.find(handle);
  if (itr == stream_ids_.end()) {
    return SENSCORD_STATUS_FAIL(
        "ws", Status::kCauseInvalidArgument, "unmanaged handle: %s",
        handle.c_str());
  }

  *stream_id = itr->second;
  return Status::OK();
}

}  // namespace ws
}  // namespace senscord
