/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "searcher/ucom_module.h"

#include <stdio.h>
#include <inttypes.h>
#include <algorithm>
#include "logger/logger.h"
#include "util/senscord_utils.h"
#include "senscord/connection_manager.h"

namespace senscord {
static const char *kAttributePort = "port";
static const char *kAttributePortDefaultValue = "65000";

/**
 * @brief Constructor.
 */
UcomModule::UcomModule()
      : port_(kAttributePortDefaultValue) {}

/**
 * @brief Destructor.
 */
UcomModule::~UcomModule() {}

/**
 * @brief Init client.
 * @param[in] (arguments) arguments map.
 * @return Status object.
 */
Status UcomModule::Init(const std::map<std::string, std::string> &arguments) {
  std::map<std::string, std::string>::const_iterator itr_port =
      arguments.find(kAttributePort);
  if (itr_port != arguments.end()) {
    port_ = itr_port->second;
  }
  SENSCORD_LOG_DEBUG("port_ %s", port_.c_str());

  return Status::OK();
}

/**
 * @brief Search servers.
 * @return vector<DeviceAddress>.
 */
const std::vector<senscord::DeviceAddress> UcomModule::Search() {
  std::vector<senscord::DeviceAddress> ret;
  std::vector<std::string> serial_number_list;
  Connection *connection;
  ConnectionManager *connection_manager = ConnectionManager::GetInstance();

  Status status = connection_manager->CreateConnection("ucom", &connection);
  if (!status.ok()) {
    connection = NULL;
    SENSCORD_LOG_ERROR("ConnectionManager CreateConnection failed");
    return ret;
  }
  status = connection->Search(&serial_number_list);
  if (!status.ok()) {
    SENSCORD_LOG_ERROR("ConnectionManager Search failed");
    return ret;
  }

  connection_manager->ReleaseConnection(connection);
  connection = NULL;

  for (std::vector<std::string>::const_iterator itr = serial_number_list.begin();
       itr != serial_number_list.end(); ++itr) {
    senscord::DeviceAddress address;
    address.SetUcom();
    std::ostringstream oss;
    oss << itr->c_str() << ":" + port_;
    address.address = oss.str();
    ret.push_back(address);
  }

  return ret;
}

}  // namespace senscord
