/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_COMPONENT_H_
#define SENSCORD_DEVELOP_COMPONENT_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/senscord.h"
#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/develop/common_types.h"
#include "senscord/develop/component_port_manager.h"

namespace senscord {

/**
 * @brief Interface class for component implementation.
 */
class Component : private util::Noncopyable {
 public:
  /**
   * @brief Initialize this component, called at once.
   * @param (core) Core instance.
   * @param (port_manager) Port manager for this component.
   * @param (args) Arguments of component starting.
   * @return Status object.
   */
  virtual Status InitComponent(
      Core* core,
      ComponentPortManager* port_manager,
      const ComponentArgument& args) {
    // Please overwrite this function.
    return InitComponent(port_manager, args);
  }

  /**
   * @deprecated
   * @brief Initialize this component, called at once.
   * @param (port_manager) Port manager for this component.
   * @param (args) Arguments of component starting.
   * @return Status object.
   */
  virtual Status InitComponent(
      ComponentPortManager* port_manager,
      const ComponentArgument& args) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseNotSupported,
        "Component::InitComponent is not implemented.");
  }

  /**
   * @brief Exit this component, called at all ports closed.
   * @return Status object.
   */
  virtual Status ExitComponent() = 0;

  /**
   * @brief Open the port.
   * @param (port_type) Port type to open.
   * @param (port_id) Port ID to open.
   * @param (args) Arguments of port starting.
   * @return Status object.
   */
  virtual Status OpenPort(
    const std::string& port_type,
    int32_t port_id,
    const ComponentPortArgument& args) = 0;

  /**
   * @brief Close the port.
   * @param (port_type) Port type to close.
   * @param (port_id) Port ID to close.
   * @return Status object.
   */
  virtual Status ClosePort(const std::string& port_type, int32_t port_id) = 0;

  /**
   * @brief Start the port.
   * @param (port_type) Port type to start.
   * @param (port_id) Port ID to start.
   * @return Status object.
   */
  virtual Status StartPort(const std::string& port_type, int32_t port_id) = 0;

  /**
   * @brief Stop the port.
   * @param (port_type) Port type to stop.
   * @param (port_id) Port ID to stop.
   * @return Status object.
   */
  virtual Status StopPort(const std::string& port_type, int32_t port_id) = 0;

  /**
   * @brief Release the frame pushed from the port.
   * @param[in] (port_type) Port type to release frame.
   * @param[in] (port_id) Port ID to release frame.
   * @param[in] (frameinfo) Informations to release frame.
   * @param[in] (referenced_channel_ids) List of referenced channel IDs.
   *                                     (NULL is the same as empty)
   * @return Status object.
   */
  virtual Status ReleasePortFrame(
    const std::string& port_type,
    int32_t port_id,
    const FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids) = 0;

  /**
   * @brief Virtual destructor.
   */
  virtual ~Component() {}
};

}   // namespace senscord
#endif  // SENSCORD_DEVELOP_COMPONENT_H_
