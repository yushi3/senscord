/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
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
 * @param[in] (connection_key) The connection key.
 * @param[in] (address) The bind address.
 */
SecondaryClientListener::SecondaryClientListener(
    ClientAdapterManager* manager,
    const std::string& connection_key, const std::string& address)
    : ClientListenerBase(manager, connection_key, address) {}

/**
 * @brief Destructor.
 */
SecondaryClientListener::~SecondaryClientListener() {}

/**
 * @brief Create client adapter from a new connection.
 * @param[in] (manager) The client adapter manager.
 * @param[in] (new_connection) new connection instance.
 * @param[in] (connection_key) The connection key.
 * @return created client adapter.
 */
ClientAdapterBase* SecondaryClientListener::CreateAdapter(
    ClientAdapterManager* manager, Connection* new_connection,
    const std::string& connection_key) {
  return new SecondaryClientAdapter(manager, new_connection);
}

}   // namespace server
}   // namespace senscord
