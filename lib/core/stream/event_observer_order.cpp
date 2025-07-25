/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stream/event_observer_order.h"

#include <inttypes.h>
#include <vector>

#include "logger/logger.h"
#include "core/internal_types.h"
#include "util/autolock.h"

namespace senscord {

/**
 * @brief Thread parameter.
 */
struct OrderEventThreadParameter {
  util::Mutex mutex;
  osal::OSCond* cond;
  bool is_start;
  std::vector<EventInfo> events;
  EventObserver::SetupParameter* param;
};

/**
 * @brief Notify Working thread.
 * @param[in] (arg) Thread parameter instance.
 * @return 0 means success or negative value means failed (error code).
 */
static osal::OSThreadResult WorkerThreadOrderEvent(void* arg);

/**
 * @brief Notify callback.
 * @param[in] (param) Thread parameter instance.
 */
static void NotifyCallbackOrderEvent(OrderEventThreadParameter* param);

/**
 * @brief Delete parameter for order thread.
 * @param[in] (param) Thread parameter instance.
 */
static void DeleteThreadParamOrderEvent(OrderEventThreadParameter* param);

/**
 * @brief Constructor.
 */
EventObserverOrder::EventObserverOrder() :
    thread_(NULL), active_param_(NULL) {}

/**
 * @brief Destructor.
 */
EventObserverOrder::~EventObserverOrder() {
  Exit();
}

/**
 * @brief Setup callback sequence.
 * @param[in] (param) Setup parameters.
 * @return Status object.
 */
Status EventObserverOrder::Init(const SetupParameter& param) {
  if ((param.stream == NULL) ||
      (param.callback == NULL && param.callback_old == NULL) ||
      (param.event_type.empty())) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  util::AutoLock lock(&mutex_);
  if (thread_ || active_param_) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "already started");
  }

  OrderEventThreadParameter* worker_param = new OrderEventThreadParameter();
  worker_param->param = new SetupParameter(param);
  worker_param->is_start = true;
  osal::OSCreateCond(&worker_param->cond);

  int32_t ret = osal::OSCreateThread(&thread_, WorkerThreadOrderEvent,
      worker_param, NULL);
  Status status;
  if (ret < 0) {
    DeleteThreadParamOrderEvent(worker_param);
    thread_ = NULL;
    status = SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "CreateThread failed: 0x%" PRIx32, ret);
  } else {
    active_param_ = worker_param;
  }
  return status;
}

/**
 * @brief Cancel callback.
 * @return Status object.
 */
Status EventObserverOrder::Exit() {
  util::AutoLock lock(&mutex_);
  if (thread_ && active_param_) {
    // wakeup
    {
      OrderEventThreadParameter* worker_param =
          reinterpret_cast<OrderEventThreadParameter*>(active_param_);
      util::AutoLock param_lock(&worker_param->mutex);
      worker_param->is_start = false;
      worker_param->events.clear();
      worker_param->param->callback = NULL;
      worker_param->param->callback_old = NULL;
      osal::OSSignalCond(worker_param->cond);
    }

    // wait to finish worker
    int32_t ret = osal::OSJoinThread(thread_, NULL);
    if (ret < 0) {
      SENSCORD_LOG_WARNING("Stop has done "
          "but JoinThread the callback failed (0x%" PRIx32 ")", ret);
      osal::OSDetachThread(thread_);  // for resolving memory leak
    }
    thread_ = NULL;
    active_param_ = NULL;
  }
  return Status::OK();
}

/**
 * @brief Notify when event arrived.
 * @param[in] (param) Arrived EventInfo.
 * @return Status object.
 */
Status EventObserverOrder::Notify(const void* param) {
  const EventInfo* event = reinterpret_cast<const EventInfo*>(param);
  if (event == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "received event is null");
  }

  util::AutoLock lock(&mutex_);
  if (thread_ && active_param_) {
    OrderEventThreadParameter* worker_param =
        reinterpret_cast<OrderEventThreadParameter*>(active_param_);

    util::AutoLock worker_lock(&worker_param->mutex);
    if ((worker_param->is_start) &&
        ((worker_param->param->event_type == kEventAny) ||
         (event->type == worker_param->param->event_type))) {
      worker_param->events.push_back(*event);
      osal::OSSignalCond(worker_param->cond);
    }
  }
  return Status::OK();
}

/**
 * @brief Notify Working thread.
 * @param[in] (arg) Thread parameter instance.
 * @return 0 means success or negative value means failed (error code).
 */
static osal::OSThreadResult WorkerThreadOrderEvent(void* arg) {
  OrderEventThreadParameter* worker_param =
      reinterpret_cast<OrderEventThreadParameter*>(arg);
  if (worker_param == NULL) {
    return static_cast<osal::OSThreadResult>(-1);
  }
  NotifyCallbackOrderEvent(worker_param);
  DeleteThreadParamOrderEvent(worker_param);
  return static_cast<osal::OSThreadResult>(0);
}

/**
 * @brief Notify callback.
 * @param[in] (param) Thread parameter instance.
 */
static void NotifyCallbackOrderEvent(OrderEventThreadParameter* param) {
  util::AutoLock lock(&param->mutex);
  while (param->is_start) {
    std::vector<EventInfo>::iterator itr = param->events.begin();
    if (itr != param->events.end()) {
      // pop event
      EventInfo event = (*itr);
      param->events.erase(itr);

      // run callback
      EventObserver::SetupParameter tmp = *param->param;
      param->mutex.Unlock();
      if (tmp.callback != NULL) {
        (*tmp.callback)(
            tmp.stream, event.type, event.argument, tmp.private_data);
      } else if (tmp.callback_old != NULL) {
        (*tmp.callback_old)(event.type, NULL, tmp.private_data);
      }
      param->mutex.Lock();
    } else {
      osal::OSWaitCond(param->cond, param->mutex.GetObject());
    }
  }
}

/**
 * @brief Delete parameter for order thread.
 * @param[in] (param) Thread parameter instance.
 */
static void DeleteThreadParamOrderEvent(OrderEventThreadParameter* param) {
  if (param) {
    osal::OSDestroyCond(param->cond);
    delete param->param;
    delete param;
  }
}

}    // namespace senscord
