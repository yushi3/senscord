/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_CONNECTION_H_
#define SENSCORD_CONNECTION_H_

#include "senscord/config.h"

#ifdef SENSCORD_SERVER

#include <inttypes.h>
#include <string>
#include <vector>
#include <map>

#include "senscord/status.h"
#include "senscord/serialize.h"
#include "senscord/connection_types.h"
#include "senscord/frame.h"

/**
 * @def Macro for the new connection registration.
 */
#define SENSCORD_REGISTER_CONNECTION(connection_class_name)      \
  extern "C" void* CreateConnection() {                          \
    return new connection_class_name();                          \
  }                                                              \
  extern "C" void DestroyConnection(void* connection) {          \
    delete reinterpret_cast<senscord::Connection*>(connection);  \
  }

namespace senscord {

/**
 * @brief The message type.
 */
enum MessageType {
  kMessageTypeUnknown = 0,
  kMessageTypeRequest,
  kMessageTypeReply,
  kMessageTypeSendFrame,
  kMessageTypeSendEvent,
  kMessageTypeHandshake,
};

/**
 * @brief The payload data type.
 */
enum MessageDataType {
  kMessageDataTypeUnknown = 0,
  kMessageDataTypeOpen,
  kMessageDataTypeClose,
  kMessageDataTypeStart,
  kMessageDataTypeStop,
  kMessageDataTypeReleaseFrame,
  kMessageDataTypeGetProperty,
  kMessageDataTypeSetProperty,
  kMessageDataTypeLockProperty,
  kMessageDataTypeUnlockProperty,
  kMessageDataTypeSendFrame,
  kMessageDataTypeSendEvent,
  kMessageDataTypeDisconnect,
  kMessageDataTypeSecondaryConnect,
  kMessageDataTypeRegisterEvent,
  kMessageDataTypeUnregisterEvent,
  kMessageDataTypeGetVersion,
  kMessageDataTypeGetPropertyList,
  kMessageDataTypeGetStreamList,
  kMessageDataTypeGetServerConfig,  // SENSCORD_SERVER_SETTING
  kMessageDataTypeOpenPublisher,
  kMessageDataTypeClosePublisher,
};

}  // namespace senscord

SENSCORD_SERIALIZE_ADD_ENUM(senscord::MessageType)
SENSCORD_SERIALIZE_ADD_ENUM(senscord::MessageDataType)

namespace senscord {
/**
 * @brief The invalid value of server stream id.
 */
const uint64_t kInvalidServerStreamId = 0;

/**
 * @brief The message header of communicate between clients/sevrer.
 */
struct MessageHeader {
  /** The ID of stream on server. same to Stream* address. */
  uint64_t server_stream_id;

  /** The ID when request. If reply then this value same its request. */
  uint64_t request_id;

  /** The type of this message. */
  MessageType type;

  /** The type of this message's payload data. */
  MessageDataType data_type;

  SENSCORD_SERIALIZE_DEFINE(server_stream_id, request_id, type, data_type)
};

/**
 * @brief The message type of communicate between clients/sevrer.
 */
struct Message {
  /** Message header. */
  MessageHeader header;
  /** Message data. (pointer to MessageDataXxx) */
  void* data;
};

/**
 * @brief The interface class for the connection on SDK.
 */
class Connection {
 public:
  /**
   * @brief Initialize of the connection only once when use.
   * @param[in] (param) The informations of the init.
   * @return Status object.
   */
  virtual Status Init(const std::string& param) {
    return Status::OK();
  }

  /**
   * @brief Terminates the only once at the end of the connection.
   * @return Status object.
   */
  virtual Status Exit() {
    return Status::OK();
  }

  /**
   * @brief Search the connection.
   * @param[out] (param) Search results.
   * @return Status object.
   */
  virtual Status Search(std::vector<std::string>* param) {
    return Status::OK();
  }

  /**
   * @brief Open the connection.
   * @param[in] (arguments) connection arguments.
   * @return Status object.
   */
  virtual Status Open(const std::map<std::string, std::string>& arguments) {
    // Override if necessary.
    return Open();
  }

  /**
   * @brief Open the connection.
   * @return Status object.
   */
  virtual Status Open() {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented");
  }

  /**
   * @brief Close the connection.
   * @return Status object.
   */
  virtual Status Close() = 0;

  /**
   * @brief Connect to the target.
   * @param[in] (param) The informations of the connected target.
   * @return Status object.
   */
  virtual Status Connect(const std::string& param) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented");
  }

  /**
   * @brief Bind as the server.
   * @param[in] (param) The informations of the binding.
   * @return Status object.
   */
  virtual Status Bind(const std::string& param) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented");
  }

  /**
   * @brief Start to listen the connection.
   * @return Status object.
   */
  virtual Status Listen() {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented");
  }

  /**
   * @brief Accept the incoming connection.
   * @param[out] (new_connection) The new connection.
   * @param[out] (is_same_system) Whether new connection is on same system.
   * @return Status object.
   */
  virtual Status Accept(Connection** new_connection, bool* is_same_system) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented");
  }

  /**
   * @brief Send the message to the connected target.
   * @param[in] (msg) The semding message.
   * @return Status object.
   */
  virtual Status Send(const Message& msg) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented");
  }

  /**
   * @brief Receive the message from the connected target.
   * @param[out] (msg) The receiving message.
   * @return Status object.
   */
  virtual Status Recv(Message* msg) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented");
  }

  /**
   * @brief Releases message data generated by Recv function.
   * @param[in] (msg_header) Message header.
   * @param[in] (msg_data) Pointer to message data to release.
   * @return Status object.
   */
  virtual Status ReleaseMessage(const MessageHeader& msg_header,
                                void* msg_data) const {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented");
  }

  /**
   * @brief Get raw data for SendFrame of the server.
   * @param[in] (channel) Channel object.
   * @param[out] (rawdata) Information of the raw data to send.
   * @return Status object.
   */
  virtual Status GetChannelRawData(const Channel* channel,
                                   ChannelRawDataInfo* rawdata) const {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented");
  }

  /**
   * @brief Wait to be readable this connection.
   * @param[in] (timeout) Nanoseconds for wating.
   * @return If be readable then return ok.
   */
  virtual Status WaitReadable(uint64_t timeout) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented");
  }

#if 0
  /**
   * @brief Wait to be writable this connection.
   * @param[in] (timeout) Nanoseconds for wating.
   * @return If be writable then return ok.
   */
  virtual Status WaitWritable(uint64_t timeout) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented");
  }
#endif

  /**
   * @brief Destructor.
   */
  virtual ~Connection() {}
};

}  // namespace senscord

#endif  // SENSCORD_SERVER
#endif  // SENSCORD_CONNECTION_H_
