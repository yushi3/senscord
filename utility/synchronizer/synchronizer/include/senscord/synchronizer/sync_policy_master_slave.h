/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
// public header

#ifndef SENSCORD_SYNCHRONIZER_SYNC_POLICY_MASTER_SLAVE_H_
#define SENSCORD_SYNCHRONIZER_SYNC_POLICY_MASTER_SLAVE_H_

#include <deque>
#include <map>
#include <string>
#include <vector>
#include <unordered_map>

#include "senscord/osal.h"
#include "senscord/senscord.h"
#include "senscord/synchronizer/fps_meter.h"
#include "senscord/synchronizer/synchronizer.h"

// === Debug Switch ===
// Affects performance if enabled
// #define SYNCPOLICY_MS_DEBUG_ENABLE
// #define SYNCPOLICY_MS_DEBUG_FRAME_JUDGE
// #define SYNCPOLICY_MS_DEBUG_FRAME_JUDGE_LOG_ALL
// #define SYNCPOLICY_MS_UNUSED_FRAME_DUMP
// #define SYNCPOLICY_MS_LOG_DEBUG_FILTER_ENABLE
// ====================

// #define SYNCPOLICY_MS_USE_FRAME_TIMESTAMP_FOR_POLLING

const uint64_t kSyncPolicyMasterSlaveDropLimit =
    2 * 1000 * 1000 * 1000;  // nanosec

class SyncPolicyMasterSlave : public SyncPolicy {
 private:
  SyncPolicyApi *policy_api_ = NULL;

  // source stream list
  std::vector<SyncStreamInfo> stream_list_;

  // index to stream_list_
  std::unordered_map<senscord::Stream *, SyncStreamInfo *> stream_map_;

  // master stream, pointer to stream_list_
  senscord::Stream *master_stream_;
  senscord::FrameRateProperty master_framerate_;

  senscord::osal::OSMutex *frame_queue_mutex_;
  std::unordered_map<senscord::Stream *, std::deque<SynchronizerFrameInfo>>
      frame_queue_;
  // in frame_queue's SynchronizerFrameInfo.timestamp is used to received time
  // (master only)
  // std::map<uint64_t, uint64_t> master_frame_arrive_time_; // <sequence_no,
  // arrive time nanosec >

  uint64_t polling_offset_, polling_period_;
  uint64_t pending_process_frame_nsec_;  // 0=not pending, else=wait for
                                         // ProcessFrame()

  uint64_t time_range_;
  uint64_t additional_wait_;

  bool overwrite_timestamp_;
  bool oneframe_per_stream_;
  bool wait_all_stream_on_start_;

  bool is_stream_active_;
  std::unordered_map<senscord::Stream *, bool> is_stream_active_map_;
  std::unordered_map<senscord::Stream *, std::string> stream_key_map_;

  bool initialized_;

#ifdef SYNCPOLICY_MS_DEBUG_ENABLE
  FpsMeter enterframe_fps;
  FpsMeter processframe_fps;
#endif

 public:
  SyncPolicyMasterSlave();
  ~SyncPolicyMasterSlave();
  senscord::Status Init(uint64_t time_range, uint64_t additional_wait,
      bool overwrite_timestamp, bool oneframe_per_stream,
      bool wait_all_stream_on_start);
  senscord::Status Exit();
  senscord::Status Start(SyncPolicyApi *policy_api);
  senscord::Status Stop();
  void EnterSourceFrame(senscord::Stream *stream,
      const std::vector<senscord::Frame *> &source_frames);
  void ProcessFrame();

  // debug
  void DebugFpsPrint();

 private:
  bool CleanupOldFrame();
  bool GenerateSyncFrame();
  bool FrameFilterNearMaster(SyncFrame *sync_frame, uint64_t master_timestamp);
  bool OverWriteMasterTimeStamp(
      SyncFrame *sync_frame, uint64_t master_timestamp);
  void UpdateNextProcessFrameTime();
};

// macros
#ifdef SYNCPOLICY_MS_DEBUG_ENABLE
#define SYNCPOLICY_MS_LOG_DEBUG(...) SENSCORD_LOG_INFO(__VA_ARGS__)
#else
#define SYNCPOLICY_MS_LOG_DEBUG(...)
#endif

#ifdef SYNCPOLICY_MS_LOG_DEBUG_FILTER_ENABLE
#define SYNCPOLICY_MS_LOG_DEBUG_FILTER(...) SYNCPOLICY_MS_LOG_DEBUG(__VA_ARGS__)
#else
#define SYNCPOLICY_MS_LOG_DEBUG_FILTER(...)
#endif

#define SYNCPOLICY_MS_LOG_INFO(...) SENSCORD_LOG_INFO(__VA_ARGS__)
#define SYNCPOLICY_MS_LOG_WARNING(...) SENSCORD_LOG_WARNING(__VA_ARGS__)
#define SYNCPOLICY_MS_LOG_ERROR(...) SENSCORD_LOG_ERROR(__VA_ARGS__)

#endif  // SENSCORD_SYNCHRONIZER_SYNC_POLICY_MASTER_SLAVE_H_
