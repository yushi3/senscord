/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_SERVER_CLIENT_LISTENER_H_
#define LIB_SERVER_CLIENT_LISTENER_H_

#include <string>
#include "senscord/osal.h"
#include "senscord/senscord.h"
#include "senscord/connection.h"
#include "client_adapter_manager.h"
#include "client_adapter.h"
#include "config_manager.h"

namespace senscord {
namespace server {

/**
 * @brief The abstract listener class for the client connection.
 */
class ClientListenerBase : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   * @param[in] (manager) The client adapter manager.
   * @param[in] (connection) The connection interface.
   * @param[in] (address) The bind address.
   */
  explicit ClientListenerBase(
      ClientAdapterManager* manager,
      Connection* connection, const std::string& address);

  /**
   * @brief Destructor.
   */
  virtual ~ClientListenerBase();

  /**
   * @brief Start to listen the connection.
   * @return Status object.
   */
  Status Start();

  /**
   * @brief Stop to listen the connection.
   * @return Status object.
   */
  Status Stop();

 protected:
  /**
   * @brief Create client adapter from a new connection.
   * @param[in] (manager) The client adapter manager.
   * @param[in] (new_connection) new connection instance.
   * @return created client adapter.
   */
  virtual ClientAdapterBase* CreateAdapter(
      ClientAdapterManager* manager, Connection* new_connection) = 0;

 private:
  /**
   * @brief Working thread for listen.
   * @param[in] (arg) The instance of client listener.
   * @return Always returns normal.
   */
  static osal::OSThreadResult ThreadProc(void* arg);

  /**
   * @brief The method for listener threading.
   */
  void Listening();

 private:
  ClientAdapterManager* manager_;
  Connection* connection_;
  std::string address_;
  osal::OSThread* thread_;
  bool end_flag_;
};

/**
 * @brief The listener class for the client connection.
 */
class ClientListener : public ClientListenerBase {
 public:
  /**
   * @brief Constructor.
   * @param[in] (manager) The client adapter manager.
   * @param[in] (connection) The connection interface.
   * @param[in] (address) The bind address.
   * @param[in] (core) The SDK Core instance.
   * @param[in] (config_manager) The config manager.
   */
  explicit ClientListener(
      ClientAdapterManager* manager,
      Connection* connection, const std::string& address,
      Core* core, const ConfigManager& config_manager);

  /**
   * @brief Destructor.
   */
  ~ClientListener();

 protected:
  /**
   * @brief Create client adapter from a new connection.
   * @param[in] (manager) The client adapter manager.
   * @param[in] (new_connection) new connection instance.
   * @return created client adapter.
   */
  ClientAdapterBase* CreateAdapter(
      ClientAdapterManager* manager, Connection* new_connection);

 private:
  Core* core_;
  const ConfigManager& config_manager_;
};

}   // namespace server
}   // namespace senscord

#endif  // LIB_SERVER_CLIENT_LISTENER_H_
