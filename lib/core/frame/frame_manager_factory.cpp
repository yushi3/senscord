/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "frame/frame_manager_factory.h"

#include <stdint.h>

#include "senscord/stream.h"
#include "frame/frame_manager_discard.h"
#include "frame/frame_manager_overwrite.h"
#include "frame/frame_manager_unlimited.h"
#include "frame/frame_manager_realtime.h"
#include "logger/logger.h"

namespace senscord {

/**
 * @brief Create FrameManager
 * @param (config) frame manager config.
 * @return FrameManager instance, NULL is failed.
 */
FrameManager* FrameManagerFactory::CreateInstance(
    const FrameBuffering& config) {
  FrameManager* instance = NULL;

  if (config.buffering == kBufferingOff) {
    instance = new FrameManagerRealtime();
  } else {
    if (config.num != kBufferNumUnlimited) {
      switch (config.format) {
        case kBufferingFormatDiscard: {
          instance = new FrameManagerDiscard();
          break;
        }
        case kBufferingFormatDefault: /* through */
        case kBufferingFormatOverwrite: {
          instance = new FrameManagerOverwrite();
          break;
        }
        default: {
          SENSCORD_LOG_ERROR("system error");
          return NULL;
        }
      }
    } else {
      instance = new FrameManagerUnlimited();
    }
  }

  return instance;
}

/**
 * @brief Destroy FrameManager
 * @param (instance) FrameManager instance.
 * @return Status object.
 */
Status FrameManagerFactory::DestroyInstance(FrameManager* instance) {
  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  delete instance;
  return Status::OK();
}

}    // namespace senscord
