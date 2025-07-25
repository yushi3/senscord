/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core/version_fetcher.h"

#include <stdint.h>
#include <string>
#include <utility>      // std::make_pair

#include "core/version.h"
#include "core/internal_types.h"
#include "core/config_manager.h"
#include "component/component_manager.h"
#include "senscord/connection_manager.h"

namespace senscord {

// waiting time
static const uint64_t kRecvWaitTime = 3ULL * 1000 * 1000 * 1000;   // 3sec

/**
 * @brief The thread processing for receive messages.
 * @param[in] (arg) Version manager pointer.
 * @return Don't care.
 */
static senscord::osal::OSThreadResult RecvThread(void* arg) {
  if (arg) {
    VersionFetcher* fercher = reinterpret_cast<VersionFetcher*>(arg);
    fercher->ConnectionThreadCore();
  }
  return static_cast<senscord::osal::OSThreadResult>(0);
}

/**
 * @brief Constructor.
 */
VersionFetcher::VersionFetcher(
    const std::string& type, const std::string& address,
    VersionManager* parent) : is_connected_(), connection_(NULL), type_(type),
    address_(address), parent_manager_(parent), recv_thread_(NULL) {
}

/**
 * @brief Destructor.
 */
VersionFetcher::~VersionFetcher() {
}

/**
 * @brief Get senscord version.
 * @return Status object.
 */
Status VersionFetcher::RequestVersion() {
  if (parent_manager_ == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument, "invalid parameter");
  }
  ConnectionManager* connection_manager = ConnectionManager::GetInstance();
  Status status = connection_manager->CreateConnection(type_, &connection_);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  std::map<std::string, std::string> arguments;
  connection_manager->GetArguments(type_, &arguments);
  // connection open
  status = connection_->Open(arguments);
  if (!status.ok()) {
    connection_manager->ReleaseConnection(connection_);
    return SENSCORD_STATUS_TRACE(status);
  }
  // start recv thread
  int32_t ret = osal::OSCreateThread(&recv_thread_, RecvThread, this, NULL);
  if (ret != 0) {
    connection_->Close();
    connection_manager->ReleaseConnection(connection_);
    return SENSCORD_STATUS_FAIL("messenger",
        Status::kCauseAborted,
        "failed to OSCreateThread: %" PRIx32, ret);
  }
  return Status::OK();
}

/**
 * @brief Send get version request to server.
 * @return Status object.
 */
Status VersionFetcher::SendGetVersionCommand() {
  Message msg = {};
  msg.header.server_stream_id = 0;  // ignore
  msg.header.request_id = 0;        // ignore
  msg.header.type = senscord::kMessageTypeRequest;
  msg.header.data_type = kMessageDataTypeGetVersion;
  msg.data = NULL;                  // ignore
  Status status = connection_->Send(msg);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief The method of connection thread.
 */
void VersionFetcher::ConnectionThreadCore() {
  ConnectionManager* connection_manager = ConnectionManager::GetInstance();
  // connect to server
  Status status = connection_->Connect(address_);
  if (!status.ok()) {
    NotifyCancel(status);
    connection_->Close();
    connection_manager->ReleaseConnection(connection_);
    SENSCORD_LOG_ERROR("%s", status.ToString().c_str());
    return;
  }
  is_connected_ = true;
  // send to server
  status = SendGetVersionCommand();
  if (!status.ok()) {
    NotifyCancel(status);
    connection_->Close();
    connection_manager->ReleaseConnection(connection_);
    SENSCORD_LOG_ERROR("%s", status.ToString().c_str());
    return;
  }
  // receiving
  ReceivingProcess();
  // disconnect
  status = connection_->Close();
  if (!status.ok()) {
    SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
  }
  is_connected_ = false;
  connection_manager->ReleaseConnection(connection_);
  connection_ = NULL;
}

/**
 * @brief Receiving response from server.
 */
void VersionFetcher::ReceivingProcess() {
  SENSCORD_LOG_DEBUG("start receiving");
  while (is_connected_) {
    Status status = connection_->WaitReadable(kRecvWaitTime);
    if (is_connected_ && (status.ok())) {
      // recv
      Message msg = {};
      status = connection_->Recv(&msg);
      if (status.ok()) {
        // processing messages.
        bool is_successful = DealMessage(&msg);
        ReleaseCommandReply(&msg);
        if (is_successful) {
          break;
        }
      } else if (status.cause() == Status::kCauseCancelled) {
        // disconnected
        SENSCORD_LOG_ERROR("disconnect server");
        NotifyCancel(status);
        is_connected_ = false;
        ReleaseCommandReply(&msg);
        break;
      } else {
        // unexpected case
        SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
        ReleaseCommandReply(&msg);
      }
    } else {
      // timeout: no message incoming
      SENSCORD_LOG_ERROR("receive processing timeout: %s",
          status.ToString().c_str());
      NotifyCancel(status);
      break;
    }
  }
  SENSCORD_LOG_DEBUG("end receiving");
}

/**
 * @brief Dealing the received message.
 * @param[in] (msg) The new incoming message.
 * @return Successfully processed Getversion reply.
 */
bool VersionFetcher::DealMessage(Message* msg) {
  if (msg->header.data_type != kMessageDataTypeGetVersion) {
    SENSCORD_LOG_WARNING("receiving message: %d", msg->header.type);
    return false;
  }
  const MessageDataVersionReply& reply_data =
    *reinterpret_cast<const MessageDataVersionReply*>(msg->data);
  parent_manager_->NotifyServerVersion(
      this, &reply_data.version, reply_data.status.Get());
  return true;
}

/**
 * @brief Release the reply message.
 * @param[in] (msg) The reply message.
 */
void VersionFetcher::ReleaseCommandReply(Message* msg) {
  if (msg != NULL) {
    connection_->ReleaseMessage(msg->header, msg->data);
  }
}

/**
 * @brief Notify the cancellation in response to the parent request.
 * @param[in] (status) Error status.
 */
void VersionFetcher::NotifyCancel(const Status& status) {
  parent_manager_->NotifyServerVersion(this, NULL, status);
}

/**
 * @brief Wait connection thread join.
 */
void VersionFetcher::WaitPostProcess() {
  senscord::osal::OSJoinThread(recv_thread_, NULL);
}

}  // namespace senscord
