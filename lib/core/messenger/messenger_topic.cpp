/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "messenger/messenger_topic.h"

#include <map>
#include <string>
#include <set>
#include <vector>
#include <utility>  // for make_pair

#include "messenger/messenger_manager.h"
#include "messenger/inner_frame_sender.h"

#ifdef SENSCORD_SERVER
#include "messenger/server_frame_sender.h"
#endif

// for debug
#define SENSCORD_MESSENGER_DEBUG
#ifdef SENSCORD_MESSENGER_DEBUG
#include <sstream>
#endif  // SENSCORD_MESSENGER_DEBUG

namespace senscord {

/**
 * @brief Constructor
 */
MessengerTopic::MessengerTopic(const std::string& name)
    : name_(name), history_book_(), current_id_(),
      records_mutex_(), latest_seq_num_() {
  history_book_ = new PropertyHistoryBook();
  records_mutex_ = new util::Mutex();
}

/**
 * @brief Destructor
 */
MessengerTopic::~MessengerTopic() {
  {
    // release all resource
    util::AutoLock lock(records_mutex_);
    while (!records_.empty()) {
      Records::iterator itr = records_.begin();
      ResourceRecord* record = &(itr->second);
      delete record->publisher;
      delete record->frame_sender;
      records_.erase(itr);
    }
  }
  delete records_mutex_;
  records_mutex_ = NULL;
  delete history_book_;
  history_book_ = NULL;
}

/**
 * @brief Publish frames to connected stream.
 * @param[in] (frames) The publish frames.
 * @return Status object.
 */
Status MessengerTopic::PublishFrames(
    PublisherCore* publisher, const std::vector<FrameInfo>& frames) {
  ResourceRecord* record = NULL;
  {
    util::AutoLock lock(records_mutex_);
    for (Records::iterator itr = records_.begin(),
        end = records_.end(); itr != end; ++itr) {
      if (itr->second.publisher == publisher) {
        record = &(itr->second);
        break;
      }
    }
    if (record == NULL) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseNotFound,
          "unmanaged publisher=%p", publisher);
    }
    // overwrite sequence_number
    for (std::vector<FrameInfo>::const_iterator
        itr = frames.begin(), end = frames.end(); itr != end; ++itr) {
      FrameInfo* frameinfo = const_cast<FrameInfo*>(&(*itr));
      frameinfo->sequence_number = latest_seq_num_;
      record->sent_frames.insert(frameinfo->sequence_number);
      ++latest_seq_num_;
    }
    PrintRecords("pub");
  }

  // send frames
  std::vector<const FrameInfo*> drop_frames;
  Status status = record->frame_sender->PublishFrames(frames, &drop_frames);
  if (!status.ok()) {
    SENSCORD_LOG_DEBUG(
        "failed to send frames: %s", status.ToString().c_str());
  }
  if (drop_frames.size() > 0) {
    for (std::vector<const FrameInfo*>::const_iterator
        itr = drop_frames.begin(), end = drop_frames.end();
        itr != end; ++itr) {
      if (*itr != NULL) {
        {
          util::AutoLock lock(records_mutex_);
          record->sent_frames.erase((*itr)->sequence_number);
        }
        publisher->ReleaseFrame(*(*itr));
      }
    }
  }
  return Status::OK();
}

/**
 * @brief Release the frame from stream.
 * @param[in] (frameinfo) Informations to release frame.
 * @return Status object.
 */
Status MessengerTopic::ReleaseFrame(const FrameInfo& frameinfo) {
  ResourceRecord* record = NULL;
  std::set<uint64_t>::const_iterator found;
  {
    util::AutoLock lock(records_mutex_);
    for (Records::iterator itr = records_.begin(),
        end = records_.end(); itr != end; ++itr) {
      found = itr->second.sent_frames.find(frameinfo.sequence_number);
      if (found != itr->second.sent_frames.end()) {
        record = &(itr->second);
        itr->second.sent_frames.erase(found);
        PrintRecords("rel");
        break;
      }
    }
  }
  if (record) {
    record->publisher->ReleaseFrame(frameinfo);
    if (IsReleaseableRecord(record)) {
      NotifyReleaseResource();
    }
    return Status::OK();
  }
  return SENSCORD_STATUS_FAIL(
      kStatusBlockCore, Status::kCauseNotFound,
      "unmanaged frame: seqnum=%" PRIu64, frameinfo.sequence_number);
}

/**
 * @brief Returns whether this resource is referenced.
 * @return True is referenced.
 */
bool MessengerTopic::IsReferenced() {
  util::AutoLock publock(records_mutex_);
  return !records_.empty();
}

/**
 * @brief Get publisher.
 * @param[in] (required_server) True is required connect server.
 * @return The publisher pointer.
 */
PublisherCore* MessengerTopic::GetPublisher(bool required_server) {
  ResourceRecord* record = GetCurrentRecord(required_server);
  return record->publisher;
}

/**
 * @brief Release publisher.
 * @param[in] (publisher) The publisher pointer.
 */
void MessengerTopic::ReleasePublisher(PublisherCore* publisher) {
  bool is_releaseable = false;
  {
    util::AutoLock lock(records_mutex_);
    for (Records::iterator itr = records_.begin(),
        end = records_.end(); itr != end; ++itr) {
      if (itr->second.publisher == publisher) {
        if (IsReleaseableRecord(&(itr->second))) {
          is_releaseable = true;
        }
        break;
      }
    }
    PrintRecords("rpb");
  }
  if (is_releaseable) {
    NotifyReleaseResource();
  }
}

/**
 * @brief Get frame sender.
 * @param[in] (required_server) True is required connect server.
 * @return The frame sender pointer.
 */
FrameSender* MessengerTopic::GetFrameSender(bool required_server) {
  ResourceRecord* record = GetCurrentRecord(required_server);
  return record->frame_sender;
}

/**
 * @brief Release frame sender.
 * @param[in] (frame_sender) The frame sender pointer.
 */
void MessengerTopic::ReleaseFrameSender(FrameSender* frame_sender) {
  bool is_releaseable = false;
  {
    util::AutoLock lock(records_mutex_);
    for (Records::iterator itr = records_.begin(),
        end = records_.end(); itr != end; ++itr) {
      if (itr->second.frame_sender == frame_sender) {
        if (IsReleaseableRecord(&(itr->second))) {
          is_releaseable = true;
        }
        break;
      }
    }
    PrintRecords("rfs");
  }
  if (is_releaseable) {
    NotifyReleaseResource();
  }
}

/**
 * @brief Notify the MessengerManager of resource release.
 * @param[in] (publisher) The publisher pointer.
 * @param[in] (frame_sender) The frame sender pointer.
 */
void MessengerTopic::NotifyReleaseResource() {
  MessengerManager* manager = MessengerManager::GetInstance();
  manager->ReleaseResources(this);
}

/**
 * @brief Release unreferenced records.
 */
void MessengerTopic::ReleaseUnreferencedResource() {
  util::AutoLock lock(records_mutex_);
  for (Records::iterator itr = records_.begin(), end = records_.end();
      itr != end;) {
    ResourceRecord* record = &(itr->second);
    if (IsReleaseableRecord(record)) {
      record->frame_sender->Close();
      delete record->frame_sender;
      record->frame_sender = NULL;
      delete record->publisher;
      record->publisher = NULL;
      records_.erase(itr++);
    } else {
      ++itr;
    }
  }
  PrintRecords("del");
}

/**
 * @brief Check to see if record is releasable.
 * @param[in] (record) Target record.
 * @return True is releaseable.
 */
bool MessengerTopic::IsReleaseableRecord(ResourceRecord* record) {
  util::AutoLock lock(records_mutex_);
  PublisherCore::PublisherState state = record->publisher->GetState();
  return (
      (state == PublisherCore::kPublisherStateInit ||
      state == PublisherCore::kPublisherStateClose) &&
      record->frame_sender->GetState() == FrameSender::kFrameSenderCloseable &&
      record->sent_frames.empty());
}

/**
 * @brief Get current record.
 * @param[in] (required_server) True is required connect server.
 * @return Current record.
 */
MessengerTopic::ResourceRecord* MessengerTopic::GetCurrentRecord(
    bool required_server) {
  bool is_releaseable = false;
  ResourceRecord* new_record = NULL;
  {
    util::AutoLock lock(records_mutex_);
    ResourceRecord& current_record = records_[current_id_];
    if (current_record.publisher == NULL) {
      // at first
      current_record.publisher = new PublisherCore(this);
#ifdef SENSCORD_SERVER
      if (required_server) {
        current_record.frame_sender = new ServerFrameSender(this);
      } else {
        current_record.frame_sender = new InnerFrameSender(this);
      }
#else
      current_record.frame_sender = new InnerFrameSender(this);
#endif  // SENSCORD_SERVER
      PrintRecords("add");
      return &current_record;
    }

    // exist record
    PublisherCore::PublisherState state = current_record.publisher->GetState();
    if (state == PublisherCore::kPublisherStateInit ||
        state == PublisherCore::kPublisherStateOpen) {
      return &current_record;
    }

    // create new record
    ++current_id_;
    new_record = &(records_[current_id_]);
    new_record->publisher = new PublisherCore(this);
#ifdef SENSCORD_SERVER
    if (required_server) {
      new_record->frame_sender = new ServerFrameSender(
            this, static_cast<ServerFrameSender*>(current_record.frame_sender));
    } else {
      new_record->frame_sender = new InnerFrameSender(
            this, static_cast<InnerFrameSender*>(current_record.frame_sender));
    }
#else
    new_record->frame_sender = new InnerFrameSender(
          this, static_cast<InnerFrameSender*>(current_record.frame_sender));
#endif  // SENSCORD_SERVER
    if (IsReleaseableRecord(&current_record)) {
      is_releaseable = true;
    }
    PrintRecords("add");
  }
  if (is_releaseable) {
    NotifyReleaseResource();
  }
  return new_record;
}

/**
 * @brief Print records info (for debug, need lock).
 * @param[in] (type) Control type.
 */
void MessengerTopic::PrintRecords(const std::string& type) {
#ifdef SENSCORD_MESSENGER_DEBUG
  std::stringstream ss;
  for (senscord::MessengerTopic::Records::const_iterator
      itr = records_.begin(), end = records_.end(); itr != end; ++itr) {
    ss << "[id:" << itr->first;
    ss << ",p:" << itr->second.publisher->GetState();
    ss << ",f:" << itr->second.frame_sender->GetState();
    ss << ",s:" << itr->second.sent_frames.size() << "(";
    for (std::set<uint64_t>::const_iterator
        f_itr = itr->second.sent_frames.begin(),
        f_end = itr->second.sent_frames.end(); f_itr != f_end; ++f_itr) {
      ss << *f_itr << ",";
    }
    ss << ")], ";
  }
  SENSCORD_LOG_DEBUG("Records(%s:%s): %s",
      name_.c_str(), type.c_str(),
      ss.str().empty() ? "(empty)" : ss.str().c_str());
#endif  // SENSCORD_MESSENGER_DEBUG
}

}   // namespace senscord
