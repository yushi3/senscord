/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "messenger/inner_frame_sender.h"

namespace senscord {

/**
 * @brief Constructor
 * @param[in] (topic) Parent messenger topic.
 */
InnerFrameSender::InnerFrameSender(MessengerTopic* topic)
    : FrameSender(topic), port_() {}

/**
 * @brief Constructor (assigning ownership).
 * @param[in] (topic) Parent messenger topic.
 * @param[in] (old) Old inner frame sender.
 */
InnerFrameSender::InnerFrameSender(
    MessengerTopic* topic, InnerFrameSender* old)
    : FrameSender(topic), port_() {
  ComponentPortCore* port = NULL;
  {
    util::AutoLock lock(old->mutex_);
    port = old->port_;
    old->SetState(kFrameSenderCloseable);
  }
  util::AutoLock lock(mutex_);
  port_ = port;
  SetState(kFrameSenderRunning);
}

/**
 * @brief Destructor
 */
InnerFrameSender::~InnerFrameSender() {}

/**
 * @brief Publish frames to connected stream.
 * @param[in] (frames) List of frame information to send.
 * @param[out] (dropped_frames) List of pointer of dropped frames.
 * @return Status object.
 */
Status InnerFrameSender::PublishFrames(
    const std::vector<FrameInfo>& frames,
    std::vector<const FrameInfo*>* dropped_frames) {
  Status status;
  if (port_) {
    status = port_->SendFrames(frames, dropped_frames);
    SENSCORD_STATUS_TRACE(status);
  } else {
    // all frame drop
    for (std::vector<FrameInfo>::const_iterator itr = frames.begin(),
        end = frames.end(); itr != end; ++itr) {
      dropped_frames->push_back(&(*itr));
    }
    status = SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseNotFound, "unconnected port");
  }
  return status;
}

/**
 * @brief Release the frame from frame sender.
 * @param[in] (frameinfo) Informations to release frame.
 * @return Status object.
 */
Status InnerFrameSender::ReleaseFrame(const FrameInfo& frameinfo) {
  return SENSCORD_STATUS_TRACE(topic_->ReleaseFrame(frameinfo));
}

/**
 * @brief Set component port core.
 * @param[in] (port) Component port core pointer.
 */
void InnerFrameSender::SetPort(ComponentPortCore* port) {
  util::AutoLock lock(mutex_);
  port_ = port;
  if (port_) {
    SetState(kFrameSenderRunning);
  } else {
    SetState(kFrameSenderCloseable);
  }
}

/**
 * @brief Get property history book.
 * @return The property history book.
 */
PropertyHistoryBook* InnerFrameSender::GetPropertyHistoryBook() const {
  return topic_->GetPropertyHistoryBook();
}

}   // namespace senscord
