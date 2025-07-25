/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_SEARCHER_DEVICE_SEARCHER_H_
#define LIB_CORE_SEARCHER_DEVICE_SEARCHER_H_
#include <map>
#include <string>
#include <vector>

#include "senscord/status.h"

namespace senscord {

/**
 * @brief Device address.
 */
class DeviceAddress {
 public:
  void SetTcp() { connection = "tcp"; }
  void SetUcom() { connection = "ucom"; }

  const std::map<std::string, std::string> GetMap() const {
    std::map<std::string, std::string> ret;
    ret["connection"] = connection;
    ret["address"] = address;
    ret["addressSecondary"] = addressSecondary;
    return ret;
  }
  std::string connection;
  std::string address;
  std::string addressSecondary;
};

/**
 * @brief Device search interface.
 */
class DeviceSearcher {
 public:
  /**
   * @brief Init client.
   * @param[in] (arguments) arguments map.
   * @return Status object.
   */
  virtual Status Init(const std::map<std::string, std::string>& arguments) = 0;

  /**
   * @brief Search servers.
   * @return vector<DeviceAddress>.
   */
  virtual const std::vector<senscord::DeviceAddress> Search() = 0;
};

}  // namespace senscord

#endif  // LIB_CORE_SEARCHER_DEVICE_SEARCHER_H_
