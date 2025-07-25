/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>

#include "linux/socket_info_manager.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"
#include "linux/osal_linuxerror.h"

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
SocketInfoManager::SocketInfoManager() : list_(), mutex_() {
  int result = pthread_mutex_init(&mutex_, NULL);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex init failed(0x%" PRIx32 ")", result);
  }
}

/**
 * @brief Destructor.
 */
SocketInfoManager::~SocketInfoManager() {
  pthread_mutex_lock(&mutex_);
  for (List::iterator itr = list_.begin(); itr != list_.end(); ++itr) {
    SocketInfo* info = itr->second;
    delete info;
  }
  list_.clear();
  pthread_mutex_unlock(&mutex_);
  int result = pthread_mutex_destroy(&mutex_);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex delete failed(0x%" PRIx32 ")", result);
  }
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
  int result = pthread_mutex_lock(&mutex_);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex lock failed(0x%" PRIx32 ")", result);
  }
  bool ret = list_.insert(List::value_type(socket, new_info)).second;
  result = pthread_mutex_unlock(&mutex_);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex unlock failed(0x%" PRIx32 ")", result);
  }
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
  int result = pthread_mutex_lock(&mutex_);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex lock failed(0x%" PRIx32 ")", result);
  }
  List::const_iterator itr = list_.find(socket);
  if (itr != list_.end()) {
    *itr->second = info;
  } else {
    cause = kErrorNotFound;
  }
  result = pthread_mutex_unlock(&mutex_);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex unlock failed(0x%" PRIx32 ")", result);
  }
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
  int result = pthread_mutex_lock(&mutex_);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex lock failed(0x%" PRIx32 ")", result);
  }
  List::const_iterator itr = list_.find(socket);
  if (itr != list_.end()) {
    *info = *itr->second;
  } else {
    cause = kErrorNotFound;
  }
  result = pthread_mutex_unlock(&mutex_);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex unlock failed(0x%" PRIx32 ")", result);
  }
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
  int result = pthread_mutex_lock(&mutex_);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex lock failed(0x%" PRIx32 ")", result);
  }
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
  result = pthread_mutex_unlock(&mutex_);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex unlock failed(0x%" PRIx32 ")", result);
  }
  return cause;
}

}  //  namespace osal
}  //  namespace senscord
