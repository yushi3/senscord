/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CONNECTION_CONNECTION_DYNAMIC_LOADER_H_
#define LIB_CORE_CONNECTION_CONNECTION_DYNAMIC_LOADER_H_

#include <string>

#include "loader/class_dynamic_loader.h"
#include "senscord/connection.h"

namespace senscord {

/**
 * @brief Connection dynamic loader.
 */
class ConnectionDynamicLoader : public ClassDynamicLoader {
 public:
  /**
   * @brief Constructor.
   */
  ConnectionDynamicLoader();

  /**
   * @brief Destructor.
   */
  ~ConnectionDynamicLoader();

  /**
   * @brief Generate an instance based on the connection name of the argument.
   * @param[in]  (name) Name of connection library.
   * @param[out] (connection) Where to store the created Connection.
   * @return Status object.
   */
  Status Create(const std::string& name,
                Connection** connection);

  /**
   * @brief Delete the connection passed in the argument.
   * @param[in] (name) Name of connection library.
   * @param[in] (connection) Connection to delete.
   * @return Status object.
   */
  Status Destroy(const std::string& name,
                 Connection* connection);

 protected:
  /**
   * @brief A function that loads a library based on the argument name.
   * @param[in] (name) Key name of library.
   * @return Status object.
   */
  Status Load(const std::string& name);
};

}   // namespace senscord

#endif  //  LIB_CORE_CONNECTION_CONNECTION_DYNAMIC_LOADER_H_
