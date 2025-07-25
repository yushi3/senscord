/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_CONNECTION_TYPES_H_
#define SENSCORD_CONNECTION_TYPES_H_

#include "senscord/config.h"

#ifdef SENSCORD_SERVER

#include <inttypes.h>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "senscord/status.h"
#include "senscord/property_types.h"
#include "senscord/senscord_types.h"

namespace senscord {

/**
 * @brief RawData delivery mode.
 */
enum DataDeliveringMode {
  /** Deliver all data.
   *
   * The server releases the frame after sending.
   */
  kDeliverAllData = 0,

  /** Deliver only address and size.
   *
   * The server waits for a response from the client and releases the frame.
   */
  kDeliverAddressSizeOnly,
};

}  // namespace senscord

SENSCORD_SERIALIZE_ADD_ENUM(senscord::DataDeliveringMode)

namespace senscord {

/**
 * @brief Status for reply message.
 */
struct MessageStatus {
  bool ok;
  int32_t level;
  int32_t cause;
  std::string message;
  std::string block;

  SENSCORD_SERIALIZE_DEFINE(ok, level, cause, message, block)

  Status Get() const {
    if (ok) {
      return Status::OK();
    }
    return Status(static_cast<Status::Level>(level),
                  static_cast<Status::Cause>(cause),
                  message.c_str()).SetBlock(block);
  }

  void Set(const Status& status) {
    ok = status.ok();
    level = status.level();
    cause = status.cause();
    message = status.message();
    block = status.block();
  }
};

/**
 * @brief The standard request message
 */
struct MessageDataStandardRequest {
  uint8_t dummy;    // no data (always 0)

  SENSCORD_SERIALIZE_DEFINE(dummy)
};

typedef MessageDataStandardRequest MessageDataCloseRequest;
typedef MessageDataStandardRequest MessageDataStartRequest;
typedef MessageDataStandardRequest MessageDataStopRequest;
typedef MessageDataStandardRequest MessageDataDisconnectRequest;
typedef MessageDataStandardRequest MessageDataSecondaryConnectRequest;
typedef MessageDataStandardRequest MessageDataGetVersionRequest;
typedef MessageDataStandardRequest MessageDataGetPropertyListRequest;
typedef MessageDataStandardRequest MessageDataClosePublisherRequest;
#ifdef SENSCORD_SERVER_SETTING
typedef MessageDataStandardRequest MessageDataGetConfigRequest;
#endif  // SENSCORD_SERVER_SETTING

/**
 * @brief The standard reply message.
 */
struct MessageDataStandardReply {
  MessageStatus status;      // result of request

  SENSCORD_SERIALIZE_DEFINE(status)
};

typedef MessageDataStandardReply MessageDataCloseReply;
typedef MessageDataStandardReply MessageDataStartReply;
typedef MessageDataStandardReply MessageDataStopReply;
typedef MessageDataStandardReply MessageDataSetPropertyReply;
typedef MessageDataStandardReply MessageDataUnlockPropertyReply;
typedef MessageDataStandardReply MessageDataReleaseFrameReply;
typedef MessageDataStandardReply MessageDataDisconnectReply;
typedef MessageDataStandardReply MessageDataSecondaryConnectReply;
typedef MessageDataStandardReply MessageDataRegisterEventReply;
typedef MessageDataStandardReply MessageDataUnregisterEventReply;
typedef MessageDataStandardReply MessageDataOpenPublisherReply;
typedef MessageDataStandardReply MessageDataClosePublisherReply;

/**
 * @brief The request message for OpenStream.
 */
struct MessageDataOpenRequest {
  std::string stream_key;
  std::map<std::string, std::string> arguments;

  SENSCORD_SERIALIZE_DEFINE(stream_key, arguments)
};

/**
 * @brief The reply message for OpenStream.
 */
struct MessageDataOpenReply {
  MessageStatus status;
  std::vector<std::string> property_key_list;

  SENSCORD_SERIALIZE_DEFINE(status, property_key_list)
};

/**
 * @brief The reply message for GetVersion.
 */
struct MessageDataVersionReply {
  MessageStatus status;
  SensCordVersion version;

  SENSCORD_SERIALIZE_DEFINE(status, version)
};

/**
 * @brief The reply message for GetPropertyList.
 */
struct MessageDataPropertyListReply {
  MessageStatus status;
  std::vector<std::string> property_list;

  SENSCORD_SERIALIZE_DEFINE(status, property_list)
};

/**
 * @brief The reply message for GetStreamList.
 */
struct MessageDataStreamListReply {
  MessageStatus status;
  std::vector<StreamTypeInfo> stream_list;

  SENSCORD_SERIALIZE_DEFINE(status, stream_list)
};

#ifdef SENSCORD_SERVER_SETTING
/**
 * @brief The reply message for GetConfig.
 */
struct MessageDataConfigReply {
  MessageStatus status;
  ServerConfig config;

  SENSCORD_SERIALIZE_DEFINE(status, config)
};
#endif  // SENSCORD_SERVER_SETTING

/**
 * @brief The request message for Get/SetProperty.
 */
struct MessageDataProperty {
  std::string key;
  BinaryProperty property;

  SENSCORD_SERIALIZE_DEFINE(key, property)
};

typedef MessageDataProperty MessageDataGetPropertyRequest;
typedef MessageDataProperty MessageDataSetPropertyRequest;

/**
 * @brief The reply message for GetProperty.
 */
struct MessageDataGetPropertyReply {
  MessageStatus status;
  std::string key;
  BinaryProperty property;

  SENSCORD_SERIALIZE_DEFINE(status, key, property)
};

/**
 * @brief The request message for LockProperty.
 */
struct MessageDataLockPropertyRequest {
  std::set<std::string> keys;
  int32_t timeout_msec;

  SENSCORD_SERIALIZE_DEFINE(keys, timeout_msec)
};

/**
 * @brief The reply message for LockProperty.
 */
struct MessageDataLockPropertyReply {
  MessageStatus status;
  uint64_t resource_id;

  SENSCORD_SERIALIZE_DEFINE(status, resource_id)
};

/**
 * @brief The request message for UnlockProperty.
 */
struct MessageDataUnlockPropertyRequest {
  uint64_t resource_id;

  SENSCORD_SERIALIZE_DEFINE(resource_id)
};

/**
 * @brief Raw data information for SendFrame.
 */
struct ChannelRawDataInfo {
  DataDeliveringMode delivering_mode;
  std::vector<uint8_t> rawdata;

  SENSCORD_SERIALIZE_DEFINE(delivering_mode, rawdata)
};

/**
 * @brief The message data corresponding to one channel.
 */
struct MessageDataChannelLocalMemory {
  uint32_t channel_id;
  std::string allocator_key;
  ChannelRawDataInfo rawdata_info;
  std::string rawdata_type;
  uint64_t timestamp;
  std::vector<MessageDataProperty> properties;
  std::vector<std::string> updated_property_keys;

  SENSCORD_SERIALIZE_DEFINE(channel_id, allocator_key, rawdata_info,
                            rawdata_type, timestamp, properties,
                            updated_property_keys)
};

/**
 * @brief The message data corresponding to one frame.
 */
struct MessageDataFrameLocalMemory {
  uint64_t sequence_number;
  uint64_t sent_time;
  std::vector<uint8_t> user_data;
  std::vector<MessageDataChannelLocalMemory> channels;

  SENSCORD_SERIALIZE_DEFINE(sequence_number, sent_time, user_data, channels)
};

/**
 * @brief The message data for SendFrame.
 */
struct MessageDataSendFrame {
  std::vector<MessageDataFrameLocalMemory> frames;

  SENSCORD_SERIALIZE_DEFINE(frames)
};

/**
 * @brief The reply message for SendFrame.
 */
struct MessageDataSendFrameReply {
  std::vector<uint64_t> sequence_numbers;

  SENSCORD_SERIALIZE_DEFINE(sequence_numbers)
};

/**
 * @brief The request message for ReleaseFrame.
 */
struct MessageDataReleaseFrameRequest {
  uint64_t sequence_number;
  bool rawdata_accessed;

  SENSCORD_SERIALIZE_DEFINE(sequence_number, rawdata_accessed)
};

/**
 * @brief The request message for OpenPublisher.
 */
struct MessageDataOpenPublisherRequest {
  std::string key;

  SENSCORD_SERIALIZE_DEFINE(key)
};

/**
 * @brief The message data for SendEvent.
 */
struct MessageDataSendEvent {
  std::string event_type;
  EventArgument args;

#ifdef  SENSCORD_STREAM_EVENT_ARGUMENT
  SENSCORD_SERIALIZE_DEFINE(event_type, args)
#else
  SENSCORD_SERIALIZE_DEFINE(event_type)
#endif  // SENSCORD_STREAM_EVENT_ARGUMENT
};

/**
 * @brief The request message for RegisterEvent.
 */
struct MessageDataRegisterEvent {
  std::string event_type;

  SENSCORD_SERIALIZE_DEFINE(event_type)
};

typedef MessageDataRegisterEvent MessageDataRegisterEventRequest;
typedef MessageDataRegisterEvent MessageDataUnregisterEventRequest;

}  // namespace senscord

#endif  // SENSCORD_SERVER
#endif  // SENSCORD_CONNECTION_TYPES_H_
