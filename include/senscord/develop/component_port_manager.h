/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_COMPONENT_PORT_MANAGER_H_
#define SENSCORD_DEVELOP_COMPONENT_PORT_MANAGER_H_

#include <stdint.h>
#include <string>

#include "senscord/config.h"
#include "senscord/noncopyable.h"
#include "senscord/stream.h"
#include "senscord/status.h"
#include "senscord/develop/common_types.h"
#include "senscord/develop/component_port.h"

namespace senscord {

/**
 * @brief Interface class of the port manager on this component.
 */
class ComponentPortManager : private util::Noncopyable {
 public:
  /**
   * @brief Create and get the new port on this component.
   * @param (type) Port type to create.
   * @param (id) Port ID to create.
   * @param (port) Instance of the created port.
   * @return Status object. failed (ex. id is already exist).
   */
  virtual Status CreatePort(
    const std::string& type,
    int32_t id,
    ComponentPort** port) = 0;

  /**
   * @brief Delete the created port on this component.
   * @param (port) Instance of the created port.
   * @return Status object.
   */
  virtual Status DestroyPort(ComponentPort* port) = 0;

  /**
   * @brief Delete all created port on this component.
   * @return Status object.
   */
  virtual Status DestroyAllPort() = 0;

  /**
   * @brief Get the created port instance on this component.
   * @param (type) Type of the target port.
   * @param (id) ID of the target port.
   * @return non NULL or NULL.
   */
  virtual ComponentPort* GetPort(const std::string& type, int32_t id) = 0;

  /**
   * @brief Virtual destructor.
   */
  virtual ~ComponentPortManager() {}
};

}   // namespace senscord
#endif  // SENSCORD_DEVELOP_COMPONENT_PORT_MANAGER_H_
