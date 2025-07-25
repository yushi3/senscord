/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_MESSAGE_PACK_REGISTER_ACCESS_16_PROPERTY_COMPONENT_H_
#define LIB_MESSAGE_PACK_REGISTER_ACCESS_16_PROPERTY_COMPONENT_H_

#include <inttypes.h>
#include <string>
#include <vector>
#include "senscord/osal.h"
#include "senscord/senscord.h"
#include "senscord/logger.h"
#include "messagepack/message_pack_property_base.h"

namespace senscord {
/**
 * @brief The base class of Property/Frame for MessagePack.
 */
class RegisterAccess16PropertyComponent : public MessagePackPropertyBase {
 public:
  /**
   * @brief Get this component instance type(Frame,Property).
   * @param[out] (new_message_pack) The new MessagePack.
   * @return instance name.
   */
  // ComponentType GetInstanceType() { return kComponentTypeProperty; }

  /**
   * @brief Get this component instance name(Frame or Property key).
   * @param[out] (new_message_pack) The new MessagePack.
   * @return instance name.
   */
  std::string GetInstanceName() { return kRegisterAccess16PropertyKey; }

  /**
   * @brief Received Property MessagePack to SensCord BinaryProperty.
   * @param[in] (src) Message pack body of the property.
   * @param[in/out] (dst) Binary of the property.
   * @return Status object.
   */
  Status MsgPackToBinary(std::vector<uint8_t> * src,
      std::vector<uint8_t> * dst);

  /**
   * @brief SensCord BinaryProperty to Property MessagePack for send data.
   * @param[in] (src) Key of the property.
   * @param[in/out] (dst) Messagepack body of the property.
   * @return Status object.
   */
  Status BinaryToMsgPack(std::vector<uint8_t> * src,
       std::vector<uint8_t> * dst);

 private:
};

}  // namespace senscord

#endif  // LIB_MESSAGE_PACK_REGISTER_ACCESS_16_PROPERTY_COMPONENT_H_
