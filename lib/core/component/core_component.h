/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_COMPONENT_CORE_COMPONENT_H_
#define LIB_CORE_COMPONENT_CORE_COMPONENT_H_

#include "senscord/senscord.h"

namespace senscord {

/**
 * @brief Core for Component.
 */
class CoreComponent : public Core {
 public:
  /**
   * @brief Initialize Core.
   * @return Status object.
   */
  virtual Status Init();

  /**
   * @brief Finalize Core.
   * @return Status object.
   */
  virtual Status Exit();

  /**
   * @brief Constructor.
   * @param[in] (behavior) The core behavior.
   */
  explicit CoreComponent(const CoreBehavior* behavior);

  /**
   * @brief Destructor.
   */
  ~CoreComponent();
};

}  // namespace senscord

#endif  // LIB_CORE_COMPONENT_CORE_COMPONENT_H_
