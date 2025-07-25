/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_STREAM_FRAME_OBSERVER_H_
#define LIB_CORE_STREAM_FRAME_OBSERVER_H_

#include <stdint.h>

#include "senscord/stream.h"
#include "util/observer.h"

namespace senscord {

/**
 * @brief Frame receiving observer interface class.
 */
class FrameObserver : public util::Observer {
 public:
  /**
   * @brief Callback setup parameters.
   */
  struct SetupParameter {
    /** Parent stream. */
    Stream* stream;

    /**< Callback function pointer. */
    Stream::OnFrameReceivedCallback callback;

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
   */
  virtual void Exit() = 0;

  /**
   * @brief Start receiving.
   * @return Status object.
   */
  virtual Status Start() = 0;

  /**
   * @brief Stop receiving.
   * @return Status object.
   */
  virtual Status Stop() = 0;

  /**
   * @brief Notify when frame arrived.
   * @param[in] (param) Arrived FrameInfo.
   * @return Status object.
   */
  virtual Status Notify(const void* param) = 0;

  /**
   * @brief Destructor.
   */
  virtual ~FrameObserver() {}

 protected:
  /**
   * @brief Constructor.
   */
  FrameObserver() {}
};

}  // namespace senscord
#endif  // LIB_CORE_STREAM_FRAME_OBSERVER_H_
