/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_SERVER_SECONDARY_CLIENT_ADAPTER_H_
#define LIB_SERVER_SECONDARY_CLIENT_ADAPTER_H_

#include "client_adapter.h"
#include "client_listener.h"

namespace senscord {
namespace server {

/**
 * @brief The secondary adapter class for the client connection.
 */
class SecondaryClientAdapter : public ClientAdapterBase {
 public:
  /**
   * @brief Constructor.
   * @param[in] (manager) The client adapter manager.
   * @param[in] (connection) The connection interface.
   */
  explicit SecondaryClientAdapter(
      ClientAdapterManager* manager, Connection* connection);

  /**
   * @brief Destructor.
   */
  ~SecondaryClientAdapter();

 protected:
  /**
   * @brief The subroutine to receive new message.
   * @return Status object.
   */
  Status RecvMessage();

  /**
   * @brief This function is called when monitoring is finished.
   */
  void OnMonitoringFinished();

 private:
  /**
   * @brief Connect to the primary adapter.
   * @param[in] (msg) message from client.
   * @return Status object.
   */
  Status ConnectToPrimaryAdapter(const Message& msg);

 private:
  uint64_t saved_stream_id_;
};

}  // namespace server
}  // namespace senscord

#endif  // LIB_SERVER_SECONDARY_CLIENT_ADAPTER_H_
