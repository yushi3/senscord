/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "client_adapter.h"
#include <string>
#include <utility>
#include "senscord/osal.h"
#include "server_log.h"

namespace senscord {
namespace server {

/**
 * @brief Constructor.
 * @param[in] (manager) The client adapter manager.
 * @param[in] (connection) The connection interface.
 */
ClientAdapterBase::ClientAdapterBase(
    ClientAdapterManager* manager, Connection* connection)
    : manager_(manager), connection_(connection), thread_(), end_flag_() {}

/**
 * @brief Destructor.
 */
ClientAdapterBase::~ClientAdapterBase() {
  if (connection_) {
    SENSCORD_SERVER_LOG_DEBUG(
        "[server] release accept connection: %p", connection_);
    connection_->Close();
    delete connection_;
    connection_ = NULL;
  }
}

/**
 * @brief Start to receive the message to the client.
 * @return Status object.
 */
Status ClientAdapterBase::Start() {
  if (thread_) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "already started");
  }
  end_flag_ = false;
  int32_t ret = osal::OSCreateThread(&thread_, ThreadProc, this, NULL);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        senscord::Status::kCauseAborted,
        "failed to create the client monitoring thread: %" PRIx32, ret);
  }
  return Status::OK();
}

/**
 * @brief Stop to receive the message.
 * @return Status object.
 */
Status ClientAdapterBase::Stop() {
  if (thread_) {
    end_flag_ = true;
    osal::OSJoinThread(thread_, NULL);
    thread_ = NULL;
  }
  return Status::OK();
}

/**
 * @brief Set the secondary client adapter.
 * @param[in] (stream_id) Identifier of server stream.
 * @param[in] (client) Secondary client adapter. (If NULL, reset)
 * @return true if it is set.
 */
bool ClientAdapterBase::SetSecondaryAdapter(
    uint64_t stream_id, ClientAdapterBase* client) {
  // override.
  return false;
}

/**
 * @brief Working thread for receiving.
 * @param[in] (arg) The instance of client adapter.
 * @return Always returns normal.
 */
osal::OSThreadResult ClientAdapterBase::ThreadProc(void* arg) {
  if (arg) {
    ClientAdapterBase* adapter = reinterpret_cast<ClientAdapterBase*>(arg);
    adapter->Monitoring();
  }
  return static_cast<osal::OSThreadResult>(0);
}

/**
 * @brief The method of the client receiving thread.
 */
void ClientAdapterBase::Monitoring() {
  SENSCORD_SERVER_LOG_DEBUG("[server] start adapter");

  Status status;
  while (!end_flag_) {
    status = connection_->WaitReadable(1000000000);
    if (status.ok()) {
      status = RecvMessage();
      if (status.cause() == Status::kCauseCancelled) {
        break;
      }
    } else if (status.cause() != Status::kCauseTimeout) {
      SENSCORD_SERVER_LOG_WARNING("[server] client connection failed: %s",
          status.ToString().c_str());
      break;
    } else {
      // no massage was arrived
    }
  }
  SENSCORD_SERVER_LOG_DEBUG("[server] end adapter");

  OnMonitoringFinished();
  manager_->Release(this);
}

/**
 * @brief Send the message to client.
 * @param[in] (msg) The message to send.
 * @return Status object.
 **/
Status ClientAdapterBase::SendMessageToClient(const Message& msg) {
  Status status = connection_->Send(msg);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Release the message.
 * @param[in] (msg) The message to release.
 */
void ClientAdapterBase::ReleaseMessage(const Message* msg) const {
  if (msg != NULL) {
    connection_->ReleaseMessage(msg->header, msg->data);
  }
  delete msg;
}

/**
 * @brief This function is called when monitoring is finished.
 */
void ClientAdapterBase::OnMonitoringFinished() {
  // Override
}

/**
 * @brief The subroutine to receive new message.
 * @return Status object.
 */
Status ClientAdapter::RecvMessage() {
  Message* msg = new Message();
  Status status = connection_->Recv(msg);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    if (msg->header.type == kMessageTypeHandshake) {
      ReleaseMessage(msg);
      return status;
    }
    // incoming new message
    status = Acception(msg);
    SENSCORD_STATUS_TRACE(status);
    if (!status.ok()) {
      SENSCORD_SERVER_LOG_ERROR("[server] failed to accept message: %s",
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
 * @brief This function is called when monitoring is finished.
 */
void ClientAdapter::OnMonitoringFinished() {
  // ignore the secondary request because the primary is disconnected.
  osal::OSLockMutex(secondary_adapter_mutex_);
  secondary_adapter_ = NULL;
  osal::OSUnlockMutex(secondary_adapter_mutex_);
  CloseAllStream();
}

/**
 * @brief The subroutine to receive new message.
 * @param[in] (msg) New received message.
 * @return Status object.
 */
Status ClientAdapter::Acception(Message* msg) {
  SENSCORD_SERVER_LOG_DEBUG("[server] incoming new message: "
      "func=%d, type=%d, request_id=%" PRIu64 ", stream_id=%" PRIx64,
      msg->header.data_type, msg->header.type, msg->header.request_id,
      msg->header.server_stream_id);

  if ((msg->header.type != kMessageTypeRequest) &&
      (msg->header.type != kMessageTypeReply)) {
    MessageType unknown_type = msg->header.type;
    ReleaseMessage(msg);
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "unknown message type: %d",
        unknown_type);
  }

  if (msg->header.data_type == kMessageDataTypeDisconnect) {
    // end of connection
    SENSCORD_SERVER_LOG_DEBUG(
        "[server] receive the request of disconnection.");

    // send reply
    // after this, connection will be disconnected by client.
    MessageDataDisconnectReply reply_data = {};
    reply_data.status.Set(Status::OK());
    Status status = SendReply(*msg, NULL, reply_data);
    ReleaseMessage(msg);

    return SENSCORD_STATUS_TRACE(status);
  }

  if (msg->header.data_type == kMessageDataTypeOpen) {
    // open new stream
    Status status = OpenStream(*msg);
    ReleaseMessage(msg);
    return SENSCORD_STATUS_TRACE(status);
  } else if (msg->header.data_type == kMessageDataTypeGetVersion) {
    Status status = GetVersion(*msg);
    ReleaseMessage(msg);
    return SENSCORD_STATUS_TRACE(status);
  } else if (msg->header.data_type == kMessageDataTypeGetStreamList) {
    Status status = GetStreamList(*msg);
    ReleaseMessage(msg);
    return SENSCORD_STATUS_TRACE(status);
  }

  // search stream adapter
  StreamAdapter* adapter = GetAdapter(msg->header.server_stream_id);
  if (adapter == NULL) {
    // unknown stream
    Status status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "unknown stream: 0x%" PRIx64,
        msg->header.server_stream_id);
    MessageStatus msg_status;
    msg_status.Set(status);
    status = SendReply(*msg, reinterpret_cast<Stream*>(
        msg->header.server_stream_id), msg_status);
    ReleaseMessage(msg);
    return SENSCORD_STATUS_TRACE(status);
  }

  // register messgae
  if (msg->header.data_type == kMessageDataTypeClose) {
    // close
    Status status = CloseStream(*msg, adapter);
    ReleaseMessage(msg);
    return SENSCORD_STATUS_TRACE(status);
  }

  // other command
  adapter->PushMessage(msg);
  return Status::OK();
}

/**
 * @brief Get the version.
 * @param[in] (msg) The received message.
 * @return Status object.
 */
Status ClientAdapter::GetVersion(const Message& msg) {
  SENSCORD_SERVER_LOG_DEBUG("[server] request to SensCord version");

  // reply
  MessageDataVersionReply reply_data = {};

  // get version
  Status status = core_->GetVersion(&reply_data.version);
  SENSCORD_STATUS_TRACE(status);
  if (!status.ok()) {
    SENSCORD_SERVER_LOG_DEBUG("[server] failed to version: %s",
        status.ToString().c_str());
  }

  reply_data.status.Set(status);

  // send reply
  status = SendReply(msg, NULL, reply_data);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get the stream list.
 * @param[in] (msg) The received message.
 * @return Status object.
 */
Status ClientAdapter::GetStreamList(const Message& msg) {
  SENSCORD_SERVER_LOG_DEBUG("[server] request to stream list");

  // reply
  MessageDataStreamListReply reply_data = {};

  // stream list
  Status status = core_->GetStreamList(&reply_data.stream_list);
  SENSCORD_STATUS_TRACE(status);
  if (!status.ok()) {
    SENSCORD_SERVER_LOG_DEBUG("[server] failed to stream list: %s",
        status.ToString().c_str());
  } else {
    status = Status::OK();
  }

  reply_data.status.Set(status);

  // send reply
  status = SendReply(msg, NULL, reply_data);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Open new stream.
 * @param[in] (msg) The received message.
 * @return Status object.
 */
Status ClientAdapter::OpenStream(const Message& msg) {
  if (msg.data == NULL) {
    Status status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "msg.data is null");

    MessageDataOpenReply reply_data = {};
    reply_data.status.Set(status);

    // send reply
    status = SendReply(msg, NULL, reply_data);
    return SENSCORD_STATUS_TRACE(status);
  }
  // create parameters of OpenStream.
  const MessageDataOpenRequest& request =
      *reinterpret_cast<const MessageDataOpenRequest*>(msg.data);
  SENSCORD_SERVER_LOG_DEBUG("[server] request to open new stream: %s",
      request.stream_key.c_str());

  OpenStreamSetting open_setting = {};
  open_setting.frame_buffering.buffering = kBufferingOn;
  open_setting.frame_buffering.num = 0;  // unlimited

  // get stream setting.
  config_manager_.GetStreamConfigByStreamKey(
      request.stream_key, &open_setting);

  SENSCORD_SERVER_LOG_DEBUG("[server] stream setting: "
      "buffering=%d, num=%" PRId32 ", format=%d",
      open_setting.frame_buffering.buffering,
      open_setting.frame_buffering.num,
      open_setting.frame_buffering.format);

  // open stream
  Stream* stream = NULL;
  MessageDataOpenReply reply_data = {};
  Status status = core_->OpenStream(request.stream_key, open_setting, &stream);
  SENSCORD_STATUS_TRACE(status);
  if (!status.ok()) {
    SENSCORD_SERVER_LOG_DEBUG("[server] failed to open: %s",
        status.ToString().c_str());
  } else {
    SENSCORD_SERVER_LOG_INFO("[server] open stream: key=%s, id=%p",
        request.stream_key.c_str(), stream);

    // get property list
    stream->GetPropertyList(&reply_data.property_key_list);

    // create adapter.
    status = CreateStreamAdapter(stream);
    if (!status.ok()) {
      SENSCORD_STATUS_TRACE(status);
      SENSCORD_SERVER_LOG_ERROR("[server] %s", status.ToString().c_str());

      // close stream force.
      core_->CloseStream(stream);
    }
  }

  reply_data.status.Set(status);

  // send reply
  status = SendReply(msg, stream, reply_data);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Close the stream.
 * @param[in] (msg) The received message.
 * @param[in] (adapter) The removing stream adapter.
 * @return Status object.
 */
Status ClientAdapter::CloseStream(const Message& msg, StreamAdapter* adapter) {
  Stream* stream = reinterpret_cast<Stream*>(msg.header.server_stream_id);
  SENSCORD_SERVER_LOG_DEBUG("[server] request to close stream: %p", stream);

  Status status = adapter->StopMonitoring();
  if (!status.ok()) {
    SENSCORD_SERVER_LOG_WARNING(
        "[server] failed to stop monitoring: %s",
        status.ToString().c_str());
  }

  // closing
  status = core_->CloseStream(stream);
  SENSCORD_STATUS_TRACE(status);
  if (!status.ok()) {
    SENSCORD_SERVER_LOG_WARNING("[server] failed to close: %s",
        status.ToString().c_str());

    adapter->StartMonitoring();
  } else {
    SENSCORD_SERVER_LOG_INFO("[server] close stream: id=%p", stream);

    // delete adapter
    DeleteStreamAdapter(adapter);
  }

  MessageDataCloseReply reply_data = {};
  reply_data.status.Set(status);

  // send reply
  status = SendReply(msg, stream, reply_data);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Create the new stream adapter. Need to lock.
 * @param[in] (stream) The pointer of new stream.
 * @return Status object.
 */
Status ClientAdapter::CreateStreamAdapter(Stream* stream) {
  StreamAdapter* adapter = new StreamAdapter(stream, this);

  Status status = adapter->StartMonitoring();
  if (!status.ok()) {
    delete adapter;
    return SENSCORD_STATUS_TRACE(status);
  }

  osal::OSLockMutex(stream_adapters_mutex_);
  stream_adapters_.insert(
      std::make_pair(reinterpret_cast<uint64_t>(stream), adapter));
  osal::OSUnlockMutex(stream_adapters_mutex_);
  return Status::OK();
}

/**
 * @brief Delete the stream adapter. Need to lock.
 * @param[in] (adapter) The stream adapter.
 * @return Status object.
 */
Status ClientAdapter::DeleteStreamAdapter(StreamAdapter* adapter) {
  osal::OSLockMutex(stream_adapters_mutex_);
  stream_adapters_.erase(reinterpret_cast<uint64_t>(adapter->GetStream()));
  osal::OSUnlockMutex(stream_adapters_mutex_);

  adapter->StopMonitoring();
  delete adapter;
  return Status::OK();
}

/**
 * @brief Get the stream adapter pointer. Need to lock.
 * @param[in] (stream_id) Identifier of server stream.
 * @return The pointer of stream adapter. NULL means failed.
 */
StreamAdapter* ClientAdapter::GetAdapter(uint64_t stream_id) const {
  StreamAdapter* adapter = NULL;
  osal::OSLockMutex(stream_adapters_mutex_);
  StreamAdapterList::const_iterator itr = stream_adapters_.find(stream_id);
  if (itr != stream_adapters_.end()) {
    adapter = itr->second;
  }
  osal::OSUnlockMutex(stream_adapters_mutex_);
  return adapter;
}

/**
 * @brief Set the secondary client adapter.
 * @param[in] (stream_id) Identifier of server stream.
 * @param[in] (client) Secondary client adapter. (If NULL, reset)
 * @return true if it is set.
 */
bool ClientAdapter::SetSecondaryAdapter(
    uint64_t stream_id, ClientAdapterBase* client) {
  bool result = false;
  osal::OSLockMutex(stream_adapters_mutex_);
  StreamAdapterList::iterator itr = stream_adapters_.find(stream_id);
  if (itr != stream_adapters_.end()) {
    osal::OSLockMutex(secondary_adapter_mutex_);
    secondary_adapter_ = client;
    osal::OSUnlockMutex(secondary_adapter_mutex_);
    result = true;
  }
  osal::OSUnlockMutex(stream_adapters_mutex_);
  return result;
}

/**
 * @brief Close all streams.
 */
void ClientAdapter::CloseAllStream() {
  osal::OSLockMutex(stream_adapters_mutex_);
  StreamAdapterList tmp_list;
  tmp_list.swap(stream_adapters_);
  osal::OSUnlockMutex(stream_adapters_mutex_);

  StreamAdapterList::iterator itr = tmp_list.begin();
  StreamAdapterList::iterator end = tmp_list.end();
  for (; itr != end; ++itr) {
    StreamAdapter* adapter = itr->second;

    // stop adapter
    adapter->StopMonitoring();

    // close stream
    Stream* stream = adapter->GetStream();
    core_->CloseStream(stream);

    // delete adapter
    delete adapter;
  }
  tmp_list.clear();
}

/**
 * @brief Get raw data from Connection class.
 * @param[in] (channel) Channel object.
 * @param[out] (rawdata) Information of the raw data to send.
 * @return Status object.
 */
Status ClientAdapter::GetChannelRawData(
    const Channel* channel, ChannelRawDataInfo* rawdata) const {
  Status status = connection_->GetChannelRawData(channel, rawdata);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Send the message to client.
 * @param[in] (msg) The message to send.
 * @return Status object.
 **/
Status ClientAdapter::SendMessageToClient(const Message& msg) {
  Status status;
  bool sent = false;
  if (msg.header.type == kMessageTypeSendFrame &&
      msg.header.data_type == kMessageDataTypeSendFrame) {
    osal::OSLockMutex(secondary_adapter_mutex_);
    if (secondary_adapter_ != NULL) {
      status = secondary_adapter_->SendMessageToClient(msg);
      if (status.ok()) {
        sent = true;
      }
    }
    osal::OSUnlockMutex(secondary_adapter_mutex_);
  }
  if (!sent) {
    status = ClientAdapterBase::SendMessageToClient(msg);
    SENSCORD_STATUS_TRACE(status);
  }
  return status;
}

/**
 * @brief Constructor.
 * @param[in] (manager) The client adapter manager.
 * @param[in] (connection) The connection inteface.
 * @param[in] (core) The SDK Core instance.
 * @param[in] (config_manager) The config manager.
 */
ClientAdapter::ClientAdapter(
    ClientAdapterManager* manager, Connection* connection,
    Core* core, const ConfigManager& config_manager)
    : ClientAdapterBase(manager, connection),
      core_(core), config_manager_(config_manager), stream_adapters_mutex_(),
      secondary_adapter_(), secondary_adapter_mutex_() {
  SENSCORD_SERVER_LOG_INFO("[server] incoming new connection");
  osal::OSCreateMutex(&stream_adapters_mutex_);
  osal::OSCreateMutex(&secondary_adapter_mutex_);
}

/**
 * @brief Destructor.
 */
ClientAdapter::~ClientAdapter() {
  CloseAllStream();
  osal::OSDestroyMutex(secondary_adapter_mutex_);
  secondary_adapter_mutex_ = NULL;
  osal::OSDestroyMutex(stream_adapters_mutex_);
  stream_adapters_mutex_ = NULL;
}

}  // namespace server
}   // namespace senscord
