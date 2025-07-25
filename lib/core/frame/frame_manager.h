/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_FRAME_FRAME_MANAGER_H_
#define LIB_CORE_FRAME_FRAME_MANAGER_H_

#include <stdint.h>
#include <vector>
#include "senscord/frame.h"
#include "senscord/noncopyable.h"
#include "senscord/property_types.h"
#include "stream/stream_core.h"

namespace senscord {

// pre-definition
class StreamCore;

/**
 * @brief Manages Frame
 */
class FrameManager : private util::Noncopyable {
 public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~FrameManager() {}

  /**
   * @brief Initialize.
   * @param (num) num of frame.
   * @param (stream) parent stream.
   * @return Status object.
   */
  virtual Status Init(int32_t num, StreamCore* stream) = 0;

  /**
   * @brief Terminate FrameManager.
   * @return Status object.
   */
  virtual Status Exit() = 0;

  /**
   * @brief Get frame.
   * @param (frame) return frame.
   * @return Status object.
   */
  virtual Status Get(Frame** frame) = 0;

  /**
   * @brief Remove and ReleaseFrame to Component.
   * @param[in] (frame) frame of interest.
   * @param[in] (rawdata_accessed) Whether you have accessed raw data.
   * @return Status object.
   */
  virtual Status Remove(const Frame* frame, bool rawdata_accessed) = 0;

  /**
   * @brief Clear frame.
   * @param[out] (released_number) Number of release frame(optional).
   * @return Status object.
   */
  virtual Status Clear(int32_t* released_number) = 0;

  /**
   * @brief Get frame buffer info.
   * @param (reserevd_num) reserved frame num.
   * @param (arrived_num) arrived frame num.
   * @param (received_num) received frame num.
   * @return Status object.
   */
  virtual Status GetFrameBufferInfo(int32_t* reserevd_num,
                                    int32_t* arrived_num,
                                    int32_t* received_num) = 0;

  /**
   * @brief Set new frame.
   * @param[in] (frameinfo) Arrived frame information.
   * @param[in] (sent_time) Time when frame was sent.
   * @return Status object.
   */
  virtual Status Set(const FrameInfo& frameinfo, uint64_t sent_time) = 0;

  /**
   * @brief Set user data.
   * @param[in] (user_data) target user data.
   * @return Status object.
   */
  virtual Status SetUserData(const FrameUserData& user_data) = 0;

  /**
   * @brief Get user data.
   * @param[out] (user_data) current user data.
   * @return Status object.
   */
  virtual Status GetUserData(FrameUserData** user_data) = 0;

  /**
   * @brief Set the channel mask.
   * @param[in] (mask) new mask channels.
   * @return Status object.
   */
  virtual Status SetChannelMask(const std::vector<uint32_t>& mask) = 0;

  /**
   * @brief Get the channel mask.
   * @param[out] (mask) current mask channels.
   * @return Status object.
   */
  virtual Status GetChannelMask(std::vector<uint32_t>* mask) const = 0;

  /**
   * @brief Set the new skip rate of the frame.
   * @param[in] (skip_rate) new skip rate.
   * @return Status object.
   */
  virtual Status SetSkipRate(uint32_t skip_rate) = 0;

  /**
   * @brief Get the skip rate of the frame.
   * @param[out] (skip_rate) current skip rate.
   * @return Status object.
   */
  virtual Status GetSkipRate(uint32_t* skip_rate) const = 0;
};

}   // namespace senscord

#endif  // LIB_CORE_FRAME_FRAME_MANAGER_H_
