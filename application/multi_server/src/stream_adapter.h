/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APPLICATION_MULTI_SERVER_SRC_STREAM_ADAPTER_H_
#define APPLICATION_MULTI_SERVER_SRC_STREAM_ADAPTER_H_

#include <string>
#include <vector>
#include <list>
#include <map>
#include "senscord/osal.h"
#include "senscord/stream.h"
#include "senscord/frame.h"
#include "senscord/connection_types.h"
#include "senscord/connection.h"
#include "resource_adapter.h"

namespace senscord {
namespace server {

class ClientAdapter;

/**
 * @brief The adapter class to access to the stream.
 */
class StreamAdapter : public ResourceAdapter {
 public:
  /**
   * @brief Close to the stream adapter.
   * @param[in] (core) Core object.
   * @return Status object.
   */
  virtual Status Close(Core* core);

  /**
   * @brief Start to access to the stream.
   * @return Status object.
   */
  virtual Status StartMonitoring();

  /**
   * @brief Stop to access to the stream.
   * @return Status object.
   */
  virtual Status StopMonitoring();

  /**
   * @brief The method to monitor new messages.
   */
  virtual void Monitoring(MonitorType type);

  /**
   * @brief Push new message for this stream.
   * @param[in] (msg) New incoming message instance.
   */
  virtual void PushMessage(const Message* msg);

  /**
   * @brief Publish the frame if arrived.
   */
  void PublishingFrame();

  /**
   * @brief Publish the event.
   * @param[in] (event_type) The type of event.
   * @param[in] (args) Event argument.
   */
  void PublishingEvent(
      const std::string& event_type, const EventArgument& args);

  /**
   * @brief Constructor.
   * @param[in] (stream) The pointer of stream.
   * @param[in] (client) The adapter to client.
   */
  explicit StreamAdapter(Stream* stream, ClientAdapter* client);

  /**
   * @brief Destructor.
   */
  ~StreamAdapter();

 private:
  typedef std::list<const Message*> MessageList;

  /**
   * Information for ReleaseFrame.
   */
  struct ReleaseFrameInfo {
    Frame* frame;
    bool rawdata_accessed;
  };

  /**
   * @brief Pop the latest messgae. if not incoming, wait forever.
   * @return The latest message.
   */
  const Message* PopMessage(MessageList* messages);

  /**
   * @brief Do the calling function to stream each message types.
   * @param[in] (msg) The message for doing.
   */
  void DoMessage(const Message& msg);

  /**
   * @brief Start stream.
   * @param[in] (msg) The message for doing.
   * @return The status of send the reply.
   */
  Status Start(const Message& msg);

  /**
   * @brief Stop stream.
   * @param[in] (msg) The message for doing.
   * @return The status of send the reply.
   */
  Status Stop(const Message& msg);

  /**
   * @brief Get the property list from stream.
   * @param[in] (msg) The message for doing.
   * @return The status of send the reply.
   */
  Status GetPropertyList(const Message& msg);

  /**
   * @brief Get the property from stream.
   * @param[in] (msg) The message for doing.
   * @return The status of send the reply.
   */
  Status GetProperty(const Message& msg);

  /**
   * @brief Set the property from stream.
   * @param[in] (msg) The message for doing.
   * @return The status of send the reply.
   */
  Status SetProperty(const Message& msg);

  /**
   * @brief Lock the property from stream.
   * @param[in] (msg) The message for doing.
   * @return The status of send the reply.
   */
  Status LockProperty(const Message& msg);

  /**
   * @brief Unlock the property from stream.
   * @param[in] (msg) The message for doing.
   * @return The status of send the reply.
   */
  Status UnlockProperty(const Message& msg);

  /**
   * @brief Release the frame to the stream.
   * @param[in] (msg) The message for doing.
   * @return The status of send the reply.
   */
  Status ReleaseFrame(const Message& msg);

  /**
   * @brief Release of frame by reply message.
   * @param[in] (msg) The message for doing.
   * @return Status object.
   */
  Status ReleaseFrameBySendFrameReply(const Message& msg);

  /**
   * @brief Get multiple frames from the stream.
   * @param[in] (max_number) The maximum number of frames to get.
   * @param[out] (frames) List of frames.
   */
  void GetFrames(size_t max_number, std::vector<Frame*>* frames);

  /**
   * @brief Send the message about new frame to client.
   * @param[in] (frames) List of frames to send.
   * @return Status object.
   */
  Status SendFrames(const std::vector<Frame*>& frames);

  /**
   * @brief Release multiple frames.
   * @param[in] (frames) List of frames to release.
   */
  void ReleaseFrames(const std::vector<ReleaseFrameInfo>& frames);

  /**
   * @brief Release the frame to the stream.
   * @param[in] (frame) The frame gotten.
   * @param[in] (is_rawdata_accessed) Whether the raw data has been accessed.
   * @return Status object of ReleaseFrame();
   */
  Status ReleaseFrameCore(Frame* frame, bool is_rawdata_accessed);

  /**
   * @brief Get the message data payload from the frame.
   * @param[out] (dest) The message data location.
   * @param[in] (src) The frame gotten from stream.
   * @param[out] (is_pending_release) Flag for pending release frame.
   * @param[out] (is_rawdata_accessed) Whether the raw data has been accessed.
   * @return Status object.
   */
  Status GetMessageDataForSendFrame(
      MessageDataFrameLocalMemory* dest, const Frame* src,
      bool* is_pending_release, bool* is_rawdata_accessed) const;

  /**
   * Get the message data payload from multiple frames.
   * @param[in] (src) List of frames.
   * @param[out] (dest) The message data location.
   * @param[out] (pending_list) List containing pending frames.
   * @param[out] (release_list) List containing release frames.
   */
  void GetMessageDataForSendFrames(
      const std::vector<Frame*>& src, MessageDataSendFrame* dest,
      std::vector<ReleaseFrameInfo>* pending_list,
      std::vector<ReleaseFrameInfo>* release_list) const;

  /**
   * @brief Push the frame to release pending list.
   * @param[in] (pending_frame) Pending frame.
   */
  void PushPendingReleaseFrame(const ReleaseFrameInfo& pending_frame);

  /**
   * @brief Pop the frame from release pending list.
   * @param[in] (sequence_number) Sequence number of frame.
   * @return Pending frame.
   */
  ReleaseFrameInfo PopPendingReleaseFrame(uint64_t sequence_number);

  /**
   * @brief Register event to the stream.
   * @param[in] (msg) The message for doing.
   * @return The status of send the reply.
   */
  Status RegisterEvent(const Message& msg);

  /**
   * @brief Unregister event to the stream.
   * @param[in] (msg) The message for doing.
   * @return The status of send the reply.
   */
  Status UnregisterEvent(const Message& msg);

  Stream* stream_;
  ClientAdapter* client_;

  MessageList messages_;
  MessageList messages_lock_property_;    // Lock/UnlockProperty request only

  typedef std::vector<osal::OSThread*> ThreadList;
  ThreadList threads_;
  osal::OSThread* thread_lock_property_;

  osal::OSMutex* messaging_mutex_;
  osal::OSCond* messaging_cond_;
  bool end_flag_;

  uint64_t event_send_count_;

  typedef std::map<uint64_t, ReleaseFrameInfo> PendingFrameMap;
  PendingFrameMap pending_release_frames_;
  osal::OSMutex* pending_frames_mutex_;
};

}  // namespace server
}  // namespace senscord

#endif  // APPLICATION_MULTI_SERVER_SRC_STREAM_ADAPTER_H_
