/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_CLIENT_MESSENGER_H_
#define SENSCORD_DEVELOP_CLIENT_MESSENGER_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "senscord/osal.h"
#include "senscord/status.h"
#include "senscord/serialize.h"
#include "senscord/connection.h"

namespace senscord {

namespace util {
class Mutex;
}

/**
 * @brief The messenger for the server application.
 */
class ClientMessenger : private util::Noncopyable {
 public:
  /**
   * @brief Message received callback type
   */
  typedef void (* OnMesageReceivedCallback)(
    const std::string& port_type, int32_t port_id,
    Message* msg, void* arg);

  /**
   * @brief Start the messaging.
   * @param[in] (connection_mode) The connection mode.
   * @param[in] (address_primary) Primary destination address.
   * @param[in] (address_secondary) Secondary destination address.
   * @return Status object.
   */
  Status Start(
      const std::string& connection_mode,
      const std::string& address_primary,
      const std::string& address_secondary);

  /**
   * @brief Stop the messaging.
   * @return Status object.
   */
  Status Stop();

  /**
 * @brief Make a secondary connection.
 * @param[in] (port_type) Client port type when opened.
 * @param[in] (port_id) Client port id when opened.
 * @param[in] (stream_id) Stream ID from server.
 * @param[in] (timeout_nsec) Timeout nanoseconds. (0: infinite)
 * @return Status object.
   */
  Status MakeSecondaryConnection(
      const std::string& port_type, int32_t port_id, uint64_t stream_id,
      uint64_t timeout_nsec);

  /**
   * @brief Register the callback for arrived frame.
   * @param[in] (callback) The callback function.
   * @param[in] (arg) The argument when callback called.
   */
  void RegisterFrameCallback(
    const OnMesageReceivedCallback callback, void* arg);

  /**
   * @brief Register the callback for arrived event.
   * @param[in] (callback) The callback function.
   * @param[in] (arg) The argument when callback called.
   */
  void RegisterEventCallback(
    const OnMesageReceivedCallback callback, void* arg);

  /**
   * @brief Register the callback for arrived request.
   * @param[in] (callback) The callback function.
   * @param[in] (arg) The argument when callback called.
   */
  void RegisterRequestCallback(
    const OnMesageReceivedCallback callback, void* arg);

  /**
   * @brief Create the new request message.
   * @param[out] (msg) The location of new message.
   * @param[in] (port_type) Client port type when opened.
   * @param[in] (port_id) Client port id when opened.
   * @param[in] (msg_data_type) The type of request message.
   * @param[in] (msg_data) The payload data of message.
   */
  template <typename T>
  void CreateRequestMessage(
      Message* msg, const std::string& port_type,
      int32_t port_id, MessageDataType msg_data_type,
      const T* msg_data) {
    if (msg) {
      msg->header.server_stream_id = GetServerStreamId(port_type, port_id);
      msg->header.request_id = GetRequestId();
      msg->header.type = kMessageTypeRequest;
      msg->header.data_type = msg_data_type;
      msg->data = const_cast<T*>(msg_data);
    }
  }

  /**
   * @brief Send the request message.
   * @param[in] (msg) The message of request.
   * @return Status object.
   */
  Status SendCommandRequest(const Message& msg);

  /**
   * @brief Wait the reply message of request.
   * @param[in] (request_id) Requested ID.
   * @param[in] (timeout_nsec) Timeout nanoseconds. (0: infinite)
   * @param[out] (msg) The reply message. When used, must be released.
   * @return Status object.
   */
  Status WaitCommandReply(
      uint64_t request_id, uint64_t timeout_nsec, Message** msg);

  /**
   * @brief Release the reply message.
   * @param[in] (msg) The reply message.
   */
  void ReleaseCommandReply(Message* msg);

  /**
   * @brief Send the reply message.
   * @param[in] (msg) The message of reply.
   * @return Status object.
   */
  Status SendCommandReply(const Message& msg);

  /**
   * @brief Send the send frame message.
   * @param[in] (msg) The message of send frame.
   * @return Status object.
   */
  Status SendCommandSendFrame(const Message& msg);

  /**
   * @brief The method of receiving thread.
   */
  void RecvThreadCore();

  /**
   * @brief Add the server stream id information.
   * @param[in] (port_type) Client port type when opened.
   * @param[in] (port_id) Client port id when opened.
   * @param[in] (server_stream_id) Stream ID from server.
   */
  void AddServerStreamId(
    const std::string& port_type, int32_t port_id, uint64_t server_stream_id);

  /**
   * @brief Delete the information about server stream id.
   * @param[in] (port_type) Client port type when opened.
   * @param[in] (port_id) Client port id when opened.
   */
  void DeleteServerStreamId(const std::string& port_type, int32_t port_id);

  /**
   * @brief Clear server stream id.
   */
  void ClearServerStreamId();

  /**
   * @brief Get the connection state.
   * @return The connection state.
   */
  bool IsConnected() const;

  Status GetChannelRawData(
      const Channel* channel, ChannelRawDataInfo* rawdata) {
    return SENSCORD_STATUS_TRACE(
        connection_->GetChannelRawData(channel, rawdata));
  }

  /**
   * @brief Lock for Client component.
   */
  void LockComponent();

  /**
   * @brief Unlock for Client component.
   */
  void UnlockComponent();

  /**
   * @brief Constructor.
   */
  ClientMessenger();

  /**
   * @brief Destructor.
   */
  ~ClientMessenger();

 private:
  /**
   * @brief The struct of server stream id information.
   */
  struct ServerStreamId {
    // client info
    std::string port_type;
    int32_t port_id;

    // server info
    uint64_t server_stream_id;
  };

  /**
   * @brief The struct for waiting reply.
   */
  struct WaitingCommand {
    uint64_t request_id;
    osal::OSCond* cond;
    Message* reply_msg;
  };

  /**
   * @brief Dealing the received message.
   * @param[in] (msg) The new incoming message.
   */
  void DealMessage(Message* msg);

  /**
   * @brief Send the request of disconnet to server.
   * @return Status object.
   */
  Status RequestDisconnection();

  /**
   * @brief Register to wait the reply message.
   * @param[in] (request_id) The requested ID.
   * @return Status object.
   */
  Status RegisterWaitCommand(uint64_t request_id);

  /**
   * @brief Unregister to wait the reply message.
   * @param[in] (request_id) The requested ID.
   */
  void UnregisterWaitCommand(uint64_t request_id);

  /**
   * @brief Get the waiting reply information.
   * @param[in] (request_id) The requested ID.
   * @return The waiting information. If no waiting then return NULL.
   */
  WaitingCommand* GetWaitCommand(uint64_t request_id);

  /**
   * @brief Get the server stream id.
   * @param[in] (port_type) Client port type when opened.
   * @param[in] (port_id) Client port id when opened.
   * @return The server stream id.
   *         If no registered then return kInvalidServerStreamId.
   */
  uint64_t GetServerStreamId(const std::string& port_type, int32_t port_id);

  /**
   * @brief Get the port id from the server stream id.
   * @param[in] (server_stream_id) The server stream id.
   * @param[out] (port_type) Client port type when opened.
   * @param[out] (port_id) Client port id when opened.
   * @return Status object.
   */
  Status GetPortID(
    uint64_t server_stream_id,
    std::string* port_type,
    int32_t* port_id);

  /**
   * @brief Get the next request ID.
   * @return The next ID.
   */
  uint64_t GetRequestId();

  /**
   * @brief Set the connection state.
   * @param[in] (status) The connection state.
   */
  void SetConnectStatus(bool status);

 private:
  // basic elements
  Connection* connection_;
  osal::OSThread* recv_thread_;
  volatile bool is_connected_;

  ClientMessenger* child_;

  // for waiting reply
  typedef std::vector<WaitingCommand*> WaitingCommandQueue;
  WaitingCommandQueue waiting_commands_;
  mutable util::Mutex* mutex_waiting_;

  // for request id
  uint64_t request_id_;
  osal::OSMutex* mutex_request_id_;

  // for server stream id
  typedef std::vector<ServerStreamId> ServerStreamIdList;
  ServerStreamIdList server_stream_ids_;
  osal::OSMutex* mutex_server_stream_id_;

  // callback
  OnMesageReceivedCallback frame_callback_;
  void* frame_callback_arg_;

  OnMesageReceivedCallback event_callback_;
  void* event_callback_arg_;

  OnMesageReceivedCallback request_callback_;
  void* request_callback_arg_;

  // for client component
  osal::OSMutex* mutex_component_;
};

}   // namespace senscord
#endif  // SENSCORD_DEVELOP_CLIENT_MESSENGER_H_
