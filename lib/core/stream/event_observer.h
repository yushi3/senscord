/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_STREAM_EVENT_OBSERVER_H_
#define LIB_CORE_STREAM_EVENT_OBSERVER_H_

#include <stdint.h>
#include <string>

#include "senscord/stream.h"
#include "util/observer.h"

namespace senscord {

/**
 * @brief Event receiving observer interface class.
 */
class EventObserver : public util::Observer {
 public:
  /**
   * @brief Callback setup parameters.
   */
  struct SetupParameter {
    /** Parent stream. */
    Stream* stream;

    /** Event type. */
    std::string event_type;

    /**< Callback function pointer. */
    Stream::OnEventReceivedCallback callback;

    /**< Callback function pointer. (old) */
    Stream::OnEventReceivedCallbackOld callback_old;

    /**< Private data. */
    void* private_data;
  };

  /**
   * @brief Setup callback sequence.
   * @param[in] (param) Setup parameters.
   * @return Status object.
   */
  virtual Status Init(const SetupParameter& param) = 0;

  /**
   * @brief Cancel callback.
   * @return Status object.
   */
  virtual Status Exit() = 0;

  /**
   * @brief Notify when frame arrived.
   * @param[in] (param) Arrived FrameInfo.
   * @return Status object.
   */
  virtual Status Notify(const void* param) = 0;

  /**
   * @brief Destructor.
   */
  virtual ~EventObserver() {}

 protected:
  /**
   * @brief Constructor.
   */
  EventObserver() {}
};

}  // namespace senscord

#endif  // LIB_CORE_STREAM_EVENT_OBSERVER_H_
