/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "senscord/osal.h"
#include "senscord/senscord.h"
#include "senscord/synchronizer/synchronizer.h"

// SyncPolicyApi impl
Synchronizer::SyncPolicyApiCore::SyncPolicyApiCore(Synchronizer *synchronizer)
    : parent_(synchronizer) {}

Synchronizer::SyncPolicyApiCore::~SyncPolicyApiCore() { parent_ = NULL; }

void Synchronizer::SyncPolicyApiCore::SendSyncFrame(const SyncFrame &frame) {
  if (frame.empty()) {
    SYNCHRONIZER_LOG_WARNING("[SendSyncFrame] sync frame is empty.");
    return;
  }

  senscord::osal::OSLockMutex(parent_->syncframe_queue_mutex_);

  if (parent_->syncframe_queue_.size() >= parent_->syncframe_queue_num_) {
    // drop old SyncFrame
    SyncFrame to_delete = parent_->syncframe_queue_.at(0);
    parent_->syncframe_queue_.pop_front();

    senscord::osal::OSUnlockMutex(parent_->syncframe_queue_mutex_);

    parent_->ReleaseSyncFrame(&to_delete);

    SYNCHRONIZER_LOG_INFO("SyncFrameQueue is full, old SyncFrame is dropped.");
    RaiseEvent(kSynchronizerEvent, senscord::kEventFrameDropped, NULL);

    senscord::osal::OSLockMutex(parent_->syncframe_queue_mutex_);
  }

  parent_->syncframe_queue_.push_back(frame);
  senscord::osal::OSBroadcastCond(parent_->syncframe_queue_enqueued_cond_);

  senscord::osal::OSUnlockMutex(parent_->syncframe_queue_mutex_);
}

void Synchronizer::SyncPolicyApiCore::DropFrame(
    senscord::Stream *stream, senscord::Frame *frame, bool drop_frame_event) {
  if ((stream == NULL) || (frame == NULL)) {
    SYNCHRONIZER_LOG_ERROR("[DropFrame] invalid pointer");
    return;
  }

#ifdef SYNCHRONIZER_DEBUG_DROP_FRAME
  uint64_t ts;
  GetTimeStamp(frame, stream, &ts);
  SYNCHRONIZER_LOG_DEBUG("[DropFrame] stream=%p frame=%p ts=" NS_PRINT
                         " evt=%d",
      stream, frame, NS_PRINT_ARG(ts), drop_frame_event);
#endif

  stream->ReleaseFrameUnused(frame);

  if (drop_frame_event) {
    RaiseEvent(stream, senscord::kEventFrameDropped, NULL);
  }
}

void Synchronizer::SyncPolicyApiCore::RaiseEvent(senscord::Stream *stream,
    const std::string &event_type, const void *param) {
  senscord::osal::OSLockMutex(parent_->event_queue_mutex_);

  parent_->event_queue_.emplace_back(
      EventQueueEntry(stream, event_type, param));
  senscord::osal::OSBroadcastCond(parent_->event_queue_cond_);

  senscord::osal::OSUnlockMutex(parent_->event_queue_mutex_);
}

void Synchronizer::SyncPolicyApiCore::SetProcessConfig(
    uint64_t polling_offset, uint64_t polling_period, bool apply_immediate) {
  senscord::osal::OSLockMutex(parent_->process_frame_config_mutex_);

  parent_->process_frame_config_.period = polling_period;
  parent_->process_frame_config_.offset = polling_offset;

  if (apply_immediate) {
    senscord::osal::OSBroadcastCond(parent_->process_frame_config_cond_);
  }

  senscord::osal::OSUnlockMutex(parent_->process_frame_config_mutex_);
}

void Synchronizer::SyncPolicyApiCore::GetSourceStreamList(
    std::vector<SyncStreamInfo> *stream_list) {
  if (stream_list) {
    stream_list->clear();

    // copy list
    (*stream_list) = parent_->stream_list_;
  }
}

bool Synchronizer::SyncPolicyApiCore::GetTimeStamp(
    senscord::Frame *frame, senscord::Stream *stream, uint64_t *timestamp) {
  senscord::Status status;

#ifndef USE_SENTTIME
  senscord::Channel *channel = NULL;
  senscord::Channel::RawData raw = {};

  status = frame->GetChannel(
      parent_->stream_map_[stream]->main_channel_id, &channel);
  if (!status.ok()) {
    SENSCORD_LOG_ERROR("[GetTimeStamp] Frame::GetChannel failed. status = %s",
        status.ToString().c_str());
    return false;
  }

  status = channel->GetRawData(&raw);
  if (!status.ok()) {
    SENSCORD_LOG_ERROR("[GetTimeStamp] Channel::GetRawData failed. status = %s",
        status.ToString().c_str());
    return false;
  }

  *timestamp = raw.timestamp;

  return true;

#else
  status = frame->GetSentTime(timestamp);
  if (!status.ok()) return false;

  return true;
#endif
}

std::string SyncPolicyApi::ToString(senscord::Stream *stream) {
  if (stream == NULL) {
    return std::string("(null)");
  }

  senscord::StreamKeyProperty key;
  senscord::Status status =
      stream->GetProperty(senscord::kStreamKeyPropertyKey, &key);

  std::ostringstream ret;
  if (status.ok()) {
    ret << key.stream_key << "(0x" << std::hex << stream << ")";
    return ret.str();

  } else {
    ret << "(0x" << std::hex << stream << ")";
    return ret.str();
  }
}
