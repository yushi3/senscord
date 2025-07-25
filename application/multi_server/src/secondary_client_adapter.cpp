/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "secondary_client_adapter.h"
#include "senscord/osal.h"
#include "server_log.h"
#include "internal_types.h"

namespace senscord {
namespace server {

/**
 * @brief Constructor.
 * @param[in] (manager) The client adapter manager.
 * @param[in] (connection) The connection interface.
 */
SecondaryClientAdapter::SecondaryClientAdapter(
    ClientAdapterManager* manager, Connection* connection)
    : ClientAdapterBase(manager, connection), saved_stream_id_() {}

/**
 * @brief Destructor.
 */
SecondaryClientAdapter::~SecondaryClientAdapter() {}

/**
 * @brief The subroutine to receive new message.
 * @return Status object.
 */
Status SecondaryClientAdapter::RecvMessage() {
  Message* msg = new Message();
  Status status = connection_->Recv(msg);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    // incoming new message
    if (msg->header.type == kMessageTypeHandshake) {
      ReleaseMessage(msg);
      return SENSCORD_STATUS_TRACE(status);
    }
    if (msg->header.data_type == kMessageDataTypeDisconnect) {
      // end of connection
      SENSCORD_SERVER_LOG_DEBUG(
          "[server] receive the request of disconnection.");

      // send reply
      // after this, connection will be disconnected by client.
      MessageDataDisconnectReply reply_data = {};
      reply_data.status.Set(Status::OK());
      status = SendReply(*msg, NULL, reply_data);
      ReleaseMessage(msg);

      return SENSCORD_STATUS_TRACE(status);
    }

    if (msg->header.data_type == kMessageDataTypeSecondaryConnect) {
      status = ConnectToPrimaryAdapter(*msg);
      SENSCORD_STATUS_TRACE(status);
      ReleaseMessage(msg);
    } else {
      // unsupported message
      status = SENSCORD_STATUS_FAIL(kStatusBlockServer,
           Status::kCauseNotSupported,
           "unsupported message: type=%d, data_type=%d",
           msg->header.type, msg->header.data_type);

      Message reply = *msg;
      reply.header.type = kMessageTypeReply;
      reply.data = NULL;
      SendMessageToClient(reply);
      ReleaseMessage(msg);
    }

    if (!status.ok()) {
      SENSCORD_SERVER_LOG_ERROR("[server] failed to receive message: %s",
          status.ToString().c_str());
    }
  } else if (status.cause() == Status::kCauseCancelled) {
    // disconnected
    SENSCORD_SERVER_LOG_INFO("[server] disconnect");
    ReleaseMessage(msg);
  } else {
    SENSCORD_SERVER_LOG_ERROR("[server] failed to recv: %s",
        status.ToString().c_str());
    ReleaseMessage(msg);
  }
  return status;
}

/**
 * @brief Connect to the primary adapter.
 * @param[in] (msg) message from client.
 * @return Status object.
 */
Status SecondaryClientAdapter::ConnectToPrimaryAdapter(const Message& msg) {
  Status status;
  Stream* stream = NULL;
  do {
    if (saved_stream_id_ != 0) {
      status = SENSCORD_STATUS_FAIL(kStatusBlockServer,
          Status::kCauseInvalidOperation, "already connected.");
      break;
    }

    SENSCORD_SERVER_LOG_DEBUG(
        "[server] request to attach socket: stream=%" PRIx64,
        msg.header.server_stream_id);

    stream = reinterpret_cast<Stream*>(msg.header.server_stream_id);

    // attach
    status = manager_->SetSecondaryAdapter(msg.header.server_stream_id, this);
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      saved_stream_id_ = msg.header.server_stream_id;
    }
  } while (false);

  MessageDataSecondaryConnectReply reply_data = {};
  reply_data.status.Set(status);

  // send reply
  status = SendReply(msg, stream, reply_data);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief This function is called when monitoring is finished.
 */
void SecondaryClientAdapter::OnMonitoringFinished() {
  if (saved_stream_id_ != 0) {
    // detach
    manager_->SetSecondaryAdapter(saved_stream_id_, NULL);
    saved_stream_id_ = 0;
  }
}

}   // namespace server
}   // namespace senscord
