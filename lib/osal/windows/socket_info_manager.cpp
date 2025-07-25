/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "windows/socket_info_manager.h"

namespace senscord {
namespace osal {

// first get singleton instance for thread safe.
static SocketInfoManager* dummy = SocketInfoManager::GetInstance();

/**
 * @brief Get singleton instance.
 */
SocketInfoManager* SocketInfoManager::GetInstance() {
  static SocketInfoManager instance;
  return &instance;
}

/**
 * @brief Constructor.
 */
SocketInfoManager::SocketInfoManager() : list_(), critical_() {
  InitializeCriticalSection(&critical_);
}

/**
 * @brief Destructor.
 */
SocketInfoManager::~SocketInfoManager() {
  EnterCriticalSection(&critical_);
  for (List::iterator itr = list_.begin(); itr != list_.end(); ++itr) {
    SocketInfo* info = itr->second;
    delete info;
  }
  list_.clear();
  LeaveCriticalSection(&critical_);
  DeleteCriticalSection(&critical_);
}

/**
 * @brief Insert socket information.
 * @param[in] socket  Socket object.
 * @param[in] info    Socket information.
 * @return OSAL error cause. On success, it returns kErrorNone.
 */
OSErrorCause SocketInfoManager::Insert(OSSocket* socket,
                                       const SocketInfo& info) {
  SocketInfo* new_info = new SocketInfo(info);
  EnterCriticalSection(&critical_);
  bool ret = list_.insert(List::value_type(socket, new_info)).second;
  LeaveCriticalSection(&critical_);
  if (!ret) {
    delete new_info;
    return kErrorAlreadyExists;
  }
  return kErrorNone;
}

/**
 * @brief Set socket information.
 * @param[in] socket  Socket object.
 * @param[in] info    Socket information.
 * @return OSAL error cause. On success, it returns kErrorNone.
 */
OSErrorCause SocketInfoManager::Set(OSSocket* socket, const SocketInfo& info) {
  OSErrorCause cause = kErrorNone;
  EnterCriticalSection(&critical_);
  List::const_iterator itr = list_.find(socket);
  if (itr != list_.end()) {
    *itr->second = info;
  } else {
    cause = kErrorNotFound;
  }
  LeaveCriticalSection(&critical_);
  return cause;
}

/**
 * @brief Get socket information.
 * @param[in]  socket  Socket object.
 * @param[out] info    Pointer to the variable that receives the socket
 *                     information.
 * @return OSAL error cause. On success, it returns kErrorNone.
 */
OSErrorCause SocketInfoManager::Get(OSSocket* socket, SocketInfo* info) const {
  if (info == NULL) {
    return kErrorInvalidArgument;
  }
  OSErrorCause cause = kErrorNone;
  EnterCriticalSection(&critical_);
  List::const_iterator itr = list_.find(socket);
  if (itr != list_.end()) {
    *info = *itr->second;
  } else {
    cause = kErrorNotFound;
  }
  LeaveCriticalSection(&critical_);
  return cause;
}

/**
 * @brief Delete socket information.
 * @param[in]  socket  Socket object.
 * @param[out] info    Pointer to the variable that receives the socket
 *                     information. (optional)
 * @return OSAL error cause. On success, it returns kErrorNone.
 */
OSErrorCause SocketInfoManager::Delete(OSSocket* socket, SocketInfo* info) {
  OSErrorCause cause = kErrorNone;
  EnterCriticalSection(&critical_);
  List::iterator itr = list_.find(socket);
  if (itr != list_.end()) {
    SocketInfo* temp_info = itr->second;
    if (info != NULL) {
      *info = *temp_info;
    }
    delete temp_info;
    list_.erase(itr);
  } else {
    cause = kErrorNotFound;
  }
  LeaveCriticalSection(&critical_);
  return cause;
}

}  //  namespace osal
}  //  namespace senscord
