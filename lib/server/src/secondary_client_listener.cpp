/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "secondary_client_listener.h"
#include <string>
#include "secondary_client_adapter.h"

namespace senscord {
namespace server {

/**
 * @brief Constructor.
 * @param[in] (manager) The client adapter manager.
 * @param[in] (connection) The connection interface.
 * @param[in] (address) The bind address.
 */
SecondaryClientListener::SecondaryClientListener(
    ClientAdapterManager* manager,
    Connection* connection, const std::string& address)
    : ClientListenerBase(manager, connection, address) {}

/**
 * @brief Destructor.
 */
SecondaryClientListener::~SecondaryClientListener() {}

/**
 * @brief Create client adapter from a new connection.
 * @param[in] (manager) The client adapter manager.
 * @param[in] (new_connection) new connection instance.
 * @return created client adapter.
 */
ClientAdapterBase* SecondaryClientListener::CreateAdapter(
    ClientAdapterManager* manager, Connection* new_connection) {
  return new SecondaryClientAdapter(manager, new_connection);
}

}   // namespace server
}   // namespace senscord
