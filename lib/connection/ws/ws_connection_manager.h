/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CONNECTION_WS_WS_CONNECTION_MANAGER_H_
#define LIB_CONNECTION_WS_WS_CONNECTION_MANAGER_H_

#include <map>
#include <string>
#include "senscord/status.h"
#include "senscord/noncopyable.h"
#include "senscord/osal.h"
#include "./ws_connection.h"

namespace senscord {
namespace ws {

struct ConnectionInfo {
  WsConnection* primary;
  WsConnection* secondary;
};

/**
 * @brief WebSocket connection manager.
 */
class WsConnectionManager : private util::Noncopyable {
 public:
  /**
   * @brief Get manager instance.
   * @return Manager instance.
   */
  static WsConnectionManager* GetInstance();

  /**
   * @brief Register the primary connection.
   * @param[in] stream_id   The stream id that acts as the search key.
   * @param[in] connection  The primary connection instance to register.
   */
  void RegisterPrimaryConnection(
      uint64_t stream_id, WsConnection* connection);

  /**
   * @brief Register the secondary connection.
   * @param[in] stream_id   The stream id that acts as the search key.
   * @param[in] connection  The secondary connection instance to register.
   */
  void RegisterSecondaryConnection(
      uint64_t stream_id, WsConnection* connection);

  /**
   * @brief Unregister the connection.
   * @param[in] stream_id   The stream id that acts as the search key.
   * @param[in] connection  The connection instance to unregister.
   */
  void UnregisterConnection(
      uint64_t stream_id, WsConnection* connection);

  /**
   * @brief Get the connection information.
   * @param[in]  stream_id  The stream id that acts as the search key.
   * @param[out] info       The registered connection information.
   * @return Status object.
   */
  Status GetConnection(uint64_t stream_id, ConnectionInfo* info) const;

  /**
   * @brief Register the stream handle.
   * @param[in] handle     The handle that acts as the search key.
   * @param[in] stream_id  The stream id to register.
   */
  void RegisterHandle(const std::string& handle, uint64_t stream_id);

  /**
   * @brief Unregister the stream handle.
   * @param[in] handle     The handle that acts as the search key.
   * @param[in] stream_id  The stream id to unregister.
   */
  void UnregisterHandle(const std::string& handle);

  /**
   * @brief Get the stream id.
   * @param[in]  handle     The handle that acts as the search key.
   * @param[out] stream_id  The registered stream id.
   * @return Status object.
   */
  Status GetStreamId(const std::string& handle, uint64_t* stream_id) const;

 private:
  /**
   * @brief Constructor
   */
  WsConnectionManager();

  /**
   * @brief Destructor.
   */
  ~WsConnectionManager();

 private:
  std::map<uint64_t, ConnectionInfo> connections_;
  osal::OSMutex* mutex_connections_;
  std::map<std::string, uint64_t> stream_ids_;
  osal::OSMutex* mutex_stream_ids_;
};

}  // namespace ws
}  // namespace senscord

#endif  // LIB_CONNECTION_WS_WS_CONNECTION_MANAGER_H_
