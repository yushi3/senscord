/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "frame/frame_manager_overwrite.h"

#include <string>
#include <vector>

#include "logger/logger.h"
#include "senscord/develop/common_types.h"
#include "stream/stream_core.h"
#include "frame/frame_manager_core.h"
#include "senscord/osal.h"
#include "util/autolock.h"

namespace senscord {

/**
 * @brief Gets frame buffer.
 * @param[out] (acquirable) Flag of acquirable frame added.
 * @return Frame buffer pointer.
 */
FrameBuffer* FrameManagerOverwrite::GetBuffer(bool* acquirable) {
  if (reserved_count_ > 0) {
    --reserved_count_;
    *acquirable = true;
  } else if (!incoming_queue_.empty()) {
    // pop oldest frame.
    FrameBuffer frame_buffer = incoming_queue_.front();
    incoming_queue_.pop_front();
    // drop
    SendFrameDropEvent(frame_buffer.frame->GetFrameInfo());
    ReleaseFrame(frame_buffer);
    delete frame_buffer.frame;
    *acquirable = false;
  } else {
    return NULL;
  }
  // add
  incoming_queue_.push_back(FrameBuffer());
  return &incoming_queue_.back();
}

}    // namespace senscord
