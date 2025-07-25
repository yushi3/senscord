/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/osal_inttypes.h"
#include "senscord/osal.h"
#include "senscord/logger.h"
#include "messagepack/web_socket_bridge.h"

#include "messagepack/register_access_64_property.h"
#include "messagepack/register_access_32_property.h"
#include "messagepack/register_access_16_property.h"
#include "messagepack/register_access_08_property.h"
#include "messagepack/record_property.h"
#include "../ws_log_macro.h"

namespace senscord {

WebSocketBridge::WebSocketBridge() {
  LOG_D("WebSocketBridge() - enter");

  regist<RegisterAccess64PropertyComponent>();
  regist<RegisterAccess32PropertyComponent>();
  regist<RegisterAccess16PropertyComponent>();
  regist<RegisterAccess08PropertyComponent>();
  regist<RecordPropertyComponent>();

  LOG_D("WebSocketBridge() - properties=%d", property_comp_list_.size());
}

WebSocketBridge::~WebSocketBridge() {
  PropertyCompList::iterator itr_prop = property_comp_list_.begin();
  PropertyCompList::iterator end_prop = property_comp_list_.end();
  for (; itr_prop != end_prop; ++itr_prop) {
    MessagePackPropertyBase * prop =
        reinterpret_cast<MessagePackPropertyBase*>(itr_prop->second);
    delete prop;
  }
  // comment out for future use.
  // FrameCompList::iterator itr_frame = frame_comp_list_.begin();
  // FrameCompList::iterator end_frame = frame_comp_list_.end();
  // for (; itr_frame != end_frame; itr_frame++) {
  //   MessagePackFrameBase * prop = (MessagePackFrameBase *)itr_frame->second;
  //   delete prop;
  // }
}

/**
 * @brief Received Property MessagePack to SensCord BinaryProperty.
 * @param[in] (key) Key name of the property.
 * @param[in] (src) Message pack body of the property.
 * @param[in/out] (dst) Binary of the property.
 * @return none.
 */
Status WebSocketBridge::PropertyPackToBinary(const std::string key,
    std::vector<uint8_t> * src,  std::vector<uint8_t> * dst) {

  Status status = Status::OK();
  PropertyCompList::const_iterator itr;
  itr = property_comp_list_.find(senscord::PropertyUtils::GetKey(key));
  if (itr != property_comp_list_.end()) {
    if (src->size() == 0) {
      dst->resize(0);
      LOG_D("WebSocketBridge::PropertyPackToBinary()"
          " data length is zero - key=%s", key.c_str());
      return status;
    }
    MessagePackPropertyBase * prop =
        reinterpret_cast<MessagePackPropertyBase*>(itr->second);
    status = prop->MsgPackToBinary(src, dst);
  } else {
    if (src->size() == 0) {
      LOG_D("WebSocketBridge::PropertyPackToBinary()"
          " data length is zero - key=%s", key.c_str());
      return status;
    }
    std::copy(src->begin(), src->end(), back_inserter(*dst));
  }
  return status;
}

/**
 * @brief SensCord BinaryProperty to Property MessagePack for send data.
 * @param[in] (key) Key name of the property.
 * @param[in] (src) Body of the binary property.
 * @param[in/out] (dst) Messagepack body of the property.
 * @return none.
 */
Status WebSocketBridge::BinaryToPropertyPack(const std::string key,
      std::vector<uint8_t> * src,
      std::vector<uint8_t> * dst) {
  Status status = Status::OK();
  PropertyCompList::const_iterator itr;
  itr = property_comp_list_.find(senscord::PropertyUtils::GetKey(key));
  if (itr != property_comp_list_.end()) {
    MessagePackPropertyBase * prop =
        reinterpret_cast<MessagePackPropertyBase*>(itr->second);
    status = prop->BinaryToMsgPack(src, dst);
  } else {
    std::copy(src->begin(), src->end(), back_inserter(*dst));
  }
  return status;
}

}  // namespace senscord
