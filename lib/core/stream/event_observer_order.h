/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_STREAM_EVENT_OBSERVER_ORDER_H_
#define LIB_CORE_STREAM_EVENT_OBSERVER_ORDER_H_

#include <stdint.h>

#include "senscord/osal.h"
#include "stream/event_observer.h"
#include "util/mutex.h"

namespace senscord {

/**
 * @brief Event receiving observer class. For the sequential ordered calling.
 */
class EventObserverOrder : public EventObserver {
 public:
  /**
   * @brief Setup callback sequence.
   * @param[in] (param) Setup parameters.
   * @return Status object.
   */
  virtual Status Init(const SetupParameter& param);

  /**
   * @brief Cancel callback.
   * @return Status object.
   */
  virtual Status Exit();

  /**
   * @brief Notify when event arrived.
   * @param[in] (param) Arrived EventInfo.
   * @return Status object.
   */
  virtual Status Notify(const void* param);

  /**
   * @brief Constructor.
   */
  EventObserverOrder();

  /**
   * @brief Destructor.
   */
  ~EventObserverOrder();

 private:
  util::Mutex mutex_;
  osal::OSThread* thread_;
  void* active_param_;
};

}  // namespace senscord

#endif  // LIB_CORE_STREAM_EVENT_OBSERVER_ORDER_H_
