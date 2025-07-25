/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "component/core_component.h"

#include "core/core_behavior.h"
#include "logger/logger.h"

namespace senscord {

/**
 * @brief Constructor.
 * @param[in] (behavior) The core behavior.
 */
CoreComponent::CoreComponent(const CoreBehavior* behavior) : Core() {
  if (behavior != NULL) {
    SetBehavior(behavior->CreateInstance());
  }

  SetConfigManager(behavior->GetConfigManager());

  Status status = Core::Init();
  if (!status.ok()) {
    SENSCORD_LOG_WARNING("failed to core init: %s",
                         status.ToString().c_str());
  }
}

/**
 * @brief Destructor.
 */
CoreComponent::~CoreComponent() {
  Status status = Core::Exit();
  if (!status.ok()) {
    SENSCORD_LOG_WARNING("failed to core exit: %s",
                         status.ToString().c_str());
  }
}

/**
 * @brief Initialize Core.
 * @return Status object.
 */
Status CoreComponent::Init() {
  // This function may be called from Component or StreamSource,
  // but does nothing here.
  return Status::OK();
}

/**
 * @brief Finalize Core.
 * @return Status object.
 */
Status CoreComponent::Exit() {
  // This function may be called from Component or StreamSource,
  // but does nothing here.
  return Status::OK();
}

}  // namespace senscord
