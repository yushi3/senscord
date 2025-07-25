/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_FRAME_FRAME_MANAGER_FACTORY_H_
#define LIB_CORE_FRAME_FRAME_MANAGER_FACTORY_H_

#include <stdint.h>

#include "frame/frame_manager.h"
#include "senscord/stream.h"
#include "senscord/noncopyable.h"

namespace senscord {

/**
 * @brief Factory of FrameManager
 */
class FrameManagerFactory : private util::Noncopyable {
 public:
  /**
   * @brief Create FrameManager
   * @param (config) frame manager config.
   * @return FrameManager instance, NULL is failed.
   */
  static FrameManager* CreateInstance(const FrameBuffering& config);

  /**
   * @brief Destroy FrameManager
   * @param (instance) FrameManager instance.
   * @return Status object.
   */
  static Status DestroyInstance(FrameManager* instance);

 private:
  /**
   * @brief Constructor.
   */
  FrameManagerFactory() {}

  /**
   * @brief Destructor.
   */
  ~FrameManagerFactory() {}
};

}  // namespace senscord

#endif  // LIB_CORE_FRAME_FRAME_MANAGER_FACTORY_H_
