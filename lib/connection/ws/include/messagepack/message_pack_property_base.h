/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_MESSAGE_PACK_PROPERTY_BASE_CLASS_H_
#define LIB_MESSAGE_PACK_PROPERTY_BASE_CLASS_H_

#include <string>
#include <vector>

#include "messagepack/message_pack_common.h"

namespace senscord {


/**
 * @brief Information for single register access.
 */
struct RegisterAccessElementJS {
  uint32_t address_low;  /**< Target address. */
  uint32_t address_high; /**< Target address. */
  uint32_t data;         /**< Writing data or read data. */

  SENSCORD_SERIALIZE_DEFINE(address_low, address_high, data)
};

/**
 * @brief Property of standard register read/write access.
 */
struct RegisterAccessPropertyJS {
  /** Register ID. */
  uint32_t id;
  /** RegisterAccessElement array. */
  std::vector<RegisterAccessElementJS > element;

  SENSCORD_SERIALIZE_DEFINE(id, element)
};

/**
 * @brief Information for single 64 bit register access.
 */
struct RegisterAccess64ElementJS {
  uint32_t address_low;  /**< Target address. */
  uint32_t address_high; /**< Target address. */
  uint32_t data_low;         /**< Writing data or read data. */
  uint32_t data_high;         /**< Writing data or read data. */

  SENSCORD_SERIALIZE_DEFINE(address_low, address_high, data_low, data_high)
};

/**
 * @brief Property of standard register read/write access.
 */
struct RegisterAccess64PropertyJS {
  /** Register ID. */
  uint32_t id;
  /** RegisterAccessElement array. */
  std::vector<RegisterAccess64ElementJS > element;

  SENSCORD_SERIALIZE_DEFINE(id, element)
};

struct RecordPropertyJS {
  /**
   * State of recording.
   * If set to true, recording will start.
   * Startable only in the stream running state.
   */
  bool enabled;

  /**
   * Top directory path of recording files.
   * When to stop, this member is ignored.
   */
  std::string path;

  /**
   * The count of record frames.
   */
  uint32_t count;

  /**
   * Format names of each channel ID.
   * Frames of no specified channel ID will not be recorded.
   * For get the available formats, use RecorderListProperty.
   * When to stop, this member is ignored.
   * 
   * original : std::map<uint32_t, std::string> formats;
   * separate elements for javascript via message pack.
   *  - formats_num = formats.size();
   *  - formats_channel_ids = formats.key;
   *  - formats_format_names = formats.value;
   */
  uint32_t formats_num;
  std::vector<uint32_t> formats_channel_ids;
  std::vector<std::string> formats_format_names;

  /**
   * Number of the buffering of recording frame queue.
   * If set zero means the number equals one.
   * When to stop, this member is ignored.
   */
  uint32_t buffer_num;

  /**
   * Directory Naming Rules.
   * key is the directory type, value is a format string.
   * When to stop, this member is ignored.
   * 
   * original : std::map<std::string, std::string> name_rules;
   * separate elements for javascript via message pack.
   *  - name_rules_num             = name_rules.size();
   *  - name_rules_directory_types = name_rules.key;
   *  - name_rules_formats         = name_rules.value;
   */
  uint32_t name_rules_num;
  std::vector<std::string> name_rules_directory_types;
  std::vector<std::string> name_rules_formats;

  SENSCORD_SERIALIZE_DEFINE(enabled, path, count, formats_num,
      formats_channel_ids, formats_format_names, buffer_num,
      name_rules_num, name_rules_directory_types, name_rules_formats)
};


/**
 * @brief The base class of Property for for Message Pack/Unpack.
 */
class MessagePackPropertyBase {
 public:
  /**
   * @brief Constructor.
   */
  MessagePackPropertyBase() {}

  /**
   * @brief Destructor.
   */
  virtual ~MessagePackPropertyBase() {}

  /**
   * @brief Get this component instance type(Frame,Property).
   * @param[out] (new_message_pack) The new MessagePack.
   * @return instance name.
   */
  // virtual ComponentType GetInstanceType() = 0;

  /**
   * @brief Get this component instance name(Frame or Property key).
   * @param[out] (new_message_pack) The new MessagePack.
   * @return instance name.
   */
  virtual std::string GetInstanceName() = 0;

  /**
   * @brief Received Property MessagePack to SensCord BinaryProperty.
   * @param[in] (src) Message pack body of the property.
   * @param[in/out] (dst) Binary of the property.
   * @return Status object.
   */
  virtual Status MsgPackToBinary(std::vector<uint8_t> * src,
      std::vector<uint8_t> * dst) = 0;

  /**
   * @brief SensCord BinaryProperty to Property MessagePack for send data.
   * @param[in] (src) Key of the property.
   * @param[in/out] (payload) Body of the property.
   * @param[in/out] (dst) Messagepack body of the property.
   * @return Status object.
   */
  virtual Status BinaryToMsgPack(std::vector<uint8_t> * src,
      std::vector<uint8_t> * dst) = 0;

  template <typename T>
  Status SerializeMessagePack(
      Status& status, T* src, std::vector<uint8_t>* dst) {
    if (status.ok()) {
      try {
        SerializeMsg(src, dst);
      } catch (const std::exception& e) {
        status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidArgument,
            "unmatch source structure for MessagePack:%s", e.what());
      }
    }
    return status;
  }

 private:
};

}  // namespace senscord

#endif  // LIB_MESSAGE_PACK_PROPERTY_BASE_CLASS_H_
