/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stream/frame_observer_simple.h"

#include "util/autolock.h"

namespace senscord {

/**
 * @brief Constructor.
 */
FrameObserverSimple::FrameObserverSimple() : is_start_(false) {
  param_.callback = NULL;
}

/**
 * @brief Destructor.
 */
FrameObserverSimple::~FrameObserverSimple() {
  Stop();
  Exit();
}

/**
 * @brief Setup and start callback sequence.
 * @param[in] (param) Start-up parameters.
 * @return Status object.
 */
Status FrameObserverSimple::Init(const SetupParameter& param) {
  if ((param.stream == NULL) || (param.callback == NULL)) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  util::AutoLock lock(&mutex_);
  param_ = param;
  return Status::OK();
}

/**
 * @brief Cancel callback.
 */
void FrameObserverSimple::Exit() {
  util::AutoLock lock(&mutex_);
  param_.callback = NULL;
}

/**
 * @brief Start receiving.
 * @return Status object.
 */
Status FrameObserverSimple::Start() {
  util::AutoLock lock(&mutex_);
  is_start_ = true;
  return Status::OK();
}

/**
 * @brief Stop receiving.
 * @return Status object.
 */
Status FrameObserverSimple::Stop() {
  util::AutoLock lock(&mutex_);
  is_start_ = false;
  return Status::OK();
}

/**
 * @brief Notify when frame arrived.
 * @return always success.
 */
Status FrameObserverSimple::Notify(const void* /* param */) {
  SetupParameter tmp;
  tmp.stream = NULL;
  tmp.callback = NULL;
  tmp.private_data = NULL;
  {
    util::AutoLock lock(&mutex_);
    if (is_start_) {
      tmp = param_;
    }
  }
  if (tmp.callback != NULL) {
    (*tmp.callback)(tmp.stream, tmp.private_data);
  }
  return Status::OK();
}

}  // namespace senscord
