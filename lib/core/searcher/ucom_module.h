/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_SEARCHER_UCOM_MODULE_H_
#define LIB_CORE_SEARCHER_UCOM_MODULE_H_
#include <string.h>

#include <map>
#include <string>
#include <vector>

#include "senscord/connection.h"
#include "searcher/device_searcher.h"

namespace senscord {

class UcomModule : public DeviceSearcher {
 public:
  /**
   * @brief Constructor.
   */
  UcomModule();

  /**
   * @brief Destructor.
   */
  virtual ~UcomModule();

  /**
   * @brief Init Client.
   * @param[in] (arguments) arguments map.
   * @return Status object.
   */
  Status Init(const std::map<std::string, std::string> &arguments);

  /**
   * @brief Search servers.
   * @return vector<DeviceAddress>.
   */
  const std::vector<senscord::DeviceAddress> Search();

 private:
  std::string port_;
};

}  // namespace senscord

#endif  // LIB_CORE_SEARCHER_UCOM_MODULE_H_
