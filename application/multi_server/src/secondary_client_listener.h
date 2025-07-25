/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APPLICATION_MULTI_SERVER_SECONDARY_CLIENT_LISTENER_H_
#define APPLICATION_MULTI_SERVER_SECONDARY_CLIENT_LISTENER_H_

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
   * @param[in] (connection_key) The connection key.
   * @param[in] (address) The bind address.
   */
  explicit SecondaryClientListener(
      ClientAdapterManager* manager,
      const std::string& connection_key, const std::string& address);

  /**
   * @brief Destructor.
   */
  ~SecondaryClientListener();

 protected:
  /**
   * @brief Create client adapter from a new connection.
   * @param[in] (manager) The client adapter manager.
   * @param[in] (new_connection) new connection instance.
   * @param[in] (connection_key) The connection key.
   * @return created client adapter.
   */
  ClientAdapterBase* CreateAdapter(
      ClientAdapterManager* manager, Connection* new_connection,
      const std::string& connection_key);
};

}  // namespace server
}  // namespace senscord

#endif  // APPLICATION_MULTI_SERVER_SECONDARY_CLIENT_LISTENER_H_
