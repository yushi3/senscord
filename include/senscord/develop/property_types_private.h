/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_PROPERTY_TYPES_PRIVATE_H_
#define SENSCORD_DEVELOP_PROPERTY_TYPES_PRIVATE_H_

#include <stdint.h>
#include <string>

#include "senscord/config.h"
#include "senscord/serialize_define.h"

namespace senscord {

/**
 * RegisterEventProperty
 */
const char kRegisterEventPropertyKey[] = "register_event_property";

/**
 * UnregisterEventProperty
 */
const char kUnregisterEventPropertyKey[] = "unregister_event_property";

/**
 * @brief Structure for the register event to server
 */
struct RegisterEventProperty {
  std::string event_type;     /**< Register event */

  SENSCORD_SERIALIZE_DEFINE(event_type)
};

/**
 * FrameExtensionProperty
 */
const char kFrameExtensionPropertyKey[] = "frame_extension_property";

/**
 * @brief Structure for the frame extension to server
 */
struct FrameExtensionProperty {
  bool disabled;     /**< disable frame extension */
  SENSCORD_SERIALIZE_DEFINE(disabled)
};

}   // namespace senscord

#endif  // SENSCORD_DEVELOP_PROPERTY_TYPES_PRIVATE_H_
