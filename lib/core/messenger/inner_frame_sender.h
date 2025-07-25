/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_MESSENGER_INNER_FRAME_SENDER_H_
#define LIB_CORE_MESSENGER_INNER_FRAME_SENDER_H_

#include <vector>

#include "messenger/frame_sender.h"
#include "core/internal_types.h"
#include "component/component_port_core.h"

namespace senscord {

/**
 * @brief Inner frame sender class.
 */
class InnerFrameSender : public FrameSender {
 public:
  /**
   * @brief Constructor.
   * @param[in] (topic) Parent messenger topic.
   */
  explicit InnerFrameSender(MessengerTopic* topic);

  /**
   * @brief Constructor (assigning ownership).
   * @param[in] (topic) Parent messenger topic.
   * @param[in] (old) Old inner frame sender.
   */
  explicit InnerFrameSender(MessengerTopic* topic, InnerFrameSender* old);

  /**
   * @brief Destructor
   */
  virtual ~InnerFrameSender();

  /**
   * @brief Publish frames to connected stream.
   * @param[in] (frames) List of frame information to send.
   * @param[out] (dropped_frames) List of pointer of dropped frames.
   * @return Status object.
   */
  virtual Status PublishFrames(
      const std::vector<FrameInfo>& frames,
      std::vector<const FrameInfo*>* dropped_frames);

  /**
   * @brief Release the frame from frame sender.
   * @param[in] (frameinfo) Informations to release frame.
   * @return Status object.
   */
  virtual Status ReleaseFrame(const FrameInfo& frameinfo);

  /**
   * @brief Set component port core.
   * @param[in] (port) Component port core pointer.
   */
  void SetPort(ComponentPortCore* port);

  /**
   * @brief Get property history book.
   * @return The property history book.
   */
  PropertyHistoryBook* GetPropertyHistoryBook() const;

 private:
  ComponentPortCore* port_;
};

}   // namespace senscord
#endif  // LIB_CORE_MESSENGER_INNER_FRAME_SENDER_H_
