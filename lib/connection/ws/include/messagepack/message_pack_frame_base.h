/*
 * SPDX-FileCopyrightText: 2017 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_MESSAGE_PACK_FRAME_BASE_CLASS_H_
#define LIB_MESSAGE_PACK_FRAME_BASE_CLASS_H_

#include <string>
#include "messagepack/message_pack_common.h"

namespace senscord {

/**
 * @brief The base class of Property/Frame for for Pack/Unpack.
 */
class MessagePackFrameBase {
 public:
  /**
   * @brief Get this component instance type(Frame,Property).
   * @param[out] (new_message_pack) The new MessagePack.
   * @return instance name.
   */
  ComponentType GetInstanceType() { return kComponentTypeFrame; }

  /**
   * @brief Get this component instance name(Frame or Property key).
   * @param[out] (new_message_pack) The new MessagePack.
   * @return instance name.
   */
  virtual std::string GetInstanceName() = 0;

  /**
   * @brief Serialize frame to MessagePack for send data.
   * @param[in] (handle) The JS handle for sending SendFrame.
   * @param[in] (data) The message data for sending SendFrame.
   * @param[in] (jobMessage) Accepted job messages.
   * @param[in/out] (dst) Messagepack body of the property.
   * @return Status object.
   */
//  Status SerializeFrameToMsgPack(const std::string handle,
//      const MesssageDataFrameLocalMemory &data,
//      JobMessage &jobMessage,
//      std::vector<uint8_t> &dst) = 0;

 private:
};

}  // namespace senscord

#endif  // LIB_MESSAGE_PACK_FRAME_BASE_CLASS_H_
