/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "frame/frame_manager_unlimited.h"

#include <stdint.h>

#include "senscord/develop/common_types.h"
#include "stream/stream_core.h"
#include "frame/frame_manager_core.h"
#include "util/autolock.h"

namespace senscord {

/**
 * @brief Initialize.
 * @param (num) Number of frame.
 * @param (stream) Parent stream.
 * @return Status object.
 */
Status FrameManagerUnlimited::Init(int32_t num, StreamCore* stream) {
  Status status = FrameManagerCore::Init(0, stream);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Gets frame buffer.
 * @param[out] (acquirable) Flag of acquirable frame added.
 * @return Frame buffer pointer.
 */
FrameBuffer* FrameManagerUnlimited::GetBuffer(bool* acquirable) {
  *acquirable = true;
  reserved_count_ = 0;  // does not use.
  // add
  incoming_queue_.push_back(FrameBuffer());
  return &incoming_queue_.back();
}

}    // namespace senscord
