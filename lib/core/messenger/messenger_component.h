/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_MESSENGER_MESSENGER_COMPONENT_H_
#define LIB_CORE_MESSENGER_MESSENGER_COMPONENT_H_

#include <string>
#include <vector>

#include "senscord/develop/component.h"
#include "messenger/messenger_topic.h"
#include "messenger/inner_frame_sender.h"

namespace senscord {

/**
 * @brief The messenger component.
 */
class MessengerComponent : public Component {
 public:
  /**
   * @brief Initialize this component, called at once.
   * @param[in] (core) Core instance.
   * @param[in] (port_manager) Port manager for this component.
   * @param[in] (args) Arguments of component starting.
   * @return Status object.
   */
  virtual Status InitComponent(
    Core* core,
    ComponentPortManager* port_manager,
    const ComponentArgument& args);

  /**
   * @brief Exit this component, called at all ports closed.
   * @return Status object.
   */
  virtual Status ExitComponent();

  /**
   * @brief Open the port.
   * @param[in] (port_type) Port type to open.
   * @param[in] (port_id) Port ID to open.
   * @param[in] (args) Arguments of port starting.
   * @return Status object.
   */
  virtual Status OpenPort(
    const std::string& port_type,
    int32_t port_id,
    const ComponentPortArgument& args);

  /**
   * @brief Close the port.
   * @param[in] (port_type) Port type to close.
   * @param[in] (port_id) Port ID to close.
   * @return Status object.
   */
  virtual Status ClosePort(const std::string& port_type, int32_t port_id);

  /**
   * @brief Start the port.
   * @param[in] (port_type) Port type to start.
   * @param[in] (port_id) Port ID to start.
   * @return Status object.
   */
  virtual Status StartPort(const std::string& port_type, int32_t port_id);

  /**
   * @brief Stop the port.
   * @param[in] (port_type) Port type to stop.
   * @param[in] (port_id) Port ID to stop.
   * @return Status object.
   */
  virtual Status StopPort(const std::string& port_type, int32_t port_id);

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
    const std::vector<uint32_t>* referenced_channel_ids);

  /**
   * @brief Constructor
   */
  MessengerComponent();

  /**
   * @brief Destructor
   */
  ~MessengerComponent();

 private:
  ComponentArgument args_;
};

}   // namespace senscord

#endif    // LIB_CORE_MESSENGER_MESSENGER_COMPONENT_H_
