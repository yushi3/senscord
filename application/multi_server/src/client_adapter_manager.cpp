/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "client_adapter_manager.h"
#include <algorithm>
#include "senscord/osal.h"
#include "server_log.h"
#include "internal_types.h"

namespace senscord {
namespace server {

/**
 * @brief Constructor.
 */
ClientAdapterManager::ClientAdapterManager()
    : thread_(), end_flag_(false), mutex_(), cond_() {
  osal::OSCreateMutex(&mutex_);
  osal::OSCreateCond(&cond_);
}

/**
 * @brief Destructor.
 */
ClientAdapterManager::~ClientAdapterManager() {
  osal::OSDestroyCond(cond_);
  cond_ = NULL;
  osal::OSDestroyMutex(mutex_);
  mutex_ = NULL;
}

/**
 * @brief Start monitoring the adapter.
 * @return Status object.
 */
Status ClientAdapterManager::Start() {
  if (thread_) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        senscord::Status::kCauseInvalidOperation, "already started");
  }

  end_flag_ = false;
  int32_t ret = osal::OSCreateThread(&thread_, ThreadProc, this, NULL);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        senscord::Status::kCauseAborted,
        "failed to create management thread: %" PRIx32, ret);
  }
  return Status::OK();
}

/**
 * @brief Stop monitoring the adapter and release all.
 * @return Status object.
 */
Status ClientAdapterManager::Stop() {
  if (thread_) {
    // stop thread
    osal::OSLockMutex(mutex_);
    end_flag_ = true;
    osal::OSSignalCond(cond_);
    osal::OSUnlockMutex(mutex_);

    osal::OSJoinThread(thread_, NULL);
    thread_ = NULL;
  }

  // release all clients
  ReleaseAllClients();

  return Status::OK();
}

/**
 * @brief Register a client as a monitoring target.
 * @param[in] (client) Client to monitor.
 */
void ClientAdapterManager::Register(ClientAdapterBase* client) {
  osal::OSLockMutex(mutex_);
  if (!end_flag_) {
    clients_.push_back(client);
  }
  osal::OSUnlockMutex(mutex_);
}

/**
 * @brief Release the registered client.
 * @param[in] (client) Client to release.
 */
void ClientAdapterManager::Release(ClientAdapterBase* client) {
  osal::OSLockMutex(mutex_);
  std::vector<ClientAdapterBase*>::iterator pos =
      std::find(clients_.begin(), clients_.end(), client);
  if (pos != clients_.end()) {
    clients_.erase(pos);
    release_list_.push_back(client);
    osal::OSSignalCond(cond_);
  }
  osal::OSUnlockMutex(mutex_);
}

/**
 * @brief Set the secondary client adapter.
 * @param[in] (stream_id) Identifier of server stream.
 * @param[in] (client) Secondary client adapter. (If NULL, reset)
 * @return Status object.
 */
Status ClientAdapterManager::SetSecondaryAdapter(
    uint64_t stream_id, ClientAdapterBase* client) {
  bool result = false;

  osal::OSLockMutex(mutex_);
  for (std::vector<ClientAdapterBase*>::iterator itr = clients_.begin(),
      end = clients_.end(); itr != end; ++itr) {
    ClientAdapterBase* adapter = *itr;
    if (adapter->SetSecondaryAdapter(stream_id, client)) {
      result = true;
      break;
    }
  }
  osal::OSUnlockMutex(mutex_);

  if (!result) {
    return SENSCORD_STATUS_FAIL(kStatusBlockServer,
        senscord::Status::kCauseNotFound,
        "stream not found : stream=%" PRIx64, stream_id);
  }

  return Status::OK();
}

/**
 * @brief Working thread for monitoring.
 * @param[in] (arg) The instance of client adapter manager.
 * @return Always returns normal.
 */
osal::OSThreadResult ClientAdapterManager::ThreadProc(void* arg) {
  if (arg) {
    ClientAdapterManager* manager =
        reinterpret_cast<ClientAdapterManager*>(arg);
    manager->Monitor();
  }
  return static_cast<osal::OSThreadResult>(0);
}

/**
 * @brief Monitor the client.
 */
void ClientAdapterManager::Monitor() {
  SENSCORD_SERVER_LOG_DEBUG("[server] start monitoring");

  osal::OSLockMutex(mutex_);
  while (!end_flag_) {
    while (!end_flag_ && release_list_.empty()) {
      osal::OSWaitCond(cond_, mutex_);
    }

    std::vector<ClientAdapterBase*> tmp_list;
    tmp_list.swap(release_list_);
    osal::OSUnlockMutex(mutex_);

    // release
    ReleaseClients(&tmp_list);

    osal::OSLockMutex(mutex_);
  }
  osal::OSUnlockMutex(mutex_);

  SENSCORD_SERVER_LOG_DEBUG("[server] stop monitoring");
}

/**
 * @brief Release All clients.
 */
void ClientAdapterManager::ReleaseAllClients() {
  osal::OSLockMutex(mutex_);
  while (!clients_.empty()) {
    std::vector<ClientAdapterBase*>::reverse_iterator itr =
        clients_.rbegin();
    ClientAdapterBase* adapter = *itr;
    clients_.erase((++itr).base());
    osal::OSUnlockMutex(mutex_);

    adapter->Stop();
    delete adapter;

    osal::OSLockMutex(mutex_);
  }

  std::vector<ClientAdapterBase*> tmp_list;
  tmp_list.swap(release_list_);
  osal::OSUnlockMutex(mutex_);
  ReleaseClients(&tmp_list);
}

/**
 * @brief Release clients from list.
 * @param[in,out] (list) List of clients to release.
 */
void ClientAdapterManager::ReleaseClients(
    std::vector<ClientAdapterBase*>* list) const {
  for (std::vector<ClientAdapterBase*>::iterator itr = list->begin(),
      end = list->end(); itr != end; ++itr) {
    ClientAdapterBase* adapter = (*itr);
    adapter->Stop();
    delete adapter;
  }
  list->clear();
}

}   // namespace server
}   // namespace senscord
