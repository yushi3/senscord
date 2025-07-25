/*
 * SPDX-FileCopyrightText: 2020-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_SEARCHER_SSDP_MODULE_H_
#define LIB_CORE_SEARCHER_SSDP_MODULE_H_
#include <map>
#include <string>
#include <vector>

#include "searcher/device_searcher.h"
#include "senscord/osal.h"

namespace senscord {

class lssdp_ctx;

class SsdpModule : public DeviceSearcher {
 public:
  /**
   * @brief Constructor.
   */
  SsdpModule();

  /**
   * @brief Destructor.
   */
  virtual ~SsdpModule();

  /**
   * @brief Init Client.
   * @param[in] (arguments) arguments map.
   * @return Status object.
   */
  Status Init(const std::map<std::string, std::string>& arguments);

  /**
   * @brief Search servers.
   * @return vector<DeviceAddress>.
   */
  const std::vector<senscord::DeviceAddress> Search();

  /**
   * @brief Get lssdp context for server.
   * @param[in] (port) port.
   * @param[in] (portSecondly) port.
   * @return Status object.
   */
  Status ServerInit(std::string port, std::string portSecondly);

  /**
   * @brief Start server.
   * @return Status object.
   */
  Status ServerStart();

  /**
   * @brief Stop Server.
   * @return Status object.
   */
  Status ServerStop();

  /**
   * @brief Add device address.
   * @param[in] (address) device address.
   */
  void AddDeviceAddress(DeviceAddress address);

  /**
   * @brief Server thread.
   */
  void ServerThread();

  /**
   * @brief Chech TCP Connection.
   * @return true tcp, false other.
   */
  static bool IsTcpConnection(std::string connection);

 private:
  /**
   * @brief Get lssdp context.
   * @param[in] (timeoutMilliseconds) wait time for client.
   * @param[in] (port) port for server.
   * @param[in] (portSecondly) secondly port for server.
   * @return lssdp_ctx.
   */
  lssdp_ctx* GetCtx(int timeoutMilliseconds, std::string port,
                   std::string portSecondly);

  /**
   * @brief Wait SSDP.
   * @param[in] (lssdp) lssdp context.
   */
  void Wait(lssdp_ctx* lssdp);

  lssdp_ctx* lssdp_;
  osal::OSThread* server_thread_;
  std::vector<DeviceAddress> address_list_;
};

}  // namespace senscord

#endif  // LIB_CORE_SEARCHER_SSDP_MODULE_H_
