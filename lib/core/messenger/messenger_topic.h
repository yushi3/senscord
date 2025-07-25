/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_MESSENGER_MESSENGER_TOPIC_H_
#define LIB_CORE_MESSENGER_MESSENGER_TOPIC_H_

#include <vector>
#include <string>
#include <map>
#include <set>

#include "core/internal_types.h"
#include "stream/property_history_book.h"
#include "messenger/publisher_core.h"
#include "messenger/frame_sender.h"
#include "util/autolock.h"

namespace senscord {

// pre-define
class PublisherCore;
class FrameSender;

/**
 * @brief Messenger topic class.
 */
class MessengerTopic : private util::Noncopyable {
 public:
  /**
   * @brief Constructor
   */
  explicit MessengerTopic(const std::string& name);

  /**
   * @brief Destructor
   */
  ~MessengerTopic();

  /**
   * @brief Publish frames to connected frame sender.
   * @param[in] (frames) The publish frames.
   * @return Status object.
   */
  Status PublishFrames(
      PublisherCore* publisher, const std::vector<FrameInfo>& frames);

  /**
   * @brief Release the frame from frame sender.
   * @param[in] (frameinfo) Informations to release frame.
   * @return Status object.
   */
  Status ReleaseFrame(const FrameInfo& frameinfo);

  /**
   * @brief Returns whether this resource is referenced.
   * @return True is referenced.
   */
  bool IsReferenced();

  /**
   * @brief Get publisher.
   * @return The publisher pointer.
   */
  PublisherCore* GetPublisher(bool required_server);

  /**
   * @brief Set publisher.
   * @param[in] (publisher) The publisher pointer.
   */
  void ReleasePublisher(PublisherCore* publisher);

  /**
   * @brief Get frame sender.
   * @return The frame sender pointer.
   */
  FrameSender* GetFrameSender(bool required_server);

  /**
   * @brief Release frame sender.
   * @param[in] (frame_sender) The frame sender pointer.
   */
  void ReleaseFrameSender(FrameSender* frame_sender);

  /**
   * @brief Get property history book.
   * @return The property history book.
   */
  PropertyHistoryBook* GetPropertyHistoryBook() const {
    return history_book_;
  }

  /**
   * @brief Release unreferenced records.
   */
  void ReleaseUnreferencedResource();

 private:
  /**
   * @brief Resource record.
   */
  struct ResourceRecord {
    PublisherCore* publisher;
    FrameSender* frame_sender;
    std::set<uint64_t> sent_frames;
  };

  /**
   * @brief Check to see if record is releasable.
   * @param[in] (record) Target record.
   * @return True is releaseable.
   */
  bool IsReleaseableRecord(ResourceRecord* record);

  /**
   * @brief Get current record.
   * @param[in] (required_server) True is required connect server.
   * @return Current record.
   */
  ResourceRecord* GetCurrentRecord(bool required_server);

  /**
   * @brief Notify the MessengerManager of resource release.
   */
  void NotifyReleaseResource();

  // key is id
  typedef std::map<uint32_t, ResourceRecord> Records;

  /**
   * @brief Print records info (for debug, need lock).
   * @param[in] (type) Control type.
   */
  void PrintRecords(const std::string& type);

  std::string name_;  // topic name
  PropertyHistoryBook* history_book_;

  Records records_;
  uint32_t current_id_;
  util::Mutex* records_mutex_;

  uint64_t latest_seq_num_;
};

}   // namespace senscord
#endif  // LIB_CORE_MESSENGER_MESSENGER_TOPIC_H_
