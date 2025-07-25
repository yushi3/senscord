/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_FRAME_FRAME_MANAGER_UNLIMITED_H_
#define LIB_CORE_FRAME_FRAME_MANAGER_UNLIMITED_H_

#include <stdint.h>

#include "senscord/develop/common_types.h"
#include "stream/stream_core.h"
#include "frame/frame_manager_core.h"

namespace senscord {

/**
 * @brief Manages Frame : Unlimited
 */
class FrameManagerUnlimited : public FrameManagerCore {
 public:
  /**
   * @brief Constructor.
   */
  FrameManagerUnlimited() {}

  /**
   * @brief Destructor.
   */
  ~FrameManagerUnlimited() {}

  /**
   * @brief Initialize.
   * @param (num) Number of frame.
   * @param (stream) Parent stream.
   * @return Status object.
   */
  virtual Status Init(int32_t num, StreamCore* stream);

 protected:
  /**
   * @brief Gets frame buffer.
   * @param[out] (acquirable) Flag of acquirable frame added.
   * @return Frame buffer pointer.
   */
  virtual FrameBuffer* GetBuffer(bool* acquirable);
};

}   // namespace senscord
#endif  // LIB_CORE_FRAME_FRAME_MANAGER_UNLIMITED_H_
