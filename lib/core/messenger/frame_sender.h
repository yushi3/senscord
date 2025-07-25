/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_MESSENGER_FRAME_SENDER_H_
#define LIB_CORE_MESSENGER_FRAME_SENDER_H_

#include <vector>

#include "core/internal_types.h"
#include "messenger/messenger_topic.h"
#include "util/autolock.h"

namespace senscord {

class MessengerTopic;

/**
 * @brief Frame sender class.
 */
class FrameSender {
 public:
  /**
   * @brief Constructor.
   */
  explicit FrameSender(MessengerTopic* topic)
      : topic_(topic), mutex_(), state_(kFrameSenderCloseable) {
    mutex_ = new util::Mutex();
  }

  /**
   * @brief Destructor.
   */
  virtual ~FrameSender() {
    delete mutex_;
    mutex_ = NULL;
  }

  /**
   * @brief Open the frame sender.
   * @return Status object.
   */
  virtual Status Open() {
    return Status::OK();
  }

  /**
   * @brief Close the frame sender.
   * @return Status object.
   */
  virtual Status Close() {
    return Status::OK();
  }

  /**
   * @brief Publish frames to connected stream.
   * @param[in] (frames) List of frame information to send.
   * @param[out] (dropped_frames) List of pointer of dropped frames.
   * @return Status object.
   */
  virtual Status PublishFrames(
      const std::vector<FrameInfo>& frames,
      std::vector<const FrameInfo*>* dropped_frames) = 0;

  /**
   * @brief Release the frame from frame sender.
   * @param[in] (frameinfo) Informations to release frame.
   * @return Status object.
   */
  virtual Status ReleaseFrame(const FrameInfo& frameinfo) = 0;

  /**
   * @brief State of frame sender.
   */
  enum FrameSenderState {
    kFrameSenderRunning = 0,
    kFrameSenderCloseable,
  };

  /**
   * @brief Set frame sender state.
   * @param[in] (state) State.
   */
  void SetState(FrameSenderState state) {
    util::AutoLock lock(mutex_);
    state_ = state;
  }

  /**
   * @brief Get frame sender state.
   * @return State of frame sender.
   */
  FrameSenderState GetState() const {
    util::AutoLock lock(mutex_);
    return state_;
  }

  /**
   * @brief Get topic.
   * @return The topic is linked to this frame sender.
   */
  MessengerTopic* GetTopic() const {
    return topic_;
  }

 protected:
  MessengerTopic* topic_;
  util::Mutex* mutex_;

 private:
  FrameSenderState state_;
};

}   // namespace senscord
#endif  // LIB_CORE_MESSENGER_FRAME_SENDER_H_
