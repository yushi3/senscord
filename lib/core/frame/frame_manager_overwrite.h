/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_FRAME_FRAME_MANAGER_OVERWRITE_H_
#define LIB_CORE_FRAME_FRAME_MANAGER_OVERWRITE_H_

#include <stdint.h>
#include <list>

#include "senscord/develop/common_types.h"
#include "stream/stream_core.h"
#include "frame/frame_manager_core.h"

namespace senscord {

/**
 * @brief Manages Frame : Overwrite
 */
class FrameManagerOverwrite : public FrameManagerCore {
 public:
  /**
   * @brief Constructor.
   */
  FrameManagerOverwrite() {}

  /**
   * @brief Destructor.
   */
  ~FrameManagerOverwrite() {}

 protected:
  /**
   * @brief Gets frame buffer.
   * @param[out] (acquirable) Flag of acquirable frame added.
   * @return Frame buffer pointer.
   */
  virtual FrameBuffer* GetBuffer(bool* acquirable);
};

}   // namespace senscord

#endif  // LIB_CORE_FRAME_FRAME_MANAGER_OVERWRITE_H_
