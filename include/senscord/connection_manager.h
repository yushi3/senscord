/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_CONNECTION_MANAGER_H_
#define SENSCORD_CONNECTION_MANAGER_H_

#include "senscord/config.h"

#ifdef SENSCORD_SERVER

#include <string>
#include <map>

#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/connection.h"

namespace senscord {

/**
 * @brief Connection manager (singleton).
 */
class ConnectionManager : private util::Noncopyable {
 public:
  /**
   * @brief Get the manager instance.
   * @return Manager instance.
   */
  static ConnectionManager* GetInstance();

  /**
   * @brief Initialize and read config file.
   * @return Status object.
   */
  Status Init();

  /**
   * @brief Create the new connection instance.
   * @param[in]  (key) Connection key.
   * @param[out] (connection) New connection instance.
   * @return Status object.
   */
  Status CreateConnection(const std::string& key, Connection** connection);

  /**
   * @brief Release the connection instance.
   * @param[in] (connection) connection instance.
   * @return Status object.
   */
  Status ReleaseConnection(Connection* connection);

  /**
   * @brief Get the connection arguments.
   * @param[in] (key) Connection key.
   * @param[in,out] (arguments) Connection arguments.
   * @return Status object.
   */
  Status GetArguments(
      const std::string& key,
      std::map<std::string, std::string>* arguments);

 private:
  /**
   * @brief Constructor.
   */
  ConnectionManager();

  /**
   * @brief Destructor.
   */
  ~ConnectionManager();

 private:
  struct Impl;
  Impl* pimpl_;
};

}  // namespace senscord

#endif  // SENSCORD_SERVER
#endif  // SENSCORD_CONNECTION_MANAGER_H_
