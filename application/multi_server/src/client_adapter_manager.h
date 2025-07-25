/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APPLICATION_MULTI_SERVER_CLIENT_ADAPTER_MANAGER_H_
#define APPLICATION_MULTI_SERVER_CLIENT_ADAPTER_MANAGER_H_

#include <vector>
#include "senscord/osal.h"
#include "client_adapter.h"

namespace senscord {
namespace server {

class ClientAdapterBase;

/**
 * @brief Class to monitor connected clients.
 */
class ClientAdapterManager : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   */
  ClientAdapterManager();

  /**
   * @brief Destructor.
   */
  ~ClientAdapterManager();

  /**
   * @brief Start monitoring the adapter.
   * @return Status object.
   */
  Status Start();

  /**
   * @brief Stop monitoring the adapter and release all.
   * @return Status object.
   */
  Status Stop();

  /**
   * @brief Register a client as a monitoring target.
   * @param[in] (client) Client to monitor.
   */
  void Register(ClientAdapterBase* client);

  /**
   * @brief Release the registered client.
   * @param[in] (client) Client to release.
   */
  void Release(ClientAdapterBase* client);

  /**
   * @brief Set the secondary client adapter.
   * @param[in] (stream_id) Identifier of server stream.
   * @param[in] (client) Secondary client adapter. (If NULL, reset)
   * @return Status object.
   */
  Status SetSecondaryAdapter(uint64_t stream_id, ClientAdapterBase* client);

 private:
  /**
   * @brief Working thread for monitoring.
   * @param[in] (arg) The instance of client adapter manager.
   * @return Always returns normal.
   */
  static osal::OSThreadResult ThreadProc(void* arg);

  /**
   * @brief Monitor the client.
   */
  void Monitor();

  /**
   * @brief Release All clients.
   */
  void ReleaseAllClients();

  /**
   * @brief Release clients from list.
   * @param[in,out] (list) List of clients to release.
   */
  void ReleaseClients(std::vector<ClientAdapterBase*>* list) const;

 private:
  osal::OSThread* thread_;
  bool end_flag_;
  osal::OSMutex* mutex_;
  osal::OSCond* cond_;
  std::vector<ClientAdapterBase*> clients_;
  std::vector<ClientAdapterBase*> release_list_;
};

}  // namespace server
}  // namespace senscord

#endif  // APPLICATION_MULTI_SERVER_CLIENT_ADAPTER_MANAGER_H_
