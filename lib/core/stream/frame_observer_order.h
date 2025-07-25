/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_STREAM_FRAME_OBSERVER_ORDER_H_
#define LIB_CORE_STREAM_FRAME_OBSERVER_ORDER_H_

#include <stdint.h>

#include "senscord/osal.h"
#include "stream/frame_observer.h"
#include "util/mutex.h"

namespace senscord {

/**
 * @brief Frame receiving observer class. For the sequential ordered calling.
 */
class FrameObserverOrder : public FrameObserver {
 public:
  /**
   * @brief Setup callback sequence.
   * @param[in] (param) Setup parameters.
   * @return Status object.
   */
  virtual Status Init(const SetupParameter& param);

  /**
   * @brief Cancel callback.
   */
  virtual void Exit();

  /**
   * @brief Start receiving.
   * @return Status object.
   */
  virtual Status Start();

  /**
   * @brief Stop receiving.
   * @return Status object.
   */
  virtual Status Stop();

  /**
   * @brief Notify when frame arrived.
   * @return always success.
   */
  virtual Status Notify(const void* /* param */);

  /**
   * @brief Constructor.
   */
  FrameObserverOrder();

  /**
   * @brief Destructor.
   */
  ~FrameObserverOrder();

 private:
  util::Mutex mutex_;
  osal::OSThread* thread_;
  void* active_param_;
  SetupParameter setup_param_;
};

}  // namespace senscord
#endif  // LIB_CORE_STREAM_FRAME_OBSERVER_ORDER_H_
