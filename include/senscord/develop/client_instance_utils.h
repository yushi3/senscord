/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_CLIENT_INSTANCE_UTILS_H_
#define SENSCORD_DEVELOP_CLIENT_INSTANCE_UTILS_H_

#include "senscord/config.h"

#ifdef SENSCORD_SERVER

#include <string>
#include <map>
#include "senscord/status.h"

namespace senscord {

/**
 * @brief Utility class for recorder and player.
 */
class ClientInstanceUtility {
 public:
  /**
   * @brief Get connection type.
   * @param[in] (arguments) Connection parameters.
   * @param[out] (type) Connection type.
   * @return Status object.
   */
  static Status GetConnectionType(
      const std::map<std::string, std::string>& arguments,
      std::string* type);

  /**
   * @brief Get connection address.
   * @param[in] (arguments) Connection parameters.
   * @param[out] (address) Server address.
   * @param[out] (address_secondary) Server secondary address.
   * @return Status object.
   */
  static Status GetConnectionAddress(
      const std::map<std::string, std::string>& arguments,
      std::string* address,
      std::string* address_secondary);

  /**
   * @brief Get connection reply timeout.
   * @param[in] (arguments) Connection parameters.
   * @param[out] (reply_timeout_nsec) Reply timeout.
   */
  static void GetConnectionReplyTimeout(
      const std::map<std::string, std::string>& arguments,
      uint64_t* reply_timeout_nsec);

 private:
  /**
   * @brief Constructor.
   */
  ClientInstanceUtility();

  /**
   * @brief Destructor.
   */
  ~ClientInstanceUtility();
};

}   // namespace senscord

#endif  // SENSCORD_SERVER
#endif  // SENSCORD_DEVELOP_CLIENT_INSTANCE_UTILS_H_
