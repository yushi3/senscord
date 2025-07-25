/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "frame/frame_manager_discard.h"

#include <stdint.h>

#include "logger/logger.h"
#include "senscord/develop/common_types.h"
#include "frame/frame_manager_core.h"
#include "senscord/osal.h"
#include "util/autolock.h"

namespace senscord {

/**
 * @brief Gets frame buffer.
 * @param[out] (acquirable) Flag of acquirable frame added.
 * @return Frame buffer pointer.
 */
FrameBuffer* FrameManagerDiscard::GetBuffer(bool* acquirable) {
  if (reserved_count_ <= 0) {
    return NULL;
  }
  --reserved_count_;
  *acquirable = true;
  // add
  incoming_queue_.push_back(FrameBuffer());
  return &incoming_queue_.back();
}

}    // namespace senscord
