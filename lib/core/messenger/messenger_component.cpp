/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "messenger/messenger_component.h"

#include <string>
#include <vector>

#include "messenger/messenger_manager.h"
#include "component/component_adapter.h"
#include "messenger/frame_sender.h"

namespace {
static const uint32_t kMessengerPortId = 0;
}

namespace senscord {

/**
 * @brief Initialize this component, called at once.
 * @param[in] (core) Core instance.
 * @param[in] (port_manager) Port manager for this component.
 * @param[in] (args) Arguments of component starting.
 * @return Status object.
 */
Status MessengerComponent::InitComponent(
    Core* core, ComponentPortManager* port_manager,
    const ComponentArgument& args) {
  args_ = args;
  // get frame sender
  MessengerManager* msg_manager = MessengerManager::GetInstance();
  FrameSender* frame_sender = NULL;
  Status status = msg_manager->GetFrameSender(
      args.instance_name, &frame_sender);
  InnerFrameSender* sender = static_cast<InnerFrameSender*>(frame_sender);
  SENSCORD_STATUS_TRACE(status);

  // create port (specify history book in topic)
  if (status.ok()) {
    ComponentAdapter* adapter = static_cast<ComponentAdapter*>(port_manager);
    PropertyHistoryBook* history_book = sender->GetPropertyHistoryBook();
    ComponentPort* port = NULL;
    status = adapter->CreatePort(
        kAnyPortType, kMessengerPortId, &port, history_book);
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      sender->SetPort(static_cast<ComponentPortCore*>(port));
    }
  }
  if (!status.ok()) {
    ExitComponent();
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Exit this component, called at all ports closed.
 * @return Status object.
 */
Status MessengerComponent::ExitComponent() {
  MessengerManager* msg_manager = MessengerManager::GetInstance();
  FrameSender* frame_sender = NULL;
  Status status = msg_manager->GetFrameSender(
      args_.instance_name, &frame_sender);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    InnerFrameSender* sender = static_cast<InnerFrameSender*>(frame_sender);
    sender->SetPort(NULL);
    msg_manager->ReleaseFrameSender(sender);
  }
  return status;
}

/**
 * @brief Open the port.
 * @param[in] (port_type) Port type to open.
 * @param[in] (port_id) Port ID to open.
 * @param[in] (args) Arguments of port starting.
 * @return Status object.
 */
Status MessengerComponent::OpenPort(
    const std::string& port_type, int32_t port_id,
    const ComponentPortArgument& args) {
  return Status::OK();
}

/**
 * @brief Close the port.
 * @param[in] (port_type) Port type to close.
 * @param[in] (port_id) Port ID to close.
 * @return Status object.
 */
Status MessengerComponent::ClosePort(
    const std::string& port_type, int32_t port_id) {
  return Status::OK();
}

/**
 * @brief Start the port.
 * @param[in] (port_type) Port type to start.
 * @param[in] (port_id) Port ID to start.
 * @return Status object.
 */
Status MessengerComponent::StartPort(
    const std::string& port_type, int32_t port_id) {
  return Status::OK();
}

/**
 * @brief Stop the port.
 * @param[in] (port_type) Port type to stop.
 * @param[in] (port_id) Port ID to stop.
 * @return Status object.
 */
Status MessengerComponent::StopPort(
    const std::string& port_type, int32_t port_id) {
  return Status::OK();
}

/**
 * @brief Release the frame pushed from the port.
 * @param[in] (port_type) Port type to release frame.
 * @param[in] (port_id) Port ID to release frame.
 * @param[in] (frameinfo) Informations to release frame.
 * @param[in] (referenced_channel_ids) List of referenced channel IDs.
 *                                     (NULL is the same as empty)
 * @return Status object.
 */
Status MessengerComponent::ReleasePortFrame(
    const std::string& port_type, int32_t port_id,
    const FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids) {
  MessengerManager* msg_manager = MessengerManager::GetInstance();
  FrameSender* frame_sender = NULL;
  Status status = msg_manager->GetFrameSender(
      args_.instance_name, &frame_sender);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    status = frame_sender->ReleaseFrame(frameinfo);
    SENSCORD_STATUS_TRACE(status);
  }
  return status;
}

/**
 * @brief Constructor
 */
MessengerComponent::MessengerComponent() : args_() {}

/**
 * @brief Destructor
 */
MessengerComponent::~MessengerComponent() {
  ExitComponent();
}

}   // namespace senscord
