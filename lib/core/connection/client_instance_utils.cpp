/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/develop/client_instance_utils.h"

#include <stdint.h>
#include <string>
#include <map>
#include <limits>

#include "senscord/osal.h"
#include "senscord/status.h"
#include "senscord/logger.h"

namespace {
const char kDefaultConnection[] = "tcp";
const char kArgumentNameConnection[] = "connection";
const char kArgumentNameAddress[] = "address";
const char kArgumentNameAddressPrimary[] = "addressPrimary";
const char kArgumentNameAddressSecondary[] = "addressSecondary";
const char kArgumentNameReplyTimeout[] = "reply_timeout_msec";

// reply timeout nanoseconds.
const uint64_t kNsecPerMsec = 1000000;
const uint64_t kMinimumTimeout = 1000ULL * kNsecPerMsec;  // 1,000 ms
}  // namespace

namespace senscord {

/**
 * @brief Get connection type.
 * @param[in] (arguments) Connection parameters.
 * @param[out] (type) Connection type.
 * @return Status object.
 */
Status ClientInstanceUtility::GetConnectionType(
    const std::map<std::string, std::string>& arguments,
    std::string* type) {
  std::map<std::string, std::string>::const_iterator itr =
      arguments.find(kArgumentNameConnection);
  if (itr == arguments.end()) {
    // use default
    *type = kDefaultConnection;
  } else {
    *type = itr->second;
  }
  return Status::OK();
}

/**
 * @brief Get connection address.
 * @param[in] (arguments) Connection parameters.
 * @param[out] (address) Server address.
 * @param[out] (address_secondary) Server secondary address.
 * @return Status object.
 */
Status ClientInstanceUtility::GetConnectionAddress(
    const std::map<std::string, std::string>& arguments,
    std::string* address, std::string* address_secondary) {
  // address
  std::map<std::string, std::string>::const_iterator itr =
      arguments.find(kArgumentNameAddressPrimary);
  std::map<std::string, std::string>::const_iterator itr2 =
      arguments.find(kArgumentNameAddress);
  if (itr != arguments.end()) {
    if (itr2 != arguments.end()) {
      return SENSCORD_STATUS_FAIL("client",
          Status::kCauseInvalidArgument,
          "Both %s and %s are defined.",
          kArgumentNameAddress, kArgumentNameAddressPrimary);
    }
  } else {
    itr = itr2;
  }
  if (itr != arguments.end()) {
    *address = itr->second;
  } else {
    address->clear();
  }

  // address secondary
  if (address_secondary) {
    itr = arguments.find(kArgumentNameAddressSecondary);
    if (itr != arguments.end()) {
      *address_secondary = itr->second;
    } else {
      address_secondary->clear();
    }
  }
  return Status::OK();
}

/**
 * @brief Get connection reply timeout.
 * @param[in] (arguments) Connection parameters.
 * @param[out] (reply_timeout_nsec) Reply timeout.
 */
void ClientInstanceUtility::GetConnectionReplyTimeout(
    const std::map<std::string, std::string>& arguments,
    uint64_t* reply_timeout_nsec) {
  std::map<std::string, std::string>::const_iterator itr =
      arguments.find(kArgumentNameReplyTimeout);
  if (itr != arguments.end()) {
    uint64_t timeout = 0;
    char* endptr = NULL;
    osal::OSStrtoull(itr->second.c_str(), &endptr, 0, &timeout);
    if (endptr == NULL || *endptr != '\0') {
      SENSCORD_LOG_WARNING(
          "%s=%s is invalid. use the default reply timeout.",
          kArgumentNameReplyTimeout, itr->second.c_str());
      return;
    }
    if (timeout == 0 ||
        timeout > (std::numeric_limits<uint64_t>::max() / kNsecPerMsec)) {
      *reply_timeout_nsec = 0;  // infinite
    } else {
      *reply_timeout_nsec = timeout * kNsecPerMsec;  // millisec -> nanosec
      if (*reply_timeout_nsec < kMinimumTimeout) {
        *reply_timeout_nsec = kMinimumTimeout;
      }
    }
  }
}

}   // namespace senscord
