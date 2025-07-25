/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "frame/frame_manager_core.h"

#include <inttypes.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <algorithm>    /* std::remove */
#include <list>

#include "stream/stream_core.h"
#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"
#include "logger/logger.h"
#include "util/mutex.h"
#include "util/autolock.h"
#include "frame/frame_core.h"

namespace senscord {

/**
 * @brief Constructor.
 */
FrameManagerCore::FrameManagerCore() {
  reserved_count_ = 0;
  stream_ = NULL;
  initialized_ = false;
  user_data_.data_address = 0;
  user_data_.data_size = 0;
  skip_rate_ = 1;
  skip_counter_ = 0;
}

/**
 * @brief Destructor.
 */
FrameManagerCore::~FrameManagerCore() {
  ClearUserData();
  ReleaseFrameOfQueue(&outgoing_queue_, false, NULL);
  ReleaseFrameOfQueue(&incoming_queue_, false, NULL);
}

/**
 * @brief Initialize FrameManager.
 * @param[in] (num) Number of frame.
 * @param[in] (stream) Parent stream.
 * @return Status object.
 */
Status FrameManagerCore::Init(int32_t num, StreamCore* stream) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  if (initialized_) {
    SENSCORD_LOG_DEBUG("already initialized");
    return Status::OK();
  }

  reserved_count_ = num;
  stream_ = stream;
  initialized_ = true;
  return Status::OK();
}

/**
 * @brief Terminate FrameManager.
 * @return Status object.
 */
Status FrameManagerCore::Exit() {
  if (!initialized_) {
    // not initialized or already exit
    return Status::OK();
  }
  // release frame of all
  ReleaseFrameOfQueue(&outgoing_queue_, true, NULL);
  ReleaseFrameOfQueue(&incoming_queue_, true, NULL);
  ClearUserData();
  initialized_ = false;
  return Status::OK();
}

/**
 * @brief Set new frame.
 * @param[in] (frameinfo) Arrived frame information.
 * @param[in] (sent_time) Time when frame was sent.
 * @return Status object.
 */
Status FrameManagerCore::Set(
    const FrameInfo& frameinfo, uint64_t sent_time) {
  if (!initialized_) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "uninitialized");
  }
  if (IsSkipFrame()) {
    SendFrameDropEvent(frameinfo);
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseCancelled, "dropped by skip frame property");
  }

  bool acquirable = false;
  FrameBuffer* frame_buffer = GetBuffer(&acquirable);
  if (frame_buffer == NULL) {
    SendFrameDropEvent(frameinfo);
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseResourceExhausted, "reserved frame is empty");
  }

  frame_buffer->frame = new FrameCore(stream_, frameinfo, sent_time);
  frame_buffer->rawdata_accessed = false;

  {
    util::AutoLock lock(&mutex_user_data_);
    if (user_data_.data_size > 0) {
      frame_buffer->frame->SetUserData(user_data_);
    }
  }
  {
    util::AutoLock lock(&mutex_channel_mask_);
    if (!masked_channels_.empty()) {
      frame_buffer->frame->SetChannelMask(masked_channels_);
    }
  }

  if (acquirable) {
    // frame notify to stream
    Status status = NotifyStream(frameinfo);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }
  return Status::OK();
}

/**
 * @brief Remove and ReleasePortFrame to Component.
 * @param[in] (frame) Frame to remove.
 * @param[in] (rawdata_accessed) Whether you have accessed raw data.
 * @return Status object.
 */
Status FrameManagerCore::Remove(const Frame* frame, bool rawdata_accessed) {
  if (frame == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  std::list<FrameBuffer>::iterator itr = outgoing_queue_.begin();
  std::list<FrameBuffer>::iterator end = outgoing_queue_.end();
  for (; itr != end; ++itr) {
    if (itr->frame == frame) {
      break;
    }
  }
  if (itr == end) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "not managed frame");
  }

  // check whether accessed
  FrameBuffer& frame_buffer = *itr;
  frame_buffer.rawdata_accessed = rawdata_accessed;
#ifdef SENSCORD_RECORDER
  frame_buffer.rawdata_accessed |= frame_buffer.frame->IsRecorded();
#endif  // SENSCORD_RECORDER

  // release
  Status status = ReleaseFrame(frame_buffer);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    delete frame_buffer.frame;
    outgoing_queue_.erase(itr);
    ++reserved_count_;
  }

  return status;
}

/**
 * @brief Get frame.
 * @param[out] (frame) Frame to be acquired.
 * @return Status object.
 */
Status FrameManagerCore::Get(Frame** frame) {
  if (frame == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  if (incoming_queue_.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseResourceExhausted, "queue is empty");
  }
  // pop oldest frame.
  FrameBuffer frame_buffer = incoming_queue_.front();
  incoming_queue_.pop_front();
  *frame = frame_buffer.frame;

  // push to using queue.
  outgoing_queue_.push_back(frame_buffer);
  return Status::OK();
}

/**
 * @brief Clear frame.
 * @param[out] (released_number) Number of release frame(optional).
 * @return Status object.
 */
Status FrameManagerCore::Clear(int32_t* released_number) {
  /* clear only incoming buffer.
     outgoing buffer is use by user, don't clear */
  ReleaseFrameOfQueue(&incoming_queue_, true, released_number);
  return Status::OK();
}

/**
 * @brief Get frame buffer info.
 * @param[out] (reserevd_num) reserved frame num(optional).
 * @param[out] (arrived_num) arrived frame num(optional).
 * @param[out] (received_num) received frame num(optional).
 * @return Status object.
 */
Status FrameManagerCore::GetFrameBufferInfo(int32_t* reserevd_num,
                                            int32_t* arrived_num,
                                            int32_t* received_num) {
  if (reserevd_num != NULL) {
    *reserevd_num = reserved_count_;
  }
  if (arrived_num != NULL) {
    *arrived_num = static_cast<int32_t>(incoming_queue_.size());
  }
  if (received_num != NULL) {
    *received_num = static_cast<int32_t>(outgoing_queue_.size());
  }
  return Status::OK();
}

/**
 * @brief Release frame.
 * @param[in] (frame_buffer) release to frame.
 * @return Status object.
 */
Status FrameManagerCore::ReleaseFrame(
    const FrameBuffer& frame_buffer) {
  std::vector<uint32_t> referenced_channel_ids;
  if (frame_buffer.rawdata_accessed) {
    // Get all unmasked channel IDs.
    const ChannelList& list = frame_buffer.frame->GetChannelList();
    for (ChannelList::const_iterator itr = list.begin(), end = list.end();
        itr != end; ++itr) {
      referenced_channel_ids.push_back(itr->first);
    }
  }

  Status status = stream_->ReleaseFrameInfo(
      frame_buffer.frame->GetFrameInfo(), referenced_channel_ids);
  SENSCORD_STATUS_TRACE(status);
  return status;
}

/**
 * @brief Set user data.
 * @param[in] (user_data) target user data.
 * @return Status object.
 */
Status FrameManagerCore::SetUserData(const FrameUserData& user_data) {
  util::AutoLock lock(&mutex_user_data_);
  // check the same user data
  if (user_data.data_size == user_data_.data_size) {
    if (osal::OSMemcmp(reinterpret_cast<const void*>(user_data.data_address),
        reinterpret_cast<const void*>(user_data_.data_address),
        user_data.data_size) == 0) {
      return Status::OK();
    }
  }

  // clear already set user data
  ClearUserData();

  // new setting
  Status status;
  if (user_data.data_size > 0) {
    user_data_.data_address =
        reinterpret_cast<uintptr_t>(osal::OSMalloc(user_data.data_size));
    if (user_data_.data_address != 0) {
      user_data_.data_size = user_data.data_size;
      int32_t ret = osal::OSMemcpy(
          reinterpret_cast<void*>(user_data_.data_address),
          user_data_.data_size,
          reinterpret_cast<void*>(user_data.data_address),
          user_data.data_size);
      if (ret < 0) {
        status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidArgument,
            "userdata copy (%" PRIxPTR " %" PRIuS ")",
            user_data.data_address, user_data.data_size);
      }
    } else {
      status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseResourceExhausted, "allocate failed");
    }
  }
  if (!status.ok()) {
    ClearUserData();
  }
  return status;
}

/**
 * @brief Get user data.
 * @param[out] (user_data) current user data.
 * @return Status object.
 */
Status FrameManagerCore::GetUserData(FrameUserData** user_data) {
  if (user_data == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  util::AutoLock lock(&mutex_user_data_);
  *user_data = &user_data_;
  return Status::OK();
}

/**
 * @brief Set the channel mask.
 * @param[in] (mask) new mask channels.
 * @return Status object.
 */
Status FrameManagerCore::SetChannelMask(const std::vector<uint32_t>& mask) {
  util::AutoLock lock(&mutex_channel_mask_);
  masked_channels_ = mask;
  return Status::OK();
}

/**
 * @brief Get the channel mask.
 * @param[out] (mask) current mask channels.
 * @return Status object.
 */
Status FrameManagerCore::GetChannelMask(std::vector<uint32_t>* mask) const {
  if (mask == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  util::AutoLock lock(&mutex_channel_mask_);
  *mask = masked_channels_;
  return Status::OK();
}

/**
 * @brief Set the new skip rate of the frame.
 * @param[in] (skip_rate) new skip rate.
 * @return Status object.
 */
Status FrameManagerCore::SetSkipRate(uint32_t skip_rate) {
  if (skip_rate == 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter: skip_rate = 0");
  }
  skip_rate_ = skip_rate;
  skip_counter_ = skip_rate - 1;  // Set not to skip the first frame.
  return Status::OK();
}

/**
 * @brief Get the skip rate of the frame.
 * @param[out] (skip_rate) current skip rate.
 * @return Status object.
 */
Status FrameManagerCore::GetSkipRate(uint32_t* skip_rate) const {
  if (skip_rate == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter: skip_rate is null");
  }
  *skip_rate = skip_rate_;
  return Status::OK();
}

/**
 * @brief Get whether the next frame is skipped.
 *        And update the counter.
 * @return True means that next frame must be skipped.
 */
bool FrameManagerCore::IsSkipFrame() {
  if (skip_rate_ <= 1) {
    return false;  // not skip.
  }
  bool is_skipped = true;
  ++skip_counter_;
  if (skip_counter_ >= skip_rate_) {
    skip_counter_ = 0;
    is_skipped = false;
  }
  return is_skipped;
}

/**
 * @brief Send frame drop event.
 * @param[in] (info) Target drop frame.
 * @return Status object.
 */
Status FrameManagerCore::SendFrameDropEvent(const FrameInfo& info) {
  EventArgument args;
  Status status = args.Set(kEventArgumentSequenceNumber, info.sequence_number);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  status = stream_->SendEvent(kEventFrameDropped, args);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Frame notify to stream core.
 * @param[in] (frameinfo) Information of arrived frame.
 * @return Status object.
 */
Status FrameManagerCore::NotifyStream(const FrameInfo& frameinfo) {
  Status status = stream_->FrameArrived(frameinfo);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Release frame of queue.
 * @param[in,out] (queue) Queue of release target.
 * @param[in] (call_release) Whether to call the release frame.
 * @param[out] (released_number) Number of release frame.
 */
void FrameManagerCore::ReleaseFrameOfQueue(
    std::list<FrameBuffer>* queue,
    bool call_release,
    int32_t* released_number) {
  int32_t num = 0;
  while (!queue->empty()) {
    FrameBuffer& frame_buffer = queue->back();
    if (call_release) {
      ReleaseFrame(frame_buffer);
    }
    delete frame_buffer.frame;
    queue->pop_back();
    ++num;
  }
  reserved_count_ += num;
  if (released_number != NULL) {
    *released_number = num;
  }
}

/**
 * @brief Clear user data.
 * @return Status object.
 */
Status FrameManagerCore::ClearUserData() {
  util::AutoLock lock(&mutex_user_data_);
  if (user_data_.data_address != 0) {
    osal::OSFree(reinterpret_cast<void*>(user_data_.data_address));
    user_data_.data_address = 0;
    user_data_.data_size = 0;
  }
  return Status::OK();
}

}    // namespace senscord
