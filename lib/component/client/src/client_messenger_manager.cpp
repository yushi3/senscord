/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "client_messenger_manager.h"
#include "senscord/osal.h"

namespace client {

/**
 * @brief Create the new messenger.
 * @param[in] (port_id) The port id of creating messenger.
 * @return The created messenger.
 */
senscord::ClientMessenger* ClientMessengerManagerSerial::CreateMessenger(
    int32_t port_id) {
  if (messenger_ == NULL) {
    messenger_ = new senscord::ClientMessenger;
  }
  return messenger_;
}

/**
 * @brief Remove the messenger of port.
 * @param[in] (port_id) The port id.
 */
void ClientMessengerManagerSerial::RemoveMessenger(int32_t port_id) {
  // do nothing
  return;
}

/**
 * @brief Get the messenger of port.
 * @param[in] (port_id) The port id.
 * @return The created messenger. If no existed then return NULL.
 */
senscord::ClientMessenger* ClientMessengerManagerSerial::GetMessenger(
    int32_t port_id) const {
  return messenger_;
}

/**
 * @biref Constructor.
 */
ClientMessengerManagerSerial::ClientMessengerManagerSerial() : messenger_() {}

/**
 * @brief Destructor.
 */
ClientMessengerManagerSerial::~ClientMessengerManagerSerial() {
  if (messenger_ != NULL) {
    messenger_->Stop();
    delete messenger_;
    messenger_ = NULL;
  }
}


/**
 * @brief Create the new messenger.
 * @param[in] (port_id) The port id of creating messenger.
 * @return The created messenger.
 */
senscord::ClientMessenger* ClientMessengerManagerParallel::CreateMessenger(
    int32_t port_id) {
  senscord::ClientMessenger* messenger = NULL;
  senscord::osal::OSLockMutex(mutex_);
  MessengerList::iterator itr = messenger_list_.find(port_id);
  if (itr != messenger_list_.end()) {
    // already created.
    messenger = itr->second;
  } else {
    messenger = new senscord::ClientMessenger;
    messenger_list_[port_id] = messenger;
  }
  senscord::osal::OSUnlockMutex(mutex_);
  return messenger;
}

/**
 * @brief Remove the messenger of port.
 * @param[in] (port_id) The port id.
 */
void ClientMessengerManagerParallel::RemoveMessenger(int32_t port_id) {
  senscord::ClientMessenger* messenger = NULL;
  senscord::osal::OSLockMutex(mutex_);
  MessengerList::iterator itr = messenger_list_.find(port_id);
  if (itr != messenger_list_.end()) {
    messenger = itr->second;
    messenger_list_.erase(itr);
  }
  senscord::osal::OSUnlockMutex(mutex_);

  if (messenger != NULL) {
    messenger->Stop();
    delete messenger;
  }
}

/**
 * @brief Get the messenger of port.
 * @param[in] (port_id) The port id.
 * @return The created messenger. If no existed then return NULL.
 */
senscord::ClientMessenger* ClientMessengerManagerParallel::GetMessenger(
    int32_t port_id) const {
  senscord::ClientMessenger* messenger = NULL;
  senscord::osal::OSLockMutex(mutex_);
  MessengerList::const_iterator itr = messenger_list_.find(port_id);
  if (itr != messenger_list_.end()) {
    messenger = itr->second;
  }
  senscord::osal::OSUnlockMutex(mutex_);
  return messenger;
}

/**
 * @biref Constructor.
 */
ClientMessengerManagerParallel::ClientMessengerManagerParallel() {
  senscord::osal::OSCreateMutex(&mutex_);
}

/**
 * @brief Destructor.
 */
ClientMessengerManagerParallel::~ClientMessengerManagerParallel() {
  senscord::osal::OSDestroyMutex(mutex_);
}

}   // namespace client
