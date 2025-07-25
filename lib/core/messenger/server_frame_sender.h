/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_MESSENGER_SERVER_FRAME_SENDER_H_
#define LIB_CORE_MESSENGER_SERVER_FRAME_SENDER_H_

#include <vector>
#include <map>
#include <string>

#include "senscord/config.h"

#ifdef SENSCORD_SERVER
#include "messenger/frame_sender.h"
#include "core/internal_types.h"
#include "senscord/develop/client_messenger.h"
#include "messenger/messenger_topic.h"
#include "stream/stream_core.h"

namespace senscord {

// pre-define
class MessengerTopic;

/**
 * @brief Server frame sender class.
 */
class ServerFrameSender : public FrameSender {
 public:
  /**
   * @brief Constructor.
   * @param[in] (topic) Parent messenger topic.
   */
  explicit ServerFrameSender(MessengerTopic* topic);

  /**
   * @brief Constructor (assigning ownership).
   * @param[in] (topic) Parent messenger topic.
   * @param[in] (old) Old server frame sender.
   */
  explicit ServerFrameSender(MessengerTopic* topic, ServerFrameSender* old);

  /**
   * @brief Destructor
   */
  virtual ~ServerFrameSender();

  /**
   * @brief Open the frame sender.
   * @param[in] (key) Publisher key.
   * @param[in] (arguments) Connection parameters.
   * @return Status object.
   */
  Status Open(
      const std::string& key,
      const std::map<std::string, std::string>& arguments);

  /**
   * @brief Close the publisher.
   * @return Status object.
   */
  virtual Status Close();

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
   * @brief Release frames.
   * @param[in] (frameinfo) frame to release.
   */
  virtual Status ReleaseFrame(const FrameInfo& frameinfo);

  /**
   * @brief Push sending meesage.
   * @param[in] (msg) Message.
   * @return Status object.
   */
  Status PushSendingsMessage(Message* msg);

 private:
  /**
   * @brief Connect to server.
   * @param[in] (connection) Connection type.
   * @param[in] (address) Server address.
   * @param[in] (address_secondary) Server secondary address.
   * @return Status object.
   */
  Status Connect(
      const std::string& connection,
      const std::string& address,
      const std::string& address_secondary);

  /**
   * @brief Disconnect to server.
   * @return Status object.
   */
  Status Disconnect();

  /**
   * Get the message data payload from multiple frames.
   * @param[in] (src) List of frames.
   * @param[out] (dest) The message data location.
   * @param[out] (pending_list) List containing pending frames.
   * @param[out] (release_list) List containing release frames.
   * @return Status object.
   */
  void GetMessageDataForSendFrames(
      const std::vector<FrameInfo>& src,
      MessageDataSendFrame* dest,
      std::vector<const FrameInfo*>* pending_list,
      std::vector<const FrameInfo*>* release_list) const;

  /**
   * @brief Get the message data payload from the frame.
   * @param[out] (dest) The message data location.
   * @param[in] (src) The frame gotten from stream.
   * @param[out] (is_pending_release) Flag for pending release frame.
   * @return Status object.
   */
  Status GetMessageDataForSendFrame(
      MessageDataFrameLocalMemory* dest, const FrameInfo& src,
      bool* is_pending_release) const;

  /**
   * @brief Push the frame to release pending list.
   * @param[in] (pending_frame) Pending frame.
   */
  void PushPendingReleaseFrame(const FrameInfo& pending_frame);

  /**
   * @brief Pop the frame from release pending list.
   * @param[in] (sequence_number) Sequence number of frame.
   * @return Pending frame.
   */
  FrameInfo PopPendingReleaseFrame(uint64_t sequence_number);

  /**
   * @brief Release of frame by send frame reply message.
   * @param[in] (msg) The message for doing.
   * @return Status object.
   */
  Status ReleaseFrameBySendFrameReply(const Message& msg);

  /**
   * @brief Release of frame by request message.
   * @param[in] (msg) The message for doing.
   * @return Status object.
   */
  Status ReleaseFrameByReleaseFrameRequest(const Message& msg);

  std::string key_;
  uint64_t reply_timeout_nsec_;
  util::Mutex* pending_frames_mutex_;
  typedef std::map<uint64_t, FrameInfo> PendingFrameMap;
  PendingFrameMap pending_release_frames_;
  ClientMessenger* messenger_;
  Stream* dummy_stream_;
};

}   // namespace senscord

#endif  // SENSCORD_SERVER

#endif  // LIB_CORE_MESSENGER_SERVER_FRAME_SENDER_H_
