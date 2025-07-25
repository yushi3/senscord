/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_SERVER_CORE_CLIENTLESS_H_
#define LIB_SERVER_CORE_CLIENTLESS_H_

#include "senscord/senscord.h"

namespace senscord {
namespace server {

/**
 * @brief Core class to open stream without client.
 */
class CoreClientless : public Core {
 public:
  /**
   * @brief Constructor.
   */
  CoreClientless();

  /**
   * @brief Destructor.
   */
  ~CoreClientless();
};

}  // namespace server
}  // namespace senscord

#endif  // LIB_SERVER_CORE_CLIENTLESS_H_
