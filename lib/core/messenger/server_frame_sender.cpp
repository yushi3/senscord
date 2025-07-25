/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "messenger/server_frame_sender.h"

#include <utility>
#include <string>
#include <limits>

#include "senscord/develop/client_instance_utils.h"
#include "frame/channel_core.h"

namespace {
static const uint64_t kDefaultTimeout = 30000ULL * 1000 * 1000;  // 30,000 ms
static const char kMessengerPortId = 0;
static const char kMessengerPortType[] = "messenger";

/**
 * @brief The callback for port sending message arrived.
 * @param[in] (port_type) The type of port. (unused)
 * @param[in] (port_id) The ID of port type. (unused)
 * @param[in] (msg) The message of arrived frame.
 * @param[in] (arg) The private data as component pointer.
 */
static void CallbackMsgArrived(
    const std::string& port_type, int32_t port_id,
    senscord::Message* msg, void* arg) {
  if (arg) {
    senscord::ServerFrameSender* instance =
        reinterpret_cast<senscord::ServerFrameSender*>(arg);
    senscord::Status status = instance->PushSendingsMessage(msg);
    SENSCORD_STATUS_TRACE(status);
    if (!status.ok()) {
      SENSCORD_LOG_WARNING(status.ToString().c_str());
    }
  }
}
}  // namespace

namespace senscord {

/**
 * @brief Constructor.
 * @param[in] (topic) Parent messenger topic.
 */
ServerFrameSender::ServerFrameSender(MessengerTopic* topic)
    : FrameSender(topic), reply_timeout_nsec_(kDefaultTimeout),
      pending_frames_mutex_(), messenger_(), dummy_stream_() {
  pending_frames_mutex_ = new util::Mutex();
  dummy_stream_ = new StreamCore();   // for history book
}

/**
 * @brief Constructor (assigning ownership).
 * @param[in] (topic) Parent messenger topic.
 * @param[in] (old) Old server frame sender.
 */
ServerFrameSender::ServerFrameSender(
    MessengerTopic* topic, ServerFrameSender* old)
    : FrameSender(topic), reply_timeout_nsec_(kDefaultTimeout),
      pending_frames_mutex_(), messenger_(), dummy_stream_() {
  // move ownership of messengger
  PendingFrameMap release_frames;
  ClientMessenger* messenger = NULL;
  Stream* dummy_stream = NULL;
  {
    util::AutoLock lock(old->mutex_);
    messenger = old->messenger_;
    old->messenger_ = NULL;
    old->SetState(kFrameSenderCloseable);
    release_frames = old->pending_release_frames_;
    old->pending_release_frames_.clear();
    dummy_stream = old->dummy_stream_;
    old->dummy_stream_ = NULL;
  }
  {
    util::AutoLock lock(mutex_);
    messenger_ = messenger;
    messenger_->RegisterRequestCallback(CallbackMsgArrived, this);
    SetState(kFrameSenderRunning);
    pending_release_frames_ = release_frames;
    dummy_stream_ = dummy_stream;   // for history book
  }
  pending_frames_mutex_ = new util::Mutex();
}

/**
 * @brief Destructor
 */
ServerFrameSender::~ServerFrameSender() {
  delete dummy_stream_;
  dummy_stream_ = NULL;
  delete pending_frames_mutex_;
  pending_frames_mutex_ = NULL;
}

/**
 * @brief Open the frame sender.
 * @param[in] (arguments) Server connection parameters.
 * @return Status object.
 */
Status ServerFrameSender::Open(
    const std::string& key,
    const std::map<std::string, std::string>& arguments) {
  util::AutoLock lock(mutex_);
  key_ = key;
  std::string connection;
  std::string address;
  std::string address_secondary;  // not used
  ClientInstanceUtility::GetConnectionReplyTimeout(
      arguments, &reply_timeout_nsec_);
  ClientInstanceUtility::GetConnectionType(arguments, &connection);
  Status status = ClientInstanceUtility::GetConnectionAddress(
      arguments, &address, NULL);
  SENSCORD_STATUS_TRACE(status);

  if (status.ok()) {
    status = Connect(connection, address, address_secondary);
    SENSCORD_STATUS_TRACE(status);
  }
  return status;
}

/**
 * @brief Request to close port.
 * @param[in] (port_type) Port type to close.
 * @param[in] (port_id) Port ID to close.
 * @param[in] (stream_core) Stream to close.
 * @return Status object.
 */
Status ServerFrameSender::Close() {
  return SENSCORD_STATUS_TRACE(Disconnect());
}

/**
 * @brief Connect to server.
 * @param[in] (connection) Connection type.
 * @param[in] (address) Server address.
 * @param[in] (address_secondary) Server secondary address.
 * @return Status object.
 */
Status ServerFrameSender::Connect(
    const std::string& connection, const std::string& address,
    const std::string& address_secondary) {
  Status status;
  if (!messenger_) {
    messenger_ = new ClientMessenger;
    status = messenger_->Start(connection, address, address_secondary);
    if (status.ok()) {
      // create message
      MessageDataOpenPublisherRequest msg_data = {};
      msg_data.key = key_;
      Message msg = {};
      messenger_->CreateRequestMessage(
          &msg, kMessengerPortType, kMessengerPortId,
          kMessageDataTypeOpenPublisher, &msg_data);

      Message* reply = NULL;
      // send request
      status = messenger_->SendCommandRequest(msg);
      SENSCORD_STATUS_TRACE(status);

      if (status.ok()) {
        // wait reply
        status = messenger_->WaitCommandReply(
            msg.header.request_id, reply_timeout_nsec_, &reply);
        SENSCORD_STATUS_TRACE(status);
      }
      if (status.ok()) {
        // cast reply payload
        const MessageDataOpenReply& reply_data =
            *reinterpret_cast<const MessageDataOpenReply*>(reply->data);
        uint64_t server_stream_id = reply->header.server_stream_id;
        // check return status.
        status = reply_data.status.Get();
        SENSCORD_STATUS_TRACE(status);
        if (status.ok()) {
          // add server info
          messenger_->AddServerStreamId(
              kMessengerPortType, kMessengerPortId, server_stream_id);
          SENSCORD_LOG_DEBUG(
              "open port: server stream id: %" PRIx64, server_stream_id);
        }
        // release reply
        messenger_->ReleaseCommandReply(reply);
        messenger_->RegisterRequestCallback(CallbackMsgArrived, this);
      }
      if (!status.ok()) {
        messenger_->Stop();
      }
    }
    if (!status.ok()) {
      delete messenger_;
      messenger_ = NULL;
    }
  }
  return status;
}

/**
 * @brief Disconnect to server.
 * @return Status object.
 */
Status ServerFrameSender::Disconnect() {
  Status status;
  if (messenger_) {
    // create message data
    MessageDataClosePublisherRequest msg_data = {};
    Message msg = {};
    messenger_->CreateRequestMessage(
        &msg, kMessengerPortType, kMessengerPortId,
        kMessageDataTypeClosePublisher, &msg_data);

    Message* reply = NULL;
    status = messenger_->SendCommandRequest(msg);
    SENSCORD_STATUS_TRACE(status);

    if (status.ok()) {
      // wait reply
      status = messenger_->WaitCommandReply(
          msg.header.request_id, reply_timeout_nsec_, &reply);
      SENSCORD_STATUS_TRACE(status);
    }
    if (status.ok()) {
      const senscord::MessageDataCloseReply& reply_data =
          *reinterpret_cast<const senscord::MessageDataCloseReply*>(
              reply->data);

      // check return status.
      status = reply_data.status.Get();
      SENSCORD_STATUS_TRACE(status);

      // release reply
      messenger_->ReleaseCommandReply(reply);
    } else {
      if (!messenger_->IsConnected()) {
        // If it is disconnected, it returns OK to release the resource.
        SENSCORD_LOG_WARNING(
            "disconnected: req_id=%" PRIu64 ", %s",
            msg.header.request_id, status.ToString().c_str());
        status = senscord::Status::OK();
      }
    }
    if (status.ok()) {
      messenger_->DeleteServerStreamId(kMessengerPortType, kMessengerPortId);
      status = messenger_->Stop();
      SENSCORD_STATUS_TRACE(status);
    }
    if (status.ok()) {
      delete messenger_;
      messenger_ = NULL;
    }
  }
  return status;
}

/**
 * @brief Release frames.
 * @param[in] (frameinfo) frame to release.
 */
Status ServerFrameSender::ReleaseFrame(const FrameInfo& frameinfo) {
  return SENSCORD_STATUS_TRACE(topic_->ReleaseFrame(frameinfo));
}

/**
 * @brief Publish frames to connected stream.
 * @param[in] (frames) List of frame information to send.
 * @param[out] (dropped_frames) List of pointer of dropped frames.
 * @return Status object.
 */
Status ServerFrameSender::PublishFrames(
    const std::vector<FrameInfo>& frames,
    std::vector<const FrameInfo*>* dropped_frames) {
  std::vector<const FrameInfo*> pending_list;
  std::vector<const FrameInfo*> release_list;

  // create message data payload
  MessageDataSendFrame msg_data = {};
  GetMessageDataForSendFrames(frames, &msg_data, &pending_list, &release_list);

  Status status;
  if (!msg_data.frames.empty()) {
    {
      // Register the frame in the release pending list.
      for (std::vector<const FrameInfo*>::const_iterator
          itr = pending_list.begin(), end = pending_list.end();
          itr != end; ++itr) {
        PushPendingReleaseFrame(*(*itr));
      }
    }
    {
      Message msg = {};
      messenger_->CreateRequestMessage(
          &msg, kMessengerPortType, kMessengerPortId,
          kMessageDataTypeSendFrame, &msg_data);
      msg.header.type = kMessageTypeSendFrame;
      status = messenger_->SendCommandSendFrame(msg);
      SENSCORD_STATUS_TRACE(status);
    }
    if (!status.ok()) {
      // Unregister pending frame.
      for (std::vector<const FrameInfo*>::const_iterator
          itr = pending_list.begin(), end = pending_list.end();
          itr != end; ++itr) {
        PopPendingReleaseFrame((*itr)->sequence_number);
        release_list.push_back(*itr);
      }
    }
  } else {
    status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation,
        "There is no frame to send.");
  }
  if (!status.ok()) {
    dropped_frames->swap(release_list);
  }

  return SENSCORD_STATUS_TRACE(status);
}

/**
 * Get the message data payload from multiple frames.
 * @param[in] (src) List of frames.
 * @param[out] (dest) The message data location.
 * @param[out] (pending_list) List containing pending frames.
 * @param[out] (release_list) List containing release frames.
 * @return Status object.
 */
void ServerFrameSender::GetMessageDataForSendFrames(
    const std::vector<FrameInfo>& src, MessageDataSendFrame* dest,
    std::vector<const FrameInfo*>* pending_list,
    std::vector<const FrameInfo*>* release_list) const {
  dest->frames.resize(src.size());

  size_t count = 0;
  for (std::vector<FrameInfo>::const_iterator
      itr = src.begin(), end = src.end(); itr != end; ++itr) {
    bool pending = false;
    MessageDataFrameLocalMemory& msg_frame = dest->frames[count];

    Status status = GetMessageDataForSendFrame(
        &msg_frame, *itr, &pending);

    if (!status.ok()) {
      SENSCORD_LOG_WARNING(
          "(%p) failed to GetMessageDataForSendFrame: %s",
          this, status.ToString().c_str());

      release_list->push_back(&(*itr));
      continue;
    }

    if (pending) {
      pending_list->push_back(&(*itr));
    } else {
      SENSCORD_LOG_DEBUG("(%p) push_back release_list", this);
      release_list->push_back(&(*itr));
    }

    ++count;
  }

  if (dest->frames.size() != count) {
    SENSCORD_LOG_DEBUG("(%p) resize the frames", this);
    dest->frames.resize(count);
  }
}

/**
 * @brief Push the frame to release pending list.
 * @param[in] (pending_frame) Pending frame.
 */
void ServerFrameSender::PushPendingReleaseFrame(
    const FrameInfo& pending_frame) {
  uint64_t seq_num = pending_frame.sequence_number;
  util::AutoLock lock(pending_frames_mutex_);
  SENSCORD_LOG_DEBUG(
      "(%p) push pending release: seq_num=%" PRIx64,
      this, seq_num);
  pending_release_frames_.insert(
      std::make_pair(seq_num, pending_frame));
  SetState(kFrameSenderRunning);
}

/**
 * @brief Pop the frame from release pending list.
 * @param[in] (sequence_number) Sequence number of frame.
 * @return Pending frame.
 */
FrameInfo ServerFrameSender::PopPendingReleaseFrame(
    uint64_t sequence_number) {
  FrameInfo pending_frame = {};
  util::AutoLock lock(pending_frames_mutex_);
  PendingFrameMap::iterator itr =
      pending_release_frames_.find(sequence_number);
  SENSCORD_LOG_DEBUG(
      "(%p) pop pending release: seq_num=%" PRIx64 "%s",
      this, sequence_number,
      (itr != pending_release_frames_.end()) ? "" : " (not found)");
  if (itr != pending_release_frames_.end()) {
    pending_frame = itr->second;
    pending_release_frames_.erase(itr);
  }
  if (pending_release_frames_.empty()) {
    SetState(kFrameSenderCloseable);
  }
  return pending_frame;
}

/**
 * @brief Push the message for port seinding.
 * @param[in] (msg) Message from server.
 * @return Status object.
 */
Status ServerFrameSender::PushSendingsMessage(Message* msg) {
  Status status;
  // sending message
  if (msg->header.type == kMessageTypeRequest) {
    switch (msg->header.data_type) {
      case kMessageDataTypeReleaseFrame:
        ReleaseFrameByReleaseFrameRequest(*msg);
        break;
      default:
        SENSCORD_LOG_WARNING(
            "unknown message type: %d", msg->header.data_type);
        break;
    }
  } else if (msg->header.type == kMessageTypeReply) {
    switch (msg->header.data_type) {
      case kMessageDataTypeSendFrame:
        ReleaseFrameBySendFrameReply(*msg);
        break;
      default:
        SENSCORD_LOG_WARNING(
            "unknown message type: %d", msg->header.data_type);
        break;
    }
  } else {
    // unknown type
    status = SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidOperation,
        "unknown message: type=%d, data_type=%d",
        msg->header.type, msg->header.data_type);
  }
  messenger_->ReleaseCommandReply(msg);
  return status;
}

/**
 * @brief Release of frame by send frame reply message.
 * @param[in] (msg) The message for doing.
 * @return Status object.
 */
Status ServerFrameSender::ReleaseFrameBySendFrameReply(const Message& msg) {
  if (msg.data == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "msg.data is null");
  }

  const MessageDataSendFrameReply& msg_data =
      *reinterpret_cast<const MessageDataSendFrameReply*>(msg.data);

  for (std::vector<uint64_t>::const_iterator
      itr = msg_data.sequence_numbers.begin(),
      end = msg_data.sequence_numbers.end(); itr != end; ++itr) {
    FrameInfo pending_frame = PopPendingReleaseFrame(*itr);
    ReleaseFrame(pending_frame);
  }

  return Status::OK();
}

/**
 * @brief Release of frame by request message.
 * @param[in] (msg) The message for doing.
 * @return Status object.
 */
Status ServerFrameSender::ReleaseFrameByReleaseFrameRequest(
    const Message& msg) {
  Status status;
  if (msg.data == NULL) {
    status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "msg.data is null");
  } else {
    const MessageDataReleaseFrameRequest& msg_data =
        *reinterpret_cast<const MessageDataReleaseFrameRequest*>(msg.data);

    FrameInfo pending_frame =
        PopPendingReleaseFrame(msg_data.sequence_number);
    ReleaseFrame(pending_frame);
  }

  MessageDataReleaseFrameReply reply_data = {};
  reply_data.status.Set(status);

  // send reply
  Message reply_msg = {};
  reply_msg.header = msg.header;
  reply_msg.header.type = kMessageTypeReply;
  reply_msg.data = &reply_data;

  // send reply to server.
  status = messenger_->SendCommandReply(reply_msg);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get the message data payload from the frame.
 * @param[out] (dest) The message data location.
 * @param[in] (src) The frame gotten from stream.
 * @param[out] (is_pending_release) Flag for pending release frame.
 * @return Status object.
 */
Status ServerFrameSender::GetMessageDataForSendFrame(
    MessageDataFrameLocalMemory* dest, const FrameInfo& src,
    bool* is_pending_release) const {
  dest->sequence_number = src.sequence_number;
  dest->sent_time = src.sent_time;

  // channel data
  dest->channels.resize(src.channels.size());

  std::vector<ChannelRawData>::const_iterator itr = src.channels.begin();
  std::vector<ChannelRawData>::const_iterator end = src.channels.end();
  for (size_t ch_index = 0; itr != end; ++itr, ++ch_index) {
    MessageDataChannelLocalMemory& dest_ch = dest->channels[ch_index];

    // channel id
    dest_ch.channel_id = itr->channel_id;

    // rawdata memory
    if (itr->data_memory != NULL) {
      MemoryAllocator* allocator = itr->data_memory->GetAllocator();
      dest_ch.allocator_key = allocator->GetKey();
    }

    // Get raw data from Connection class.
    {
      ChannelCore ch(*itr, NULL, NULL);
      Status status = messenger_->GetChannelRawData(
          &ch, &dest_ch.rawdata_info);
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
    }

    // rawdata type
    dest_ch.rawdata_type = itr->data_type;

    // timestamp
    dest_ch.timestamp = itr->captured_timestamp;

    // all properties
    PropertyHistoryBook* history_book = topic_->GetPropertyHistoryBook();
    std::map<std::string, uint32_t> properties;
    history_book->ReferenceCurrentProperties(dest_ch.channel_id, &properties);
    dest_ch.properties.resize(properties.size());
    size_t prop_index = 0;
    for (std::map<std::string, uint32_t>::iterator
        prop_itr = properties.begin(), prop_end = properties.end();
        prop_itr != prop_end; ++prop_index, ++prop_itr) {
      MessageDataProperty& property = dest_ch.properties[prop_index];
      property.key = prop_itr->first;

      void* serialized = NULL;
      size_t serialized_size = 0;
      Status status = history_book->GetProperty(
          dest_ch.channel_id, property.key,
          prop_itr->second, &serialized, &serialized_size);
      if (status.ok()) {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(serialized);
        property.property.data.reserve(serialized_size);
        property.property.data.assign(ptr, ptr + serialized_size);
      }
    }

    // updated properties
    dest_ch.updated_property_keys.clear();
    history_book->GetUpdatedPropertyList(
        dummy_stream_, dest_ch.channel_id, properties,
        &dest_ch.updated_property_keys);

    history_book->ReleaseProperties(dest_ch.channel_id, properties);
  }

  if (is_pending_release != NULL) {
    *is_pending_release = true;
  }

  return Status::OK();
}
}   // namespace senscord
