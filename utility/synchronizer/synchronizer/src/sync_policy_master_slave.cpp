/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

const char kStatusBlockSyncPolicyMasterSlave[] = "SyncPolicyMasterSlave";

#include "senscord/synchronizer/sync_policy_master_slave.h"

#include <algorithm>
#include <utility>
#include <vector>

SyncPolicyMasterSlave::SyncPolicyMasterSlave() : initialized_(false) {}
SyncPolicyMasterSlave::~SyncPolicyMasterSlave() {
  if (initialized_) {
    Exit();
  }
}

senscord::Status SyncPolicyMasterSlave::Init(uint64_t time_range,
    uint64_t additional_wait, bool overwrite_timestamp,
    bool oneframe_per_stream, bool wait_all_stream_on_start) {
  SYNCPOLICY_MS_LOG_DEBUG("Init();");

  if (initialized_) {
    return SENSCORD_STATUS_FAIL(kStatusBlockSyncPolicyMasterSlave,
        senscord::Status::kCauseInvalidOperation, "aleady initialized.");
  }

  policy_api_ = NULL;
  time_range_ = time_range;
  additional_wait_ = additional_wait;
  overwrite_timestamp_ = overwrite_timestamp;
  oneframe_per_stream_ = oneframe_per_stream;
  wait_all_stream_on_start_ = wait_all_stream_on_start;
  master_stream_ = NULL;
  initialized_ = true;

  master_framerate_.num = 0;
  master_framerate_.denom = 0;

  senscord::osal::OSCreateMutex(&frame_queue_mutex_);

  return senscord::Status::OK();
}

senscord::Status SyncPolicyMasterSlave::Exit() {
  if (frame_queue_mutex_) {
    senscord::osal::OSDestroyMutex(frame_queue_mutex_);
    frame_queue_mutex_ = NULL;
  }

  initialized_ = false;

  return senscord::Status::OK();
}

senscord::Status SyncPolicyMasterSlave::Start(SyncPolicyApi *policy_api) {
  senscord::Status status;
  SYNCPOLICY_MS_LOG_DEBUG("Start();");
  policy_api_ = policy_api;

  // copy stream list from Synchronizer
  policy_api_->GetSourceStreamList(&stream_list_);
  if (stream_list_.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockSyncPolicyMasterSlave,
        senscord::Status::kCauseUnknown, "source stream is empty.");
  }
  master_stream_ = stream_list_.front().instance;

  SYNCPOLICY_MS_LOG_INFO("[Start] MasterStream: %s",
      SyncPolicyApi::ToString(master_stream_).c_str());

  // create stream map and print streams
  stream_key_map_.clear();
  for (auto &sf : stream_list_) {
    if (sf.instance == NULL) {
      SYNCPOLICY_MS_LOG_WARNING("[Start] null stream found in SyncStreamInfo");
      continue;
    }

    stream_map_.insert(std::make_pair(sf.instance, &sf));

    senscord::StreamKeyProperty key;
    status = sf.instance->GetProperty(senscord::kStreamKeyPropertyKey, &key);

    stream_key_map_.insert(std::make_pair(sf.instance, key.stream_key));

    if (status.ok()) {
      SYNCPOLICY_MS_LOG_INFO("[Start]  stream:%s(0x%p) main_ch:%d evt_dis:%s",
          key.stream_key.c_str(), sf.instance, sf.main_channel_id,
          sf.disabled_event_type.c_str());
    }
  }

  // init frame queue
  frame_queue_.clear();

  status = master_stream_->GetProperty(
      senscord::kFrameRatePropertyKey, &master_framerate_);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  uint64_t now = 0;
  senscord::osal::OSGetTime(&now);

  polling_period_ = 1000000000ULL *
                    static_cast<uint64_t>(master_framerate_.denom) /
                    static_cast<uint64_t>(master_framerate_.num);
  polling_offset_ = 0;
  pending_process_frame_nsec_ = 0;

  policy_api_->SetProcessConfig(polling_offset_, polling_period_, false);

  SYNCPOLICY_MS_LOG_DEBUG("[Start][config] time_range: " NS_PRINT_LONG
                          ", addtional_time: " NS_PRINT_LONG,
      NS_PRINT_LONG_ARG(time_range_), NS_PRINT_LONG_ARG(additional_wait_));
  SYNCPOLICY_MS_LOG_DEBUG(
      "[Start][config] overwrite_timestamp_: %d, oneframe_per_stream_: %d, "
      "wait_all_stream_on_start_: %d",
      overwrite_timestamp_, oneframe_per_stream_, wait_all_stream_on_start_);

  is_stream_active_ = !wait_all_stream_on_start_;
  is_stream_active_map_.clear();

  if (wait_all_stream_on_start_) {
    SYNCPOLICY_MS_LOG_INFO(
        "[wait_all_stream_on_start] option is enabled. some frames will be "
        "blocked.");
  }

  return senscord::Status::OK();
}

senscord::Status SyncPolicyMasterSlave::Stop() {
  SYNCPOLICY_MS_LOG_DEBUG("[Stop]");

  senscord::osal::OSLockMutex(frame_queue_mutex_);
  for (auto &stream_frames_pair : frame_queue_) {
    SYNCPOLICY_MS_LOG_DEBUG("[Stop] unused frame : %d frames [%s] ",
        stream_frames_pair.second.size(),
        stream_key_map_[stream_frames_pair.first].c_str());

    for (auto frameinfo : stream_frames_pair.second) {
#ifdef SYNCPOLICY_MS_UNUSED_FRAME_DUMP
      uint64_t ts = 0;
      policy_api_->GetTimeStamp(frameinfo.frame, stream_frames_pair.first, &ts);
      SYNCPOLICY_MS_LOG_DEBUG("[Stop] unused frame : %p " NS_PRINT,
          frameinfo.frame, NS_PRINT_ARG(ts));
#endif

      stream_frames_pair.first->ReleaseFrame(frameinfo.frame);
    }
  }
  frame_queue_.clear();
  senscord::osal::OSUnlockMutex(frame_queue_mutex_);

  return senscord::Status::OK();
}

void SyncPolicyMasterSlave::EnterSourceFrame(senscord::Stream *stream,
    const std::vector<senscord::Frame *> &source_frames) {
  uint64_t now = 0;
  senscord::osal::OSGetTime(&now);

  // enqueue source frame to policy local queue
  senscord::osal::OSLockMutex(frame_queue_mutex_);

  for (auto src_frame : source_frames) {
    // set receive time(now) to FrameInfo.timestamp temporally.
    frame_queue_[stream].emplace_back(SynchronizerFrameInfo(src_frame, now));
  }

  senscord::osal::OSUnlockMutex(frame_queue_mutex_);

  if (!is_stream_active_) {
    is_stream_active_map_[stream] = true;

    if (is_stream_active_map_.size() == stream_list_.size()) {
      is_stream_active_ = true;

      SYNCPOLICY_MS_LOG_INFO(
          "[wait_all_stream_on_start] all stream has been activated. "
          "frame blocking is released. time:" NS_PRINT,
          NS_PRINT_ARG(now));

    } else {
      if (stream == master_stream_) {
        // drop master frame until is_stream_active=false
        senscord::osal::OSLockMutex(frame_queue_mutex_);

        senscord::Frame *new_master_frame = frame_queue_[stream].front().frame;
        frame_queue_[stream].pop_front();

        stream->ReleaseFrameUnused(new_master_frame);

        senscord::osal::OSUnlockMutex(frame_queue_mutex_);

        SYNCPOLICY_MS_LOG_DEBUG("[is_stream_active_] master drop");
      }
    }

    return;
  }

  if (stream == master_stream_ && pending_process_frame_nsec_ == 0) {
    UpdateNextProcessFrameTime();
  }

#ifdef SYNCPOLICY_MS_DEBUG_ENABLE
  enterframe_fps.TickFrame();
#endif
}

void SyncPolicyMasterSlave::UpdateNextProcessFrameTime() {
  senscord::osal::OSLockMutex(frame_queue_mutex_);

  if ((frame_queue_.count(master_stream_) == 0) ||
      frame_queue_.at(master_stream_).empty()) {
    senscord::osal::OSUnlockMutex(frame_queue_mutex_);
    return;
  }

  uint64_t master_frame_arrived =
      frame_queue_[master_stream_].front().timestamp;
  uint64_t expect_process_frame_time =
      master_frame_arrived + time_range_ + additional_wait_;

  polling_period_ =
      1000000000ULL * master_framerate_.denom / master_framerate_.num;
  polling_offset_ = expect_process_frame_time;
  pending_process_frame_nsec_ = expect_process_frame_time;

  senscord::osal::OSUnlockMutex(frame_queue_mutex_);

  policy_api_->SetProcessConfig(polling_offset_, polling_period_, true);

  SYNCPOLICY_MS_LOG_DEBUG("[UpdatePollingConfig] master arrived: " NS_PRINT
                          ", expect_process_time: " NS_PRINT,
      NS_PRINT_ARG(master_frame_arrived),
      NS_PRINT_ARG(expect_process_frame_time));
}

void SyncPolicyMasterSlave::ProcessFrame() {
  if (!is_stream_active_) {
    CleanupOldFrame();
    return;
  }

  uint64_t now = 0;
  senscord::osal::OSGetTime(&now);

  senscord::osal::OSLockMutex(frame_queue_mutex_);

#ifdef SYNCPOLICY_MS_DEBUG_ENABLE
  SYNCPOLICY_MS_LOG_DEBUG("[ProcessFrame] now: " NS_PRINT_LONG
                          " master count:%d",
      NS_PRINT_LONG_ARG(now), frame_queue_[master_stream_].size());
#endif

  if (pending_process_frame_nsec_ < now) {
    pending_process_frame_nsec_ = 0;
  }

  while (GenerateSyncFrame()) {
  }

  CleanupOldFrame();

  if (!frame_queue_.empty()) {
    UpdateNextProcessFrameTime();
  }

  senscord::osal::OSUnlockMutex(frame_queue_mutex_);

#ifdef SYNCPOLICY_MS_DEBUG_ENABLE
  processframe_fps.TickFrame();
#endif
}

bool SyncPolicyMasterSlave::CleanupOldFrame() {
  uint64_t now = 0, drop_limit;
  senscord::osal::OSGetTime(&now);
  drop_limit = now - kSyncPolicyMasterSlaveDropLimit;

  senscord::osal::OSLockMutex(frame_queue_mutex_);

  senscord::Status status;
  //  uint64_t frame_ts;
  int drop_count = 0;
  for (auto &sf : frame_queue_) {
    if (sf.first == master_stream_) continue;
    for (size_t i = 0; i < sf.second.size(); i++) {
      SynchronizerFrameInfo &frame_info = sf.second.at(i);

      if (frame_info.timestamp < drop_limit) {
        // if not stream_active, do not send FrameDropped event
        policy_api_->DropFrame(sf.first, frame_info.frame, is_stream_active_);
        sf.second.erase(sf.second.begin() + i);
        drop_count++;
      }
    }
  }

  senscord::osal::OSUnlockMutex(frame_queue_mutex_);

  if (drop_count) {
    SYNCPOLICY_MS_LOG_DEBUG(
        "[CleanupOldFrame] Dropped frame count: %d", drop_count);
  }

  return true;
}

bool SyncPolicyMasterSlave::GenerateSyncFrame() {
  if (frame_queue_.count(master_stream_) <= 0) {
    return false;
  }

  auto &master_frame_list = frame_queue_[master_stream_];
  if (master_frame_list.empty()) {
    return false;
  }

  auto &master_frame_info = frame_queue_[master_stream_].front();

  // check receive time timestamp
  uint64_t processable_time =
      master_frame_info.timestamp + time_range_ + additional_wait_;
  uint64_t now = 0;
  senscord::osal::OSGetTime(&now);

  if (processable_time > now) {
    SYNCPOLICY_MS_LOG_DEBUG(
        "[GenSyncFrame] The range is not finished yet. " NS_PRINT
        " > " NS_PRINT,
        NS_PRINT_ARG(processable_time), NS_PRINT_ARG(now));
    return false;
  }

  // process frame timestamp
  uint64_t master_ts;
  if (!policy_api_->GetTimeStamp(
          master_frame_info.frame, master_stream_, &master_ts)) {
    SYNCPOLICY_MS_LOG_ERROR(
        "[GenSyncFrame] Failed to get MasterFrame's timestamp.");

    policy_api_->DropFrame(
        master_stream_, master_frame_info.frame, is_stream_active_);
    frame_queue_[master_stream_].pop_front();

    policy_api_->RaiseEvent(kSynchronizerEvent, senscord::kEventError, NULL);

    return false;
  }

  if (master_ts < polling_period_) {
    SYNCPOLICY_MS_LOG_ERROR(
        "[GenSyncFrame] MasterFrame timestamp is too small. timestamp=%" PRIu64
        ", polling_period=%" PRIu64 ".",
        master_ts, polling_period_);

    policy_api_->DropFrame(
        master_stream_, master_frame_info.frame, is_stream_active_);
    frame_queue_[master_stream_].pop_front();

    policy_api_->RaiseEvent(kSynchronizerEvent, senscord::kEventError, NULL);
    return false;
  }

  // calc time range
  uint64_t range_start = master_ts - time_range_;
  uint64_t range_end = master_ts + time_range_;

  SYNCPOLICY_MS_LOG_DEBUG("[GenSyncFrame] range: <" NS_PRINT_LONG
                          " -- " NS_PRINT_LONG " -- " NS_PRINT_LONG ">",
      NS_PRINT_LONG_ARG(range_start), NS_PRINT_LONG_ARG(master_ts),
      NS_PRINT_LONG_ARG(range_end));

  // judge frames
  SyncFrame sync_frame;
  for (auto &sf : frame_queue_) {
    if (sf.first == master_stream_) {
      // process master frame
      SynchronizerFrameInfo &mf = sf.second.front();

      mf.timestamp = master_ts;

      // copy to SyncFrame
      sync_frame[sf.first].push_back(mf);

      sf.second.pop_front();  // dequeue master frame
      continue;
    }

    // process slave stream
#ifdef SYNCPOLICY_MS_DEBUG_FRAME_JUDGE
#ifndef SYNCPOLICY_MS_DEBUG_FRAME_JUDGE_LOG_ALL
    bool frame_judge_printed = false;
#endif
#endif
    size_t old_drop_count = 0, in_range_count = 0;
    for (size_t i = 0; i < sf.second.size(); /* do nothing */) {
      SynchronizerFrameInfo &frameinfo = sf.second.at(i);
      uint64_t slave_ts;

      policy_api_->GetTimeStamp(frameinfo.frame, sf.first, &slave_ts);

      if (slave_ts < range_start) {
        // too old frame
        policy_api_->DropFrame(sf.first, frameinfo.frame, is_stream_active_);
        sf.second.erase(sf.second.begin() + i);  // dequeue slave frame
        old_drop_count++;

      } else if (slave_ts < range_end) {
        // in range
        frameinfo.timestamp = slave_ts;

        sync_frame[sf.first].push_back(frameinfo);
        sf.second.erase(sf.second.begin() + i);  // dequeue slave frame
        in_range_count++;

      } else {
        // out of range
        // skip
        i++;
      }

#ifdef SYNCPOLICY_MS_DEBUG_FRAME_JUDGE
#ifndef SYNCPOLICY_MS_DEBUG_FRAME_JUDGE_LOG_ALL
      if (!frame_judge_printed) {
#else
      {
#endif
        uint64_t last_ts = 0;
        if (!sf.second.empty())
          policy_api_->GetTimeStamp(sf.second.back().frame, sf.first, &last_ts);

        SYNCPOLICY_MS_LOG_DEBUG(
            "[GenSyncFrame][Judge] %s ts:" NS_PRINT_LONG " (~" NS_PRINT
            "), current drop:%zu, in range:%zu remaining frame:%zu",
            stream_key_map_[sf.first].c_str(), NS_PRINT_LONG_ARG(slave_ts),
            NS_PRINT_ARG(last_ts), old_drop_count, in_range_count,
            sf.second.size());
#ifndef SYNCPOLICY_MS_DEBUG_FRAME_JUDGE_LOG_ALL
        frame_judge_printed = true;
#endif
      }
#endif
    }
  }

  if (oneframe_per_stream_) {
    FrameFilterNearMaster(&sync_frame, master_ts);
  }

  if (overwrite_timestamp_) {
    OverWriteMasterTimeStamp(&sync_frame, master_ts);
  }

  policy_api_->SendSyncFrame(sync_frame);

  return true;
}

bool SyncPolicyMasterSlave::FrameFilterNearMaster(
    SyncFrame *sync_frame, uint64_t master_timestamp) {
  SYNCPOLICY_MS_LOG_DEBUG_FILTER("[FrameFilterNearMaster] MasterTS: " NS_PRINT,
      NS_PRINT_ARG(master_timestamp));

  for (auto &sf : *sync_frame) {
    if (sf.first == master_stream_) continue;
    SYNCPOLICY_MS_LOG_DEBUG_FILTER(
        "[FrameFilterNearMaster] Stream=%s", stream_key_map_[sf.first].c_str());

    // find most master near frame
    int64_t min_ts_distance = INT64_MAX;
    SynchronizerFrameInfo *min_frameinfo = NULL;
    for (auto &frameinfo : sf.second) {
      int64_t ts_distance = abs(static_cast<int64_t>(frameinfo.timestamp) -
                                static_cast<int64_t>(master_timestamp));
      if (ts_distance < min_ts_distance) {
        min_frameinfo = &frameinfo;
        min_ts_distance = ts_distance;
      }

      SYNCPOLICY_MS_LOG_DEBUG_FILTER(
          "[FrameFilterNearMaster]   Check: " NS_PRINT " diff: " NS_PRINT,
          NS_PRINT_ARG(frameinfo.timestamp), NS_PRINT_ARG(ts_distance));
    }

    if (min_frameinfo) {
      SYNCPOLICY_MS_LOG_DEBUG_FILTER(
          "[FrameFilterNearMaster]   SELECTED: " NS_PRINT,
          NS_PRINT_ARG(min_frameinfo->timestamp));

      // release other frames
      for (auto &frameinfo : sf.second) {
        if (&frameinfo != min_frameinfo) {
          policy_api_->DropFrame(sf.first, frameinfo.frame, is_stream_active_);
          frameinfo.frame = NULL;
        }
      }

      // copy
      SynchronizerFrameInfo tmp(*min_frameinfo);
      sf.second.clear();
      sf.second.push_back(tmp);
    }
  }
  return true;
}

bool SyncPolicyMasterSlave::OverWriteMasterTimeStamp(
    SyncFrame *sync_frame, uint64_t master_timestamp) {
  for (auto &sf : *sync_frame) {
    if (sf.first == master_stream_) continue;
    for (auto &framinfo : sf.second) {
      framinfo.timestamp = master_timestamp;
    }
  }
  return true;
}

// debug print
void SyncPolicyMasterSlave::DebugFpsPrint() {
#ifdef SYNCPOLICY_MS_DEBUG_ENABLE
  SYNCPOLICY_MS_LOG_DEBUG(
      "[policy backdoor] FrameCallback: %.1lffps, ProcessFrame: %.1lffps",
      enterframe_fps.GetFrameRate(), processframe_fps.GetFrameRate());
#endif
  // policy_api_->SetProcessConfig(0, 1000 * 1000 * 3, true);
}
