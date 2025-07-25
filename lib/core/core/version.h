/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CORE_VERSION_H_
#define LIB_CORE_CORE_VERSION_H_

#include <stdint.h>

#include <string>

namespace senscord {

struct CoreVersion {
  static std::string Name();
  static uint32_t Major();
  static uint32_t Minor();
  static uint32_t Patch();
  static std::string Description();
};

}   // namespace senscord
#endif  // LIB_CORE_CORE_VERSION_H_
