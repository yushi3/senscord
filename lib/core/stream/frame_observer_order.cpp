/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stream/frame_observer_order.h"

#include <inttypes.h>

#include "logger/logger.h"
#include "util/autolock.h"

namespace senscord {

/**
 * @brief Thread parameter.
 */
struct OrderFrameThreadParameter {
  util::Mutex mutex;
  osal::OSCond* cond;
  bool is_start;
  uint32_t callback_count;
  FrameObserver::SetupParameter* param;
};

/**
 * @brief Notify Working thread.
 * @param[in] (arg) Thread parameter instance.
 * @return 0 means success or negative value means failed (error code).
 */
static osal::OSThreadResult WorkerThreadOrderFrame(void* arg);

/**
 * @brief Notify callback.
 * @param[in] (param) Thread parameter instance.
 */
static void NotifyCallbackOrderFrame(OrderFrameThreadParameter* param);

/**
 * @brief Delete parameter for order thread.
 * @param[in] (param) Thread parameter instance.
 */
static void DeleteThreadParamOrderFrame(OrderFrameThreadParameter* param);

/**
 * @brief Constructor.
 */
FrameObserverOrder::FrameObserverOrder() :
    thread_(NULL), active_param_(NULL) {}

/**
 * @brief Destructor.
 */
FrameObserverOrder::~FrameObserverOrder() {
  Stop();
  Exit();
}

/**
 * @brief Setup callback sequence.
 * @param[in] (param) Setup parameters.
 * @return Status object.
 */
Status FrameObserverOrder::Init(const SetupParameter& param) {
  if ((param.stream == NULL) || (param.callback == NULL)) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  setup_param_ = param;
  return Status::OK();
}

/**
 * @brief Cancel callback.
 */
void FrameObserverOrder::Exit() {
  // do nothing.
}

/**
 * @brief Start receiving.
 * @return Status object.
 */
Status FrameObserverOrder::Start() {
  util::AutoLock lock(&mutex_);
  if (thread_) {
    return Status::OK();
  }

  OrderFrameThreadParameter* worker_param = new OrderFrameThreadParameter();
  worker_param->param = new SetupParameter(setup_param_);
  worker_param->is_start = true;
  worker_param->callback_count = 0;
  osal::OSCreateCond(&worker_param->cond);

  int32_t ret = osal::OSCreateThread(&thread_, WorkerThreadOrderFrame,
      worker_param, NULL);
  Status status;
  if (ret < 0) {
    DeleteThreadParamOrderFrame(worker_param);
    thread_ = NULL;
    status = SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "CreateThread failed: 0x%" PRIx32, ret);
  } else {
    active_param_ = worker_param;
  }
  return status;
}

/**
 * @brief Stop receiving.
 * @return Status object.
 */
Status FrameObserverOrder::Stop() {
  util::AutoLock lock(&mutex_);
  if (thread_ && active_param_) {
    // wakeup
    {
      OrderFrameThreadParameter* worker_param =
          reinterpret_cast<OrderFrameThreadParameter*>(active_param_);
      util::AutoLock worker_lock(&worker_param->mutex);
      worker_param->is_start = false;
      worker_param->callback_count = 0;
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
 * @brief Notify when frame arrived.
 * @return always success.
 */
Status FrameObserverOrder::Notify(const void* /* param */) {
  util::AutoLock lock(&mutex_);
  if (thread_ && active_param_) {
    OrderFrameThreadParameter* worker_param =
        reinterpret_cast<OrderFrameThreadParameter*>(active_param_);
    util::AutoLock worker_lock(&worker_param->mutex);
    if (worker_param->is_start) {
      ++worker_param->callback_count;
      osal::OSSignalCond(worker_param->cond);
    }
  }
  return Status::OK();
}

/**
 * @brief Notify callback.
 * @param[in] (param) Thread parameter instance.
 */
static void NotifyCallbackOrderFrame(OrderFrameThreadParameter* param) {
  util::AutoLock worker_lock(&param->mutex);
  while (param->is_start) {
    if (param->callback_count > 0) {
      --param->callback_count;

      FrameObserver::SetupParameter tmp = *param->param;
      param->mutex.Unlock();
      if (tmp.callback != NULL) {
        (*tmp.callback)(tmp.stream, tmp.private_data);
      }
      param->mutex.Lock();
    } else {
      osal::OSWaitCond(param->cond, param->mutex.GetObject());
    }
  }
}

/**
 * @brief Notify Working thread.
 * @param[in] (arg) Thread parameter instance.
 * @return 0 means success or negative value means failed (error code).
 */
static osal::OSThreadResult WorkerThreadOrderFrame(void* arg) {
  OrderFrameThreadParameter* param =
      reinterpret_cast<OrderFrameThreadParameter*>(arg);
  if (param == NULL) {
    return static_cast<osal::OSThreadResult>(-1);
  }
  NotifyCallbackOrderFrame(param);
  DeleteThreadParamOrderFrame(param);
  return static_cast<osal::OSThreadResult>(0);
}

/**
 * @brief Delete parameter for order thread.
 * @param[in] (param) Thread parameter instance.
 */
static void DeleteThreadParamOrderFrame(OrderFrameThreadParameter* param) {
  if (param) {
    osal::OSDestroyCond(param->cond);
    delete param->param;
    delete param;
  }
}

}  // namespace senscord
