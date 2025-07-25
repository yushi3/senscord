/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_CLIENT_SRC_COMPONENT_CLIENT_H_
#define LIB_COMPONENT_CLIENT_SRC_COMPONENT_CLIENT_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

#include "senscord/osal.h"
#include "senscord/develop/component.h"
#include "senscord/develop/client_messenger.h"
#include "senscord/connection.h"
#include "senscord/connection_types.h"
#include "./client_messenger_manager.h"
#include "./allocate_manager.h"
#include "./port_frame_manager.h"

namespace client {

/**
 * @brief The component of client for server connection.
 */
class ClientComponent
    : public senscord::Component, public PortFrameEventListener {
 public:
  /**
   * @brief Elements for send frames and events to component port.
   */
  struct PortSendingElements {
    ClientComponent* component;
    std::string port_type;
    int32_t port_id;

    bool end_flg;
    senscord::osal::OSThread* thread;
    senscord::osal::OSMutex* mutex;
    senscord::osal::OSCond* cond;
    std::vector<senscord::Message*> messages;   /**< messages from the server */
  };

  /**
   * Initialize this component.
   */
  virtual senscord::Status InitComponent(
    senscord::Core* core,
    senscord::ComponentPortManager* port_manager,
    const senscord::ComponentArgument& args);

  /**
   * Exit this component.
   */
  virtual senscord::Status ExitComponent();

  /**
   * Open the port.
   */
  virtual senscord::Status OpenPort(
    const std::string& port_type,
    int32_t port_id,
    const senscord::ComponentPortArgument& args);

  /**
   * Close the port.
   */
  virtual senscord::Status ClosePort(const std::string& port_type,
                                     int32_t port_id);

  /**
   * Start the port.
   */
  virtual senscord::Status StartPort(const std::string& port_type,
                                     int32_t port_id);

  /**
   * Stop the cport.
   */
  virtual senscord::Status StopPort(const std::string& port_type,
                                    int32_t port_id);

  /**
   * @brief Monitor the port sending messages from server.
   * @param[in] (elements) The elements for port.
   */
  void MonitorMessages(PortSendingElements* elements);

  /**
   * @brief Push the message for port seinding.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @param[in] (msg) Message from server.
   * @return Status object.
   */
  senscord::Status PushPortSendingsMessage(
    const std::string& port_type, int32_t port_id, senscord::Message* msg);

  /**
   * Release the frame pushed from the port.
   */
  virtual senscord::Status ReleasePortFrame(
    const std::string& port_type,
    int32_t port_id,
    const senscord::FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids);

  /**
   * @brief Set the serialized property.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @param[in] (key) Key of property.
   * @param[in] (serialized_property) Serialized property address.
   * @param[in] (serialized_size) Serialized property size.
   * @return Status object.
   */
  senscord::Status SetProperty(
    const std::string& port_type,
    int32_t port_id,
    const std::string& key,
    const void* serialized_property,
    size_t serialized_size);

  /**
   * @brief Get and create new serialized property.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @param[in] (key) Key of property.
   * @param[in] (serialized_input_property) Input serialized property address.
   * @param[in] (serialized_input_size) Input serialized property size.
   * @param[out] (serialized_property) New serialized property address.
   * @param[out] (serialized_size) Serialized property size.
   * @return Status object.
   */
  senscord::Status GetProperty(
    const std::string& port_type,
    int32_t port_id,
    const std::string& key,
    const void* serialized_input_property,
    size_t serialized_input_size,
    void** serialized_property,
    size_t* serialized_size);

  /**
   * @brief Release the serialized property.
   * @param[in] (key) Key of property.
   * @param[in] (serialized_property) Serialized property address by Get().
   * @param[in] (serialized_size) Serialized property size by Get().
   * @return Status object.
   */
  senscord::Status ReleaseProperty(
    const std::string& key,
    void* serialized_property,
    size_t serialized_size);

  /**
   * @brief Lock the port properties.
   * @param[in] (port) The target port.
   * @param[in] (args) The arguments of callback.
   * @return Status object.
   */
  senscord::Status LockProperty(
      senscord::ComponentPort* port,
      const senscord::ComponentPort::LockPropertyArguments& args);

  /**
   * @brief Unlock the port properties.
   * @param[in] (port) The target port.
   * @param[in] (lock_resource) Lock resource.
   * @return Status object.
   */
  senscord::Status UnlockProperty(
      senscord::ComponentPort* port,
      senscord::PropertyLockResource* lock_resource);

  /**
   * @brief Release the all frames.
   *
   * This function is called when the following conditions:
   * - When stream stop is called when there is no frame being acquired.
   * - When all frames are released after stream stop.
   *
   * @param[in] (port_id) Port ID.
   */
  virtual void OnReleaseAllFrames(int32_t port_id);

  /**
   * @brief Constructor.
   */
  ClientComponent();

  /**
   * @brief Destructor.
   */
  ~ClientComponent();

 private:
  /**
   * @brief The threading mode of connection to server.
   */
  enum ConnectionThreading {
    kThreadingParallel = 0,  // default
    kThreadingSerial,
  };

  // typedefs
  typedef std::vector<std::string> PropertyKeyList;
  typedef std::map<senscord::ComponentPort*, PropertyKeyList*>
      PortPropertyKeyMap;

  /**
   * @brief Reference counter for each event.
   */
  struct PortEvent {
    std::string event_type; /**< event type */
    uint32_t referenced;    /**< reference count */
  };

  /**
   * @brief Create the frame info for SendFrame.
   * @param[out] (dest) The destination of FrameInfo.
   * @param[in] (src) The frame message from the server.
   * @return Status object.
   */
  senscord::Status CreateFrameInfo(
    int32_t port_id,
    senscord::FrameInfo* dest,
    const senscord::MessageDataFrameLocalMemory& src);

  /**
   * @brief Whether to reply to SendFrame message.
   * @param[in] (frame) The frame message from the server.
   * @return The need to reply.
   */
  bool IsReplyToSendFrame(
      const senscord::MessageDataFrameLocalMemory& frame) const;

  /**
   * @brief Return whether the FrameProperty has been updated.
   * @param[in] (src) The frame message from the server.
   * @return whether the FrameProperty has been updated.
   */
  bool IsUpdatedFrameProperty(
      const senscord::MessageDataFrameLocalMemory& src) const;

  /**
   * @brief Update properties by the frame from the server.
   * @param[in] (port) The component port.
   * @param[in] (src) The frame message from the server.
   * @return Status object.
   */
  senscord::Status UpdateFrameProperties(
    senscord::ComponentPort* port,
    const senscord::MessageDataFrameLocalMemory& src);

  /**
   * @brief Analyze the component arguments for port numbers.
   * @param[in] (args) The arguments of the component.
   * @return Status object.
   */
  senscord::Status AnalyzePortNum(const senscord::ComponentArgument& args);

  /**
   * @brief Analyze the component arguments for threading mode.
   * @param[in] (args) The arguments of the component.
   * @return Status object.
   */
  senscord::Status AnalyzeThreading(const senscord::ComponentArgument& args);

  /**
   * @brief Start-up to send to the component port with server messages.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @return Status object.
   */
  senscord::Status StartPortSendings(
    const std::string& port_type, int32_t port_id);

  /**
   * @brief End to send to the component port with server messages.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   */
  void StopPortSendings(const std::string& port_type, int32_t port_id);

  /**
   * @brief The processing for arrived multiple frames.
   * @param[in] (messenger) The messenger for sending reply.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @param[in] (msg) The message of arrived frame.
   */
  void ArrivedFrame(
    senscord::ClientMessenger* messenger,
    const std::string& port_type, int32_t port_id,
    const senscord::Message& msg);

  /**
   * @brief The processing for arrived event.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @param[in] (msg) The message of arrived event.
   */
  void ArrivedEvent(
    const std::string& port_type, int32_t port_id,
    const senscord::Message& msg);

  /**
   * @brief Release frames that failed to be sent.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @param[in] (frames) List of frame information to send.
   * @param[in] (dropped_frames) List of sequence numbers of dropped frames.
   */
  void ReleaseFrames(
      const std::string& port_type, int32_t port_id,
      const std::vector<senscord::FrameInfo>& frames,
      const std::vector<uint64_t>& dropped_frames);

  /**
   * @brief Get property list to server.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @param[out] (property_list) Property list obtained from server.
   * @return Status object.
   */
  senscord::Status GetPropertyList(
      const std::string& port_type,
      int32_t port_id,
      std::vector<std::string>* property_list);

  /**
   * @brief Register the properties to created component port.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @param[in] (key_list) The list of supported property keys.
   * @return Status object.
   */
  senscord::Status RegisterProperties(
    const std::string& port_type,
    int32_t port_id,
    const PropertyKeyList& key_list);

  /**
   * @brief Register the properties to component port.
   * @param[in] (port) The component port.
   * @param[in] (key_list) The list of supported property keys.
   * @param[out] (dst_key_list) The list of registerd property keys.
   * @return Status object.
   */
  senscord::Status RegisterPortProperties(
      senscord::ComponentPort* port,
      const PropertyKeyList& key_list,
      PropertyKeyList* dst_key_list);

  /**
   * @brief Unregister the properties from component port.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @return Status object.
   */
  senscord::Status UnregisterProperties(
    const std::string& port_type, int32_t port_id);

  /**
   * @brief Unregister the properties from component port.
   * @param[in] (port) The component port.
   * @param[in] (key_list) The list of supported property keys.
   */
  void UnregisterPortProperties(
    senscord::ComponentPort* port, const PropertyKeyList* key_list);

#ifdef SENSCORD_PLAYER
  /**
   * @brief Reload the properties from to component port.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @return Status object.
   */
  senscord::Status ReloadProperties(
      const std::string& port_type,
      int32_t port_id,
      const std::string& caller_property_key);
#endif  // SENSCORD_PLAYER

  /**
   * @brief Get the port address created.
   * @param[in] (type) The type of port.
   * @param[in] (id) The ID of port.
   * @return Component port.
   */
  senscord::ComponentPort* GetPort(const std::string& type, int32_t id) const;

  /**
   * @brief Create the manager of the messenger.
   * @return Status object.
   */
  senscord::Status CreateMessengerManager();

  /**
   * @brief Create the manager on port.
   * @param[in] (port_id) The port id.
   * @param[out] (messenger) The created messenger.
   * @return Status object.
   */
  senscord::Status CreateMessenger(
    int32_t port_id, senscord::ClientMessenger** messenger);

  /**
   * @brief Get the string of the current threading mode.
   * @return the string of the current threading mode.
   */
  const char* GetThreadingString() const;

  /**
   * @brief Register event callback.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @param[in] (serialized_property) Serialized property address.
   * @param[in] (serialized_size) Serialized property size.
   * @return Status object.
   */
  senscord::Status RegisterEvent(
      const std::string& port_type,
      int32_t port_id,
      const void* serialized_property,
      size_t serialized_size);

  /**
   * @brief Unregister event callback.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @param[in] (serialized_property) Serialized property address.
   * @param[in] (serialized_size) Serialized property size.
   * @return Status object.
   */
  senscord::Status UnregisterEvent(
      const std::string& port_type,
      int32_t port_id,
      const void* serialized_property,
      size_t serialized_size);

  /**
   * @brief Get port event element.
   * @param[in] (port_id) The ID of port type.
   * @param[in] (event_type) The type of event.
   * @param[out] (port_event) The port event element.
   * @return Status object.
   */
  senscord::Status GetPortEventElement(
      int32_t port_id,
      const std::string& event_type,
      PortEvent** port_event);

  /**
   * @brief Delete port events.
   * @param[in] (port_id) The ID of port type.
   */
  void DeletePortEvents(int32_t port_id);

  /**
   * @brief Remove resource of lock property.
   * @param[in] (port_id) The ID of port.
   */
  void RemovePortLockResources(int32_t port_id);

  /**
   * @brief Send the frame dropped event.
   * @param[in] (port) The component port.
   * @param[in] (sequence_number) The sequence number that was dropped.
   */
  void SendEventFrameDropped(
      senscord::ComponentPort* port, uint64_t sequence_number);

  /**
   * Release the frame pushed from the port.
   * @param[in] (port_type) The type of port.
   * @param[in] (port_id) The ID of port type.
   * @param[in] (frameinfo) The frame information.
   * @param[in] (referenced_channel_ids) The list of referenced channel IDs.
   * @param[in] (required_release_to_server) The flag to release to server.
   */
  senscord::Status ReleasePortFrameCore(
    const std::string& port_type,
    int32_t port_id,
    const senscord::FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids,
    bool required_release_to_server);

  // elements
  uint32_t port_num_;
  std::string instance_name_;
  std::string address_primary_;    // primary address
  std::string address_secondary_;  // secondary address
  ConnectionThreading threading_;
  std::string connection_mode_;
  uint64_t reply_timeout_nsec_;

  // managers
  senscord::ComponentPortManager* port_manager_;
  std::vector<senscord::MemoryAllocator*> allocators_;

  // supported properties
  PortPropertyKeyMap port_property_key_map_;
  senscord::osal::OSMutex* mutex_port_property_key_map_;

  // messaging management
  ClientMessengerManager* msg_manager_;

  // mapping allocator
  AllocateManager alloc_manager_;

  // messages for sending to port
  typedef std::map<int32_t, PortSendingElements*> PortSendingElemMap;
  PortSendingElemMap port_sendings_;
  senscord::osal::OSMutex* mutex_port_sendings_;

  // Frame management for each port
  PortFrameManager* frame_manager_;

  // register event management
  typedef std::vector<PortEvent*> PortEvents;
  std::map<int32_t, PortEvents> port_event_map_;
  senscord::osal::OSMutex* mutex_port_event_map_;

  /**
   * @brief Property lock resource of server.
   */
  struct ServerPropertyLockResource {
    uint64_t resource_id;
  };
  // Resource of lock property
  typedef std::map<senscord::PropertyLockResource*, ServerPropertyLockResource>
      PortLockResources;
  std::map<int32_t, PortLockResources> port_lock_resources_;
  senscord::osal::OSMutex* mutex_port_lock_resources_;
};

}   // namespace client
#endif  // LIB_COMPONENT_CLIENT_SRC_COMPONENT_CLIENT_H_
