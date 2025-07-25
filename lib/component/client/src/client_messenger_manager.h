/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_CLIENT_SRC_CLIENT_MESSENGER_MANAGER_H_
#define LIB_COMPONENT_CLIENT_SRC_CLIENT_MESSENGER_MANAGER_H_

#include <stdint.h>
#include <map>
#include "senscord/osal.h"
#include "senscord/noncopyable.h"
#include "senscord/develop/client_messenger.h"

namespace client {

/**
 * @brief The interface class for managing messengers.
 */
class ClientMessengerManager : private senscord::util::Noncopyable {
 public:
  /**
   * @brief Create the new messenger.
   * @param[in] (port_id) The port id of creating messenger.
   * @return The created messenger.
   */
  virtual senscord::ClientMessenger* CreateMessenger(int32_t port_id) = 0;

  /**
   * @brief Remove the messenger of port.
   * @param[in] (port_id) The port id.
   */
  virtual void RemoveMessenger(int32_t port_id) = 0;

  /**
   * @brief Get the messenger of port.
   * @param[in] (port_id) The port id.
   * @return The created messenger. If no existed then return NULL.
   */
  virtual senscord::ClientMessenger* GetMessenger(int32_t port_id) const = 0;

  /**
   * @brief Destructor.
   */
  virtual ~ClientMessengerManager() {}
};

/**
 * @brief The manager with serial on ports.
 */
class ClientMessengerManagerSerial : public ClientMessengerManager {
 public:
  /**
   * @brief Create the new messenger.
   * @param[in] (port_id) The port id of creating messenger.
   * @return The created messenger.
   */
  virtual senscord::ClientMessenger* CreateMessenger(int32_t port_id);

  /**
   * @brief Remove the messenger of port.
   * @param[in] (port_id) The port id.
   */
  virtual void RemoveMessenger(int32_t port_id);

  /**
   * @brief Get the messenger of port.
   * @param[in] (port_id) The port id.
   * @return The created messenger. If no existed then return NULL.
   */
  virtual senscord::ClientMessenger* GetMessenger(int32_t port_id) const;

  /**
   * @biref Constructor.
   */
  ClientMessengerManagerSerial();

  /**
   * @brief Destructor.
   */
  virtual ~ClientMessengerManagerSerial();

 private:
  senscord::ClientMessenger* messenger_;
};

/**
 * @brief The manager with parallel on ports.
 */
class ClientMessengerManagerParallel : public ClientMessengerManager {
 public:
  /**
   * @brief Create the new messenger.
   * @param[in] (port_id) The port id of creating messenger.
   * @return The created messenger.
   */
  virtual senscord::ClientMessenger* CreateMessenger(int32_t port_id);

  /**
   * @brief Remove the messenger of port.
   * @param[in] (port_id) The port id.
   */
  virtual void RemoveMessenger(int32_t port_id);

  /**
   * @brief Get the messenger of port.
   * @param[in] (port_id) The port id.
   * @return The created messenger. If no existed then return NULL.
   */
  virtual senscord::ClientMessenger* GetMessenger(int32_t port_id) const;

  /**
   * @biref Constructor.
   */
  ClientMessengerManagerParallel();

  /**
   * @brief Destructor.
   */
  virtual ~ClientMessengerManagerParallel();

 private:
  typedef std::map<int32_t, senscord::ClientMessenger*> MessengerList;
  MessengerList messenger_list_;
  mutable senscord::osal::OSMutex* mutex_;
};

}   // namespace client
#endif  // LIB_COMPONENT_CLIENT_SRC_CLIENT_MESSENGER_MANAGER_H_
