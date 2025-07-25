/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>

#include "senscord/osal_inttypes.h"
#include "senscord/osal.h"
#include "senscord/serialize.h"
#include "senscord/develop/connection_utils.h"

namespace senscord {
namespace connection {

/**
 * @brief Find header from socket stream.
 * @param[in] (socket) Socket object.
 * @param[out] (header) Header.
 * @param[in] (timeout_nsec) Timeout relative time. (minus means no timeout)
 * @return Status object.
 */
Status FindHeader(
    osal::OSSocket* socket, Header* header, int64_t timeout_nsec) {
  uint8_t buffer[sizeof(Header)];
  uint32_t read_size = sizeof(buffer);
  while (true) {
    uint32_t offset = static_cast<uint32_t>(sizeof(buffer) - read_size);
    uint64_t start_time = 0;
    osal::OSGetTime(&start_time);
    Status status = ReceiveWithTimeout(
        socket, &buffer[offset], &read_size, timeout_nsec);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    if (timeout_nsec >= 0) {
      uint64_t end_time = 0;
      osal::OSGetTime(&end_time);
      int64_t elapsed = (end_time < start_time) ? 0 : (end_time - start_time);
      timeout_nsec = (timeout_nsec < elapsed) ? 0 : (timeout_nsec - elapsed);
#if 0
      osal::OSPrintf("timeout=%" PRId64 " (elapsed=%" PRId64 ")\n",
                     timeout_nsec, elapsed);
#endif
    }
    // find signature
    const uint32_t count = sizeof(buffer) - sizeof(kHeaderSignature);
    for (offset = 0; offset < count; ++offset) {
      if ((buffer[offset + 0] == kHeaderSignature[0]) &&
          (buffer[offset + 1] == kHeaderSignature[1]) &&
          (buffer[offset + 2] == kHeaderSignature[2]) &&
          (buffer[offset + 3] == kHeaderSignature[3])) {
        break;
      }
    }
    if (offset == 0) {
      break;
    }
    osal::OSMemmove(
        &buffer[0], sizeof(buffer),
        &buffer[offset], sizeof(buffer) - offset);
    read_size = offset;
  }
  *header = *reinterpret_cast<Header*>(buffer);
  return Status::OK();
}

/**
 * @brief Fixed size receive function with timeout.
 * @param[in] (socket) Socket object.
 * @param[out] (buffer) Destination buffer.
 * @param[in,out] (recv_size) in: Fixed size to receive, out: Received size.
 * @param[in] (timeout_nsec) Timeout relative time. (minus means no timeout)
 * @return Status object.
 */
Status ReceiveWithTimeout(
    osal::OSSocket* socket, void* buffer, uint32_t* recv_size,
    int64_t timeout_nsec) {
  Status status;
  uint8_t* ptr = reinterpret_cast<uint8_t*>(buffer);
  size_t total_received_size = 0;
  size_t remaining_size = *recv_size;
  *recv_size = 0;
  while (remaining_size > 0) {
    if (timeout_nsec >= 0) {
      std::vector<osal::OSSocket*> readable;
      readable.push_back(socket);
      int32_t ret = osal::OSRelativeTimedSelectSocket(
          &readable, NULL, NULL, static_cast<uint64_t>(timeout_nsec));
      if (ret < 0) {
        if (osal::error::IsTimeout(ret)) {
          status = SENSCORD_STATUS_FAIL(
              kStatusBlockCore, Status::kCauseTimeout,
              "Receive processing timed out.");
          break;
        }
        return SENSCORD_STATUS_FAIL(
            kStatusBlockCore, Status::kCauseCancelled,
            "Failed to select: return=0x%" PRIx32, ret);
      }
    }
    size_t received_size = 0;
    int32_t ret = osal::OSRecvSocket(
        socket, &ptr[total_received_size], remaining_size, &received_size);
    if (ret < 0) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseCancelled,
          "Failed to recv: return=0x%" PRIx32, ret);
    } else if (received_size == 0) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseCancelled, "Disconnected");
    }
    total_received_size += received_size;
    remaining_size -= received_size;
  }
  *recv_size = static_cast<uint32_t>(total_received_size);
  return status;
}

/**
 * @brief Serialize message header and message data.
 * @param[in] (msg) The message to serialize.
 * @param[out] (serialized_msg) The serialized message.
 * @return Status object.
 */
Status SerializeMessage(
    const Message& msg, serialize::Buffer* serialized_msg) {
  serialize::Encoder encoder(serialized_msg);

  // serialize header.
  Status status = encoder.Push(msg.header);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  if (msg.data == NULL) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // serialize data.
  switch (msg.header.type) {
    case kMessageTypeSendFrame: {
      MessageDataSendFrame* tmp =
          reinterpret_cast<MessageDataSendFrame*>(msg.data);
      status = encoder.Push(*tmp);
      SENSCORD_STATUS_TRACE(status);
      break;
    }
    case kMessageTypeSendEvent: {
      MessageDataSendEvent* tmp =
          reinterpret_cast<MessageDataSendEvent*>(msg.data);
      status = encoder.Push(*tmp);
      SENSCORD_STATUS_TRACE(status);
      break;
    }
    case kMessageTypeRequest: {
      switch (msg.header.data_type) {
        case kMessageDataTypeOpen: {
          MessageDataOpenRequest* tmp =
              reinterpret_cast<MessageDataOpenRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeClose: {
          MessageDataCloseRequest* tmp =
              reinterpret_cast<MessageDataCloseRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeStart: {
          MessageDataStartRequest* tmp =
              reinterpret_cast<MessageDataStartRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeStop: {
          MessageDataStopRequest* tmp =
              reinterpret_cast<MessageDataStopRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeReleaseFrame: {
          MessageDataReleaseFrameRequest* tmp =
              reinterpret_cast<MessageDataReleaseFrameRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeGetProperty: {
          MessageDataGetPropertyRequest* tmp =
              reinterpret_cast<MessageDataGetPropertyRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeSetProperty: {
          MessageDataSetPropertyRequest* tmp =
              reinterpret_cast<MessageDataSetPropertyRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeLockProperty: {
          MessageDataLockPropertyRequest* tmp =
              reinterpret_cast<MessageDataLockPropertyRequest*>(msg.data);
          status = encoder.Push(*tmp);
          break;
        }
        case kMessageDataTypeUnlockProperty: {
          MessageDataUnlockPropertyRequest* tmp =
              reinterpret_cast<MessageDataUnlockPropertyRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeDisconnect: {
          MessageDataDisconnectRequest* tmp =
              reinterpret_cast<MessageDataDisconnectRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeSecondaryConnect: {
          MessageDataSecondaryConnectRequest* tmp =
              reinterpret_cast<MessageDataSecondaryConnectRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeRegisterEvent: {
          MessageDataRegisterEventRequest* tmp =
              reinterpret_cast<MessageDataRegisterEventRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeUnregisterEvent: {
          MessageDataUnregisterEventRequest* tmp =
              reinterpret_cast<MessageDataUnregisterEventRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeGetVersion: {
          MessageDataGetVersionRequest* tmp =
              reinterpret_cast<MessageDataGetVersionRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeGetPropertyList: {
          MessageDataGetPropertyListRequest* tmp =
              reinterpret_cast<MessageDataGetPropertyListRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeOpenPublisher: {
          MessageDataOpenPublisherRequest* tmp =
              reinterpret_cast<MessageDataOpenPublisherRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeClosePublisher: {
          MessageDataClosePublisherRequest* tmp =
              reinterpret_cast<MessageDataClosePublisherRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
#ifdef SENSCORD_SERVER_SETTING
        case kMessageDataTypeGetServerConfig: {
          MessageDataGetConfigRequest* tmp =
              reinterpret_cast<MessageDataGetConfigRequest*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
#endif  // SENSCORD_SERVER_SETTING
        default:
          status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
              Status::kCauseInvalidArgument,
              "invalid MessageDataType: type=Request, data_type=%" PRIu32,
              msg.header.data_type);
          break;
      }
      break;
    }
    case kMessageTypeReply: {
      switch (msg.header.data_type) {
        case kMessageDataTypeOpen: {
          MessageDataOpenReply* tmp =
              reinterpret_cast<MessageDataOpenReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeClose: {
          MessageDataCloseReply* tmp =
              reinterpret_cast<MessageDataCloseReply*>(msg.data);
          status = encoder.Push(*tmp);
          break;
        }
        case kMessageDataTypeStart: {
          MessageDataStartReply* tmp =
              reinterpret_cast<MessageDataStartReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeStop: {
          MessageDataStopReply* tmp =
              reinterpret_cast<MessageDataStopReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeReleaseFrame: {
          MessageDataReleaseFrameReply* tmp =
              reinterpret_cast<MessageDataReleaseFrameReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeGetProperty: {
          MessageDataGetPropertyReply* tmp =
              reinterpret_cast<MessageDataGetPropertyReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeSetProperty: {
          MessageDataSetPropertyReply* tmp =
              reinterpret_cast<MessageDataSetPropertyReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeLockProperty: {
          MessageDataLockPropertyReply* tmp =
              reinterpret_cast<MessageDataLockPropertyReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeUnlockProperty: {
          MessageDataUnlockPropertyReply* tmp =
              reinterpret_cast<MessageDataUnlockPropertyReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeSendFrame: {
          MessageDataSendFrameReply* tmp =
              reinterpret_cast<MessageDataSendFrameReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeDisconnect: {
          MessageDataDisconnectReply* tmp =
              reinterpret_cast<MessageDataDisconnectReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeSecondaryConnect: {
          MessageDataSecondaryConnectReply* tmp =
              reinterpret_cast<MessageDataSecondaryConnectReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeRegisterEvent: {
          MessageDataRegisterEventReply* tmp =
              reinterpret_cast<MessageDataRegisterEventReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeUnregisterEvent: {
          MessageDataUnregisterEventReply* tmp =
              reinterpret_cast<MessageDataUnregisterEventReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeGetVersion: {
          MessageDataVersionReply* tmp =
              reinterpret_cast<MessageDataVersionReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeGetPropertyList: {
          MessageDataPropertyListReply* tmp =
              reinterpret_cast<MessageDataPropertyListReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeOpenPublisher: {
          MessageDataOpenPublisherReply* tmp =
              reinterpret_cast<MessageDataOpenPublisherReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
        case kMessageDataTypeClosePublisher: {
          MessageDataClosePublisherReply* tmp =
              reinterpret_cast<MessageDataClosePublisherReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
#ifdef SENSCORD_SERVER_SETTING
        case kMessageDataTypeGetServerConfig: {
          MessageDataConfigReply* tmp =
              reinterpret_cast<MessageDataConfigReply*>(msg.data);
          status = encoder.Push(*tmp);
          SENSCORD_STATUS_TRACE(status);
          break;
        }
#endif  // SENSCORD_SERVER_SETTING
        default:
          status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
              Status::kCauseInvalidArgument,
              "invalid MessageDataType: type=Reply, data_type=%" PRIu32,
              msg.header.data_type);
          break;
      }
      break;
    }
    default:
      status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument,
          "invalid MessageType: type=0x%" PRIx32 ", data_type=%" PRIu32,
          msg.header.type, msg.header.data_type);
      break;
  }

  return status;
}

/**
 * @brief Deserialize message header and message data.
 * @param[in] (buffer) Pointer to the serialized buffer.
 * @param[in] (size) The size of the buffer.
 * @param[out] (msg) The deserialized message.
 * @return Status object.
 */
Status DeserializeMessage(const void* buffer, size_t size, Message* msg) {
  serialize::Decoder decoder(buffer, size);

  // deserialize header.
  Status status = decoder.Pop(msg->header);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  if (decoder.GetOffset() >= size) {
    msg->data = NULL;
    return Status::OK();
  }

  // deserialize data.
  switch (msg->header.type) {
    case kMessageTypeSendFrame: {
      MessageDataSendFrame* tmp = new MessageDataSendFrame;
      status = decoder.Pop(*tmp);
      SENSCORD_STATUS_TRACE(status);
      msg->data = tmp;
      break;
    }
    case kMessageTypeSendEvent: {
      MessageDataSendEvent* tmp = new MessageDataSendEvent;
      status = decoder.Pop(*tmp);
      SENSCORD_STATUS_TRACE(status);
      msg->data = tmp;
      break;
    }
    case kMessageTypeRequest: {
      switch (msg->header.data_type) {
        case kMessageDataTypeOpen: {
          MessageDataOpenRequest* tmp = new MessageDataOpenRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeClose: {
          MessageDataCloseRequest* tmp = new MessageDataCloseRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeStart: {
          MessageDataStartRequest* tmp = new MessageDataStartRequest;
          status = decoder.Pop(*tmp);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeStop: {
          MessageDataStopRequest* tmp = new MessageDataStopRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeReleaseFrame: {
          MessageDataReleaseFrameRequest* tmp =
              new MessageDataReleaseFrameRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeGetProperty: {
          MessageDataGetPropertyRequest* tmp =
              new MessageDataGetPropertyRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeSetProperty: {
          MessageDataSetPropertyRequest* tmp =
              new MessageDataSetPropertyRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeLockProperty: {
          MessageDataLockPropertyRequest* tmp =
              new MessageDataLockPropertyRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeUnlockProperty: {
          MessageDataUnlockPropertyRequest* tmp =
              new MessageDataUnlockPropertyRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeDisconnect: {
          MessageDataDisconnectRequest* tmp =
              new MessageDataDisconnectRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeSecondaryConnect: {
          MessageDataSecondaryConnectRequest* tmp =
              new MessageDataSecondaryConnectRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeRegisterEvent: {
          MessageDataRegisterEventRequest* tmp =
              new MessageDataRegisterEventRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeUnregisterEvent: {
          MessageDataUnregisterEventRequest* tmp =
              new MessageDataUnregisterEventRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeGetVersion: {
          MessageDataGetVersionRequest* tmp =
              new MessageDataGetVersionRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeGetPropertyList: {
          MessageDataGetPropertyListRequest* tmp =
              new MessageDataGetPropertyListRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeOpenPublisher: {
          MessageDataOpenPublisherRequest* tmp =
              new MessageDataOpenPublisherRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeClosePublisher: {
          MessageDataClosePublisherRequest* tmp =
              new MessageDataClosePublisherRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
#ifdef SENSCORD_SERVER_SETTING
        case kMessageDataTypeGetServerConfig: {
          MessageDataGetConfigRequest* tmp =
              new MessageDataGetConfigRequest;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
#endif  // SENSCORD_SERVER_SETTING
        default:
          status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
              Status::kCauseInvalidArgument,
              "invalid MessageDataType: type=Request, data_type=%" PRIu32,
              msg->header.data_type);
          break;
      }
      break;
    }
    case kMessageTypeReply: {
      switch (msg->header.data_type) {
        case kMessageDataTypeOpen: {
          MessageDataOpenReply* tmp = new MessageDataOpenReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeClose: {
          MessageDataCloseReply* tmp = new MessageDataCloseReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeStart: {
          MessageDataStartReply* tmp = new MessageDataStartReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeStop: {
          MessageDataStopReply* tmp = new MessageDataStopReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeReleaseFrame: {
          MessageDataReleaseFrameReply* tmp = new MessageDataReleaseFrameReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeGetProperty: {
          MessageDataGetPropertyReply* tmp = new MessageDataGetPropertyReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeSetProperty: {
          MessageDataSetPropertyReply* tmp = new MessageDataSetPropertyReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeLockProperty: {
          MessageDataLockPropertyReply* tmp = new MessageDataLockPropertyReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeUnlockProperty: {
          MessageDataUnlockPropertyReply* tmp =
              new MessageDataUnlockPropertyReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeSendFrame: {
          MessageDataSendFrameReply* tmp = new MessageDataSendFrameReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeDisconnect: {
          MessageDataDisconnectReply* tmp =
              new MessageDataDisconnectReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeSecondaryConnect: {
          MessageDataSecondaryConnectReply* tmp =
              new MessageDataSecondaryConnectReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeRegisterEvent: {
          MessageDataRegisterEventReply* tmp =
              new MessageDataRegisterEventReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeUnregisterEvent: {
          MessageDataUnregisterEventReply* tmp =
              new MessageDataUnregisterEventReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeGetVersion: {
          MessageDataVersionReply* tmp =
              new MessageDataVersionReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeGetPropertyList: {
          MessageDataPropertyListReply* tmp =
              new MessageDataPropertyListReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeOpenPublisher: {
          MessageDataOpenPublisherReply* tmp =
              new MessageDataOpenPublisherReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
        case kMessageDataTypeClosePublisher: {
          MessageDataClosePublisherReply* tmp =
              new MessageDataClosePublisherReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
#ifdef SENSCORD_SERVER_SETTING
        case kMessageDataTypeGetServerConfig: {
          MessageDataConfigReply* tmp =
              new MessageDataConfigReply;
          status = decoder.Pop(*tmp);
          SENSCORD_STATUS_TRACE(status);
          msg->data = tmp;
          break;
        }
#endif  // SENSCORD_SERVER_SETTING
        default:
          status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
              Status::kCauseInvalidArgument,
              "invalid MessageDataType: type=Reply, data_type=%" PRIu32,
              msg->header.data_type);
          break;
      }
      break;
    }
    default:
      status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument,
          "invalid MessageType: type=0x%" PRIx32 ", data_type=%" PRIu32,
          msg->header.type, msg->header.data_type);
      break;
  }

  return status;
}

/**
 * @brief Releases message data generated by DeserializeMessage function.
 * @param[in] (msg_header) Message header.
 * @param[in] (msg_data) Pointer to message data to release.
 * @return Status object.
 */
Status ReleaseMessage(const MessageHeader& msg_header, void* msg_data) {
  if (msg_data == NULL) {
    // do nothing.
    return Status::OK();
  }

  Status status;
  switch (msg_header.type) {
    case kMessageTypeSendFrame: {
      MessageDataSendFrame* tmp =
          reinterpret_cast<MessageDataSendFrame*>(msg_data);
      delete tmp;
      break;
    }
    case kMessageTypeSendEvent: {
      MessageDataSendEvent* tmp =
          reinterpret_cast<MessageDataSendEvent*>(msg_data);
      delete tmp;
      break;
    }
    case kMessageTypeRequest: {
      switch (msg_header.data_type) {
        case kMessageDataTypeOpen: {
          MessageDataOpenRequest* tmp =
              reinterpret_cast<MessageDataOpenRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeClose: {
          MessageDataCloseRequest* tmp =
              reinterpret_cast<MessageDataCloseRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeStart: {
          MessageDataStartRequest* tmp =
              reinterpret_cast<MessageDataStartRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeStop: {
          MessageDataStopRequest* tmp =
              reinterpret_cast<MessageDataStopRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeReleaseFrame: {
          MessageDataReleaseFrameRequest* tmp =
              reinterpret_cast<MessageDataReleaseFrameRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeGetProperty: {
          MessageDataGetPropertyRequest* tmp =
              reinterpret_cast<MessageDataGetPropertyRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeSetProperty: {
          MessageDataSetPropertyRequest* tmp =
              reinterpret_cast<MessageDataSetPropertyRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeLockProperty: {
          MessageDataLockPropertyRequest* tmp =
              reinterpret_cast<MessageDataLockPropertyRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeUnlockProperty: {
          MessageDataUnlockPropertyRequest* tmp =
              reinterpret_cast<MessageDataUnlockPropertyRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeDisconnect: {
          MessageDataDisconnectRequest* tmp =
              reinterpret_cast<MessageDataDisconnectRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeSecondaryConnect: {
          MessageDataSecondaryConnectRequest* tmp =
              reinterpret_cast<MessageDataSecondaryConnectRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeRegisterEvent: {
          MessageDataRegisterEventRequest* tmp =
              reinterpret_cast<MessageDataRegisterEventRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeUnregisterEvent: {
          MessageDataUnregisterEventRequest* tmp =
              reinterpret_cast<MessageDataUnregisterEventRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeGetVersion: {
          MessageDataGetVersionRequest* tmp =
              reinterpret_cast<MessageDataGetVersionRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeGetPropertyList: {
          MessageDataGetPropertyListRequest* tmp =
              reinterpret_cast<MessageDataGetPropertyListRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeOpenPublisher: {
          MessageDataOpenPublisherRequest* tmp =
              reinterpret_cast<MessageDataOpenPublisherRequest*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeClosePublisher: {
          MessageDataClosePublisherRequest* tmp =
              reinterpret_cast<MessageDataClosePublisherRequest*>(msg_data);
          delete tmp;
          break;
        }
#ifdef SENSCORD_SERVER_SETTING
        case kMessageDataTypeGetServerConfig: {
          MessageDataGetConfigRequest* tmp =
              reinterpret_cast<MessageDataGetConfigRequest*>(msg_data);
          delete tmp;
          break;
        }
#endif  // SENSCORD_SERVER_SETTING
        default:
          status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
              Status::kCauseInvalidArgument,
              "invalid MessageDataType: type=Request, data_type=%" PRIu32,
              msg_header.data_type);
          break;
      }
      break;
    }
    case kMessageTypeReply: {
      switch (msg_header.data_type) {
        case kMessageDataTypeOpen: {
          MessageDataOpenReply* tmp =
              reinterpret_cast<MessageDataOpenReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeClose: {
          MessageDataCloseReply* tmp =
              reinterpret_cast<MessageDataCloseReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeStart: {
          MessageDataStartReply* tmp =
              reinterpret_cast<MessageDataStartReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeStop: {
          MessageDataStopReply* tmp =
              reinterpret_cast<MessageDataStopReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeReleaseFrame: {
          MessageDataReleaseFrameReply* tmp =
              reinterpret_cast<MessageDataReleaseFrameReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeGetProperty: {
          MessageDataGetPropertyReply* tmp =
              reinterpret_cast<MessageDataGetPropertyReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeSetProperty: {
          MessageDataSetPropertyReply* tmp =
              reinterpret_cast<MessageDataSetPropertyReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeLockProperty: {
          MessageDataLockPropertyReply* tmp =
              reinterpret_cast<MessageDataLockPropertyReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeUnlockProperty: {
          MessageDataUnlockPropertyReply* tmp =
              reinterpret_cast<MessageDataUnlockPropertyReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeSendFrame: {
          MessageDataSendFrameReply* tmp =
              reinterpret_cast<MessageDataSendFrameReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeDisconnect: {
          MessageDataDisconnectReply* tmp =
              reinterpret_cast<MessageDataDisconnectReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeSecondaryConnect: {
          MessageDataSecondaryConnectReply* tmp =
              reinterpret_cast<MessageDataSecondaryConnectReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeRegisterEvent: {
          MessageDataRegisterEventReply* tmp =
              reinterpret_cast<MessageDataRegisterEventReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeUnregisterEvent: {
          MessageDataUnregisterEventReply* tmp =
              reinterpret_cast<MessageDataUnregisterEventReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeGetVersion: {
          MessageDataVersionReply* tmp =
              reinterpret_cast<MessageDataVersionReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeGetPropertyList: {
          MessageDataPropertyListReply* tmp =
              reinterpret_cast<MessageDataPropertyListReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeOpenPublisher: {
          MessageDataOpenPublisherReply* tmp =
              reinterpret_cast<MessageDataOpenPublisherReply*>(msg_data);
          delete tmp;
          break;
        }
        case kMessageDataTypeClosePublisher: {
          MessageDataClosePublisherReply* tmp =
              reinterpret_cast<MessageDataClosePublisherReply*>(msg_data);
          delete tmp;
          break;
        }
#ifdef SENSCORD_SERVER_SETTING
        case kMessageDataTypeGetServerConfig: {
          MessageDataConfigReply* tmp =
              reinterpret_cast<MessageDataConfigReply*>(msg_data);
          delete tmp;
          break;
        }
#endif  // SENSCORD_SERVER_SETTING
        default:
          status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
              Status::kCauseInvalidArgument,
              "invalid MessageDataType: type=Reply, data_type=%" PRIu32,
              msg_header.data_type);
          break;
      }
      break;
    }
    default:
      status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument,
          "invalid MessageType: type=0x%" PRIx32 ", data_type=%" PRIu32,
          msg_header.type, msg_header.data_type);
      break;
  }

  return status;
}

}   // namespace connection
}   // namespace senscord
