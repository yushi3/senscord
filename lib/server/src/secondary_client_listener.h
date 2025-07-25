/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_SERVER_SECONDARY_CLIENT_LISTENER_H_
#define LIB_SERVER_SECONDARY_CLIENT_LISTENER_H_

#include <string>
#include "client_listener.h"

namespace senscord {
namespace server {

/**
 * @brief The secondary listener class for the client connection.
 */
class SecondaryClientListener : public ClientListenerBase {
 public:
  /**
   * @brief Constructor.
   * @param[in] (manager) The client adapter manager.
   * @param[in] (connection) The connection interface.
   * @param[in] (address) The bind address.
   */
  explicit SecondaryClientListener(
      ClientAdapterManager* manager,
      Connection* connection, const std::string& address);

  /**
   * @brief Destructor.
   */
  ~SecondaryClientListener();

 protected:
  /**
   * @brief Create client adapter from a new connection.
   * @param[in] (manager) The client adapter manager.
   * @param[in] (new_connection) new connection instance.
   * @return created client adapter.
   */
  ClientAdapterBase* CreateAdapter(
      ClientAdapterManager* manager, Connection* new_connection);
};

}  // namespace server
}  // namespace senscord

#endif  // LIB_SERVER_SECONDARY_CLIENT_LISTENER_H_
