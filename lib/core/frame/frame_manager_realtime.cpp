/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "frame/frame_manager_realtime.h"

#include <stdint.h>

#include "senscord/develop/common_types.h"
#include "stream/stream_core.h"
#include "frame/frame_manager_core.h"
#include "util/autolock.h"

namespace senscord {

/**
 * @brief Initialize FrameManager.
 * @param[in] (num) Number of frame.
 * @param[in] (stream) Parent stream.
 * @return Status object.
 */
Status FrameManagerRealtime::Init(int32_t num, StreamCore* stream) {
  Status status = FrameManagerCore::Init(0, stream);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Gets frame buffer.
 * @param[out] (acquirable) Flag of acquirable frame added.
 * @return Frame buffer pointer.
 */
FrameBuffer* FrameManagerRealtime::GetBuffer(bool* acquirable) {
  *acquirable = true;
  // incoming queue release and clear
  while (!incoming_queue_.empty()) {
    // pop oldest frame.
    FrameBuffer frame_buffer = incoming_queue_.front();
    incoming_queue_.pop_front();
    // drop
    SendFrameDropEvent(frame_buffer.frame->GetFrameInfo());
    ReleaseFrame(frame_buffer);
    delete frame_buffer.frame;
    *acquirable = false;
  }
  reserved_count_ = 0;  // does not use.
  // add
  incoming_queue_.push_back(FrameBuffer());
  return &incoming_queue_.back();
}

}    // namespace senscord
