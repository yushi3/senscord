/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_FRAME_FRAME_MANAGER_CORE_H_
#define LIB_CORE_FRAME_FRAME_MANAGER_CORE_H_

#include <stdint.h>
#include <vector>
#include <string>
#include <list>

#include "senscord/frame.h"
#include "senscord/property_types.h"
#include "senscord/develop/common_types.h"
#include "frame/frame_manager.h"
#include "stream/stream_core.h"
#include "frame/frame_core.h"
#include "util/mutex.h"

namespace senscord {

/**
 * @brief Frame buffer.
 */
struct FrameBuffer {
  FrameCore* frame;
  bool rawdata_accessed;
};

// pre-definition
class StreamCore;

/**
 * @brief Manages Frame
 */
class FrameManagerCore : public FrameManager {
 public:
  /**
   * @brief Destructor.
   */
  virtual ~FrameManagerCore();

  /**
   * @brief Initialize FrameManager.
   * @param[in] (num) Number of frame.
   * @param[in] (stream) Parent stream.
   * @return Status object.
   */
  virtual Status Init(int32_t num, StreamCore* stream);

  /**
   * @brief Terminate FrameManager.
   * @return Status object.
   */
  virtual Status Exit();

  /**
   * @brief Get frame.
   * @param[out] (frame) Frame to be acquired.
   * @return Status object.
   */
  virtual Status Get(Frame** frame);

  /**
   * @brief Remove and ReleasePortFrame to Component.
   * @param[in] (frame) Frame to remove.
   * @param[in] (rawdata_accessed) Whether you have accessed raw data.
   * @return Status object.
   */
  virtual Status Remove(const Frame* frame, bool rawdata_accessed);

  /**
   * @brief Clear frame.
   * @param[out] (released_number) Number of release frame(optional).
   * @return Status object.
   */
  virtual Status Clear(int32_t* released_number);

  /**
   * @brief Get frame buffer info.
   * @param[out] (reserevd_num) reserved frame num(optional).
   * @param[out] (arrived_num) arrived frame num(optional).
   * @param[out] (received_num) received frame num(optional).
   * @return Status object.
   */
  virtual Status GetFrameBufferInfo(int32_t* reserevd_num,
                                    int32_t* arrived_num,
                                    int32_t* received_num);

  /**
   * @brief Set new frame.
   * @param[in] (frameinfo) Arrived frame information.
   * @param[in] (sent_time) Time when frame was sent.
   * @return Status object.
   */
  virtual Status Set(const FrameInfo& frameinfo, uint64_t sent_time);

  /**
   * @brief Set user data.
   * @param[in] (user_data) target user data.
   * @return Status object.
   */
  virtual Status SetUserData(const FrameUserData& user_data);

  /**
   * @brief Get user data.
   * @param[out] (user_data) current user data.
   * @return Status object.
   */
  virtual Status GetUserData(FrameUserData** user_data);

  /**
   * @brief Set the channel mask.
   * @param[in] (mask) new mask channels.
   * @return Status object.
   */
  virtual Status SetChannelMask(const std::vector<uint32_t>& mask);

  /**
   * @brief Get the channel mask.
   * @param[out] (mask) current mask channels.
   * @return Status object.
   */
  virtual Status GetChannelMask(std::vector<uint32_t>* mask) const;

  /**
   * @brief Set the new skip rate of the frame.
   * @param[in] (skip_rate) new skip rate.
   * @return Status object.
   */
  virtual Status SetSkipRate(uint32_t skip_rate);

  /**
   * @brief Get the skip rate of the frame.
   * @param[out] (skip_rate) current skip rate.
   * @return Status object.
   */
  virtual Status GetSkipRate(uint32_t* skip_rate) const;

 protected:
  /**
   * @brief Constructor.
   */
  FrameManagerCore();

  /**
   * @brief Gets frame buffer.
   * @param[out] (acquirable) Flag of acquirable frame added.
   * @return Frame buffer pointer.
   */
  virtual FrameBuffer* GetBuffer(bool* acquirable) = 0;

  /**
   * @brief Release frame.
   * @param[in] (frame_buffer) release to frame.
   * @return Status object.
   */
  Status ReleaseFrame(const FrameBuffer& frame_buffer);

  /**
   * @brief Send frame drop event.
   * @param[in] (info) Target drop frame.
   * @return Status object.
   */
  Status SendFrameDropEvent(const FrameInfo& info);

 private:
  /**
   * @brief Frame notify to stream core.
   * @param[in] (frameinfo) Information of arrived frame.
   * @return Status object.
   */
  Status NotifyStream(const FrameInfo& frameinfo);

  /**
   * @brief Release frame of queue.
   * @param[in,out] (queue) Queue of release target.
   * @param[in] (call_release) Whether to call the release frame.
   * @param[out] (released_number) Number of release frame.
   */
  void ReleaseFrameOfQueue(
      std::list<FrameBuffer>* queue,
      bool call_release,
      int32_t* released_number);

  /**
   * @brief Clear user data.
   * @return Status object.
   */
  Status ClearUserData();

  /**
   * @brief Get whether the next frame is skipped.
   *        And update the counter.
   * @return True means that next frame must be skipped.
   */
  bool IsSkipFrame();

 protected:
  // frame queue
  int32_t reserved_count_;
  std::list<FrameBuffer> incoming_queue_;
  std::list<FrameBuffer> outgoing_queue_;

 private:
  // parent stream
  StreamCore* stream_;

  // been initialized
  bool initialized_;

  // user data
  FrameUserData user_data_;

  // mutex for user data
  mutable util::Mutex mutex_user_data_;

  // the list of masked channel IDs
  std::vector<uint32_t> masked_channels_;

  // mutex for channel mask
  mutable util::Mutex mutex_channel_mask_;

  // skip rate of the frame.
  uint32_t skip_rate_;

  uint32_t skip_counter_;
};

}   // namespace senscord
#endif  // LIB_CORE_FRAME_FRAME_MANAGER_CORE_H_
