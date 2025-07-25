/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stream/event_observer_simple.h"
#include "logger/logger.h"
#include "core/internal_types.h"
#include "util/autolock.h"

namespace senscord {

/**
 * @brief Constructor.
 */
EventObserverSimple::EventObserverSimple() {
  param_.callback = NULL;
}

/**
 * @brief Destructor.
 */
EventObserverSimple::~EventObserverSimple() {
  Exit();
}

/**
 * @brief Setup callback sequence.
 * @param[in] (param) Setup parameters.
 * @return Status object.
 */
Status EventObserverSimple::Init(const SetupParameter& param) {
  if ((param.callback == NULL) || (param.event_type.empty())) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  util::AutoLock lock(&mutex_);
  param_ = param;
  return Status::OK();
}

/**
 * @brief Cancel callback.
 * @return Status object.
 */
Status EventObserverSimple::Exit() {
  util::AutoLock lock(&mutex_);
  param_.callback = NULL;
  return Status::OK();
}

/**
 * @brief Notify when event arrived.
 * @param[in] (param) Arrived EventInfo.
 * @return Status object.
 */
Status EventObserverSimple::Notify(const void* param) {
  const EventInfo* event = reinterpret_cast<const EventInfo*>(param);
  if (event == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "received event is null");
  }

  SetupParameter tmp;
  tmp.callback = NULL;
  {
    util::AutoLock lock(&mutex_);
    tmp = param_;
  }
  if ((tmp.callback != NULL) &&
      ((tmp.event_type == kEventAny) || (event->type == tmp.event_type))) {
    (*tmp.callback)(event->type, event->argument, tmp.private_data);
  }
  return Status::OK();
}

}    // namespace senscord
