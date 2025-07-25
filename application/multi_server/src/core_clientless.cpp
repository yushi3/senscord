/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core_clientless.h"

#include "clientless_core_behavior.h"

namespace senscord {
namespace server {

/**
 * @brief Constructor.
 */
CoreClientless::CoreClientless() : Core() {
  SetBehavior(new ClientlessCoreBehavior());
}

/**
 * @brief Destructor.
 */
CoreClientless::~CoreClientless() {
}

}  // namespace server
}  // namespace senscord
