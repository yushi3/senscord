/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include "messagepack/register_access_16_property.h"

#if 0
/**
 * @brief Create component instance.
 * @return Created component instance. In case of failure, it returns NULL.
 */
extern "C" void* CreateComponent() {
  return new PseudoImageProperty();
}

/**
 * @brief Destroy component instance.
 * @param[in] component Instance created in CreateComponent().
 */
extern "C" void DestroyComponent(void* component) {
  delete reinterpret_cast<PseudoImageProperty*>(component);
}
#endif

namespace senscord {

/**
 * @brief Received Property MessagePack to SensCord BinaryProperty.
 * @param[in] (src) Message pack body of the property.
 * @param[in/out] (dst) Key of the property.
 * @return Status object.
 */
Status RegisterAccess16PropertyComponent::MsgPackToBinary(
    std::vector<uint8_t>* src, std::vector<uint8_t>* dst) {

  RegisterAccess16Property property = {};
  RegisterAccessPropertyJS property_js = {};

  Status status = DeserializeMsg(
      reinterpret_cast<char*>(src->data()), src->size(), &property_js);
  if (!status.ok()) {
    return status;
  }

  property.id = property_js.id;
  property.element =
      std::vector<RegisterAccessElement<uint16_t> >(property_js.element.size());
  for (size_t i = 0; i < property_js.element.size(); ++i) {
    property.element[i].address =
        property_js.element[i].address_low |
        static_cast<uint64_t>(property_js.element[i].address_high) << 32;
    if (property_js.element[i].data > std::numeric_limits<uint16_t>::max()) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidArgument,
          "data is out of range. (id:%" PRIu64 ", data:%" PRIu32")",
          property.element[i].address, property_js.element[i].data);
    }
    property.element[i].data =
        static_cast<uint16_t>(property_js.element[i].data);
  }

  return SENSCORD_STATUS_TRACE(PropertyToBinary(property, dst));
}

/**
 * @brief SensCord BinaryProperty to Property MessagePack for send data.
 * @param[in] (src) Key of the property.
 * @param[in/out] (payload) Body of the property.
 * @param[in/out] (len) Length of the property data.
 * @return Status object.
 */
Status RegisterAccess16PropertyComponent::BinaryToMsgPack(
    std::vector<uint8_t>* src, std::vector<uint8_t>* dst) {

  RegisterAccess16Property property = {};
  RegisterAccessPropertyJS property_js = {};

  Status status = BinaryToProperty(src, property);
  if (status.ok()) {
    property_js.id = property.id;
    property_js.element =
        std::vector<RegisterAccessElementJS >(property.element.size());
    for (size_t i = 0; i < property.element.size(); ++i) {
      property_js.element[i].address_low =
          property.element[i].address & 0xffffffff;
      property_js.element[i].address_high =
          static_cast<uint32_t>(property.element[i].address >> 32);
      property_js.element[i].data = property.element[i].data;
    }
    status = SerializeMessagePack(status, &property_js, dst);
  }
  return SENSCORD_STATUS_TRACE(status);
}

}  // namespace senscord
