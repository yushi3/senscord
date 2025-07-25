/*
 * SPDX-FileCopyrightText: 2020-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/synchronizer/synchronizer.h"

#include <algorithm>
#include <deque>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "senscord/osal.h"
#include "senscord/senscord.h"

uint64_t kSynchronizerStopThreadRetryInterval = 1ULL * 1000 * 1000;

#define CheckStateEq(state, cause)                                       \
  {                                                                      \
    auto local_state = GetState();                                       \
    if (local_state == state) {                                          \
      return SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,              \
          senscord::Status::kCauseInvalidOperation, cause " (state=%d)", \
          local_state);                                                  \
    }                                                                    \
  }

#define CheckStateNeq(state, cause)                                      \
  {                                                                      \
    auto local_state = GetState();                                       \
    if (local_state != state) {                                          \
      return SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,              \
          senscord::Status::kCauseInvalidOperation, cause " (state=%d)", \
          local_state);                                                  \
    }                                                                    \
  }

Synchronizer::Synchronizer()
    : policy_api_(this),
      policy_(NULL),
      auto_start_(false),
      getframe_error_count_(0),
      syncframe_queue_num_(3) {
  senscord::osal::OSCreateMutex(&state_mutex_);
  SetState(kSynchronizerStateNoInit);
}
Synchronizer::~Synchronizer() {
  if (GetState() != kSynchronizerStateNoInit) Exit();

  senscord::osal::OSDestroyMutex(state_mutex_);
  state_mutex_ = NULL;
}

// receive paramater and initialize resources
senscord::Status Synchronizer::Init(SyncPolicy *policy,
    const std::vector<SyncStreamInfo> &streams, bool auto_start,
    uint32_t syncframe_queue_num) {
  CheckStateNeq(kSynchronizerStateNoInit, "aleady initialized");

  if (streams.empty())
    return SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,
        senscord::Status::kCauseInvalidArgument, "stream info is empty.");

  policy_ = policy;
  auto_start_ = auto_start;

  if (syncframe_queue_num < 1) {
    return SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,
        senscord::Status::kCauseInvalidArgument,
        "syncframe_queue_num is too small.");
  }

  // map stream list
  stream_list_.clear();
  stream_list_ = streams;

  stream_map_.clear();
  for (auto &s : stream_list_) {
    if (s.instance == NULL) {
      return SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,
          senscord::Status::kCauseInvalidArgument, "null stream found.");
    }

    // check main_channel_id is valid
    senscord::Status status;
    senscord::ChannelInfoProperty chinfo;
    status =
        s.instance->GetProperty(senscord::kChannelInfoPropertyKey, &chinfo);
    if (status.ok()) {
      if (chinfo.channels.count(s.main_channel_id) == 0) {
        return SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,
            senscord::Status::kCauseInvalidArgument,
            "main channel %dch was not found in stream=%s", s.main_channel_id,
            SyncPolicyApi::ToString(s.instance).c_str());
      }

    } else {
      SYNCHRONIZER_LOG_ERROR(
          "[Init] Failed to get ChannelIfnoProperty at stream=%s. "
          "main_channel_id check is skiped.",
          SyncPolicyApi::ToString(s.instance).c_str());
    }

    stream_map_[s.instance] = &s;
  }

  // SyncFrame Queue
  syncframe_queue_num_ = syncframe_queue_num;
  syncframe_queue_.resize(syncframe_queue_num_);
  syncframe_queue_.clear();
  senscord::osal::OSCreateMutex(&syncframe_queue_mutex_);
  senscord::osal::OSCreateCond(&syncframe_queue_enqueued_cond_);

  // Event Queue
  event_queue_.clear();
  senscord::osal::OSCreateMutex(&event_queue_mutex_);
  senscord::osal::OSCreateCond(&event_queue_cond_);

  // callbacks
  senscord::osal::OSCreateMutex(&frame_callback_.mutex);
  senscord::osal::OSCreateMutex(&event_callbacks_mutex_);
  frame_callback_.func_ptr = NULL;
  event_callbacks_.clear();

  // process frame thins
  senscord::osal::OSCreateMutex(&process_frame_config_mutex_);
  senscord::osal::OSCreateCond(&process_frame_config_cond_);
  process_frame_config_.offset = kDefaultPollingOffset;
  process_frame_config_.period = kDefaultPollingPeriod;

  SetState(kSynchronizerStateReady);
  return senscord::Status::OK();
}

// release resource
senscord::Status Synchronizer::Exit() {
  CheckStateEq(kSynchronizerStateNoInit, "not initialized");

  if (GetState() == kSynchronizerStateRunning) {
    SYNCHRONIZER_LOG_WARNING("[Exit] synchronizer not stopped. stopping...");

    senscord::Status status = Stop();
    if (!status.ok()) {
      SYNCHRONIZER_LOG_ERROR(
          "[Exit] An error occurred in Stop. %s", status.ToString().c_str());
      SYNCHRONIZER_LOG_ERROR("[Exit] Processing will continue.");
    }
  }

  senscord::osal::OSDestroyMutex(syncframe_queue_mutex_);
  senscord::osal::OSDestroyCond(syncframe_queue_enqueued_cond_);
  syncframe_queue_mutex_ = NULL;
  syncframe_queue_enqueued_cond_ = NULL;

  senscord::osal::OSDestroyMutex(event_queue_mutex_);
  senscord::osal::OSDestroyCond(event_queue_cond_);
  event_queue_mutex_ = NULL;
  event_queue_cond_ = NULL;

  senscord::osal::OSDestroyMutex(frame_callback_.mutex);
  senscord::osal::OSDestroyMutex(event_callbacks_mutex_);
  frame_callback_.mutex = NULL;
  event_callbacks_mutex_ = NULL;

  senscord::osal::OSDestroyMutex(process_frame_config_mutex_);
  senscord::osal::OSDestroyCond(process_frame_config_cond_);
  process_frame_config_mutex_ = NULL;
  process_frame_config_cond_ = NULL;

  stream_map_.clear();

  // enum unregisted event callback
  std::vector<std::string> event_names;
  event_names.reserve(event_callbacks_.size());
  std::transform(
      event_callbacks_.cbegin(),
      event_callbacks_.cend(),
      std::back_inserter(event_names),
      [](const std::pair<std::string, EventCallbackConfig*>& event) {
        return event.first;
      });

  // clear event callback configurations
  for (auto event : event_names) {
    SYNCHRONIZER_LOG_DEBUG("[Exit] not unregisterd event callback: %s",
                           event.c_str());

    senscord::Status status = UnregisterEventCallbackInternal(event);
    if (!status.ok()) {
      SYNCHRONIZER_LOG_WARNING(
          "[Exit] UnregisterEventCallback failed. %s",
          status.ToString().c_str());
    }
  }
  event_callbacks_.clear();

  SetState(kSynchronizerStateNoInit);

  return senscord::Status::OK();
}

senscord::Status Synchronizer::Start() {
  CheckStateNeq(kSynchronizerStateReady, "not ready to start");

  // clear source stream GetFrame error count
  getframe_error_count_ = 0;

  // start policy before stream start
  senscord::Status status;
  status = policy_->Start(&policy_api_);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Register frame callback to source stream
  for (auto &s : stream_list_) {
    status = s.instance->RegisterFrameCallback(
        Synchronizer::SourceStreamFrameCallback, this);
    if (!status.ok()) {
      SYNCHRONIZER_LOG_ERROR(
          "[Start] RegisterFrameCallback failed at stream=%s. status=%s",
          SyncPolicyApi::ToString(s.instance).c_str(),
          status.ToString().c_str());
      policy_->Stop();
      return SENSCORD_STATUS_TRACE(status);
    }
  }

  // auto start
  if (auto_start_) {
    for (size_t i = 0; i < stream_list_.size(); i++) {
      SyncStreamInfo &stream_info = stream_list_.at(i);
      status = stream_info.instance->Start();
      SYNCHRONIZER_LOG_INFO("[AutoStart] stream=%s",
          SyncPolicyApi::ToString(stream_info.instance).c_str());

      if (!status.ok()) {
        SYNCHRONIZER_LOG_ERROR(
            "[AutoStart] Failed to start at stream=%s. status=%s",
            SyncPolicyApi::ToString(stream_info.instance).c_str(),
            status.ToString().c_str());

        // cancel start
        policy_->Stop();
        for (size_t j = 0; j < i; j++) {
          SyncStreamInfo &cancel_stream = stream_list_.at(j);
          senscord::Status tstat = cancel_stream.instance->Stop();
          if (!tstat.ok()) {
            SYNCHRONIZER_LOG_ERROR(
                "[AutoStart] Stop() for cancellation has failed at stream=%s. "
                "status=%s",
                SyncPolicyApi::ToString(cancel_stream.instance).c_str(),
                tstat.ToString().c_str());
          }
        }

        return SENSCORD_STATUS_TRACE(status);
      }
    }
  }

  SetState(kSynchronizerStateRunning);

  senscord::osal::OSCreateThread(
      &process_frame_thread_, ProcessFrameThread, this, NULL);
  senscord::osal::OSCreateThread(
      &user_frame_callback_thread_, UserFrameCallbackThread, this, NULL);
  senscord::osal::OSCreateThread(
      &user_event_callback_thread_, UserEventCallbackThread, this, NULL);

  // acquire pre-buffered frames
  for (auto &s : stream_list_) {
    int ret = AcquireFrame(this, s.instance);
    SYNCHRONIZER_LOG_DEBUG(
        "[Start] Initial AcquireFrame: stream=%s, frame count=%d",
        SyncPolicyApi::ToString(s.instance).c_str(), ret);
  }

  return senscord::Status::OK();
}

senscord::Status Synchronizer::Stop() {
  CheckStateNeq(kSynchronizerStateRunning, "not running");

  senscord::Status status;
  senscord::Status ret;

  // Unregister frame callback to source stream
  for (auto &s : stream_list_) {
    status = s.instance->UnregisterFrameCallback();
    if (!status.ok()) {
      SYNCHRONIZER_LOG_ERROR(
          "[Stop] UnregisterFrameCallback failed at stream=%s. status=%s",
          SyncPolicyApi::ToString(s.instance).c_str(),
          status.ToString().c_str());
    }
  }

  // auto stop
  if (auto_start_) {
    for (auto &s : stream_list_) {
      SYNCHRONIZER_LOG_INFO(
          "[AutoStop] stream=%s", SyncPolicyApi::ToString(s.instance).c_str());

      status = s.instance->Stop();
      if (!status.ok()) {
        SYNCHRONIZER_LOG_ERROR(
            "[AutoStop] Failed to stop at stream=%s. status=%s",
            SyncPolicyApi::ToString(s.instance).c_str(),
            status.ToString().c_str());
        ret = SENSCORD_STATUS_TRACE(status);
      }
    }
  }

  SetState(kSynchronizerStateWaitStop);

  // Stop threads
  struct {
    const char *label;
    senscord::osal::OSThread **pp_handle;
    senscord::osal::OSMutex *mutex;
    senscord::osal::OSCond *cond;
  } threads[] = {
      {"ProcessFrameThread", &process_frame_thread_,
          process_frame_config_mutex_, process_frame_config_cond_},
      {"UserFrameCallbackThread", &user_frame_callback_thread_,
          syncframe_queue_mutex_, syncframe_queue_enqueued_cond_},
      {"UserEventCallbackThread", &user_event_callback_thread_,
          event_queue_mutex_, event_queue_cond_},
  };

  SYNCHRONIZER_LOG_DEBUG("[Stop] Thread join start");
  for (size_t i = 0; i < (sizeof(threads) / sizeof(threads[0])); i++) {
    SYNCHRONIZER_LOG_INFO("[Stop] waiting %s exit... ", threads[i].label);

    while (true) {
      int32_t retval = senscord::osal::OSRelativeTimedJoinThread(
          *(threads[i].pp_handle), kSynchronizerStopThreadRetryInterval, NULL);
      if (senscord::osal::error::IsTimeout(retval)) {
        // notify stop to thread
        senscord::osal::OSLockMutex(threads[i].mutex);
        senscord::osal::OSBroadcastCond(threads[i].cond);
        senscord::osal::OSUnlockMutex(threads[i].mutex);
      } else {
        break;
      }
    }

    *(threads[i].pp_handle) = NULL;
  }
  SYNCHRONIZER_LOG_DEBUG("[Stop] Thread join complete");

  // stop policy and expects release policy keeped frame
  status = policy_->Stop();
  if (!status.ok()) {
    ret = SENSCORD_STATUS_TRACE(status);
  }

  senscord::osal::OSLockMutex(syncframe_queue_mutex_);

  SYNCHRONIZER_LOG_DEBUG("[Stop] Cleanup syncframe queue... %zu synframes",
      syncframe_queue_.size());

  for (auto &sf : syncframe_queue_) {
    ReleaseSyncFrame(&sf);
  }
  syncframe_queue_.clear();

  senscord::osal::OSUnlockMutex(syncframe_queue_mutex_);

  SetState(kSynchronizerStateReady);

  return ret;
}

senscord::Status Synchronizer::GetSyncFrame(
    SyncFrame *sync_frame, int32_t timeout_msec) {
  CheckStateNeq(kSynchronizerStateRunning, "not running");

  // ref: stream_core.cpp
  uint64_t abstime = 0;
  if (timeout_msec > 0) {
    senscord::osal::OSGetTime(&abstime);
    abstime += static_cast<uint64_t>(timeout_msec) * 1000 * 1000;
  }

  senscord::Status ret = senscord::Status::OK();

  senscord::osal::OSLockMutex(syncframe_queue_mutex_);
  while (true) {
    if (syncframe_queue_.size() > 0) {
      *sync_frame = syncframe_queue_.front();
      syncframe_queue_.pop_front();
      break;
    }

    if (timeout_msec == senscord::kTimeoutPolling) {
      ret = SENSCORD_STATUS_FAIL(
          kStatusBlockSynchronizer, senscord::Status::kCauseTimeout,
          "no frame received.");
      break;
    } else if (timeout_msec > 0) {
      // timeout wait
      int32_t cv_ret = senscord::osal::OSTimedWaitCond(
          syncframe_queue_enqueued_cond_, syncframe_queue_mutex_, abstime);
      if (cv_ret < 0) {
        ret = SENSCORD_STATUS_FAIL(
            kStatusBlockSynchronizer, senscord::Status::kCauseTimeout,
            "no frame received.");
        break;
      }
    } else {
      // forever wait
      senscord::osal::OSWaitCond(
          syncframe_queue_enqueued_cond_, syncframe_queue_mutex_);
    }

    // check status after OSWaitCond
    if (GetState() != kSynchronizerStateRunning) {
      ret = SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,
          senscord::Status::kCauseInvalidOperation, "stream stopped");
      break;
    }
  }
  senscord::osal::OSUnlockMutex(syncframe_queue_mutex_);

  return SENSCORD_STATUS_TRACE(ret);
}

senscord::Status Synchronizer::ReleaseSyncFrame(SyncFrame *sync_frame) {
  senscord::Status result = senscord::Status::OK();
  senscord::Status status;

  if (sync_frame == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,
        senscord::Status::kCauseInvalidArgument, "SyncFrame is null.");
  }

  for (auto stream_frames_pair : *sync_frame) {
    if (stream_frames_pair.first == NULL) {
      result = SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,
          senscord::Status::kCauseInvalidArgument,
          "null stream found. skipped.");
      continue;
    }

    for (auto frame_info : stream_frames_pair.second) {
      // don't check Frame* null (then, ReleaseFrame returns error)
      status = stream_frames_pair.first->ReleaseFrame(frame_info.frame);

      if (!status.ok()) {
        result = SENSCORD_STATUS_TRACE(status);
      }
    }
    stream_frames_pair.second.clear();
  }
  sync_frame->clear();

  return result;
}

// =============================================================
// callback registrars
senscord::Status Synchronizer::RegisterSyncFrameCallback(
    const SynchronizerOnFrameReceivedCallback callback, void *private_data) {
  CheckStateEq(kSynchronizerStateNoInit, "Not initialized.");

  if (callback == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,
        senscord::Status::kCauseTimeout, "callback is null");
  }
  senscord::osal::OSLockMutex(frame_callback_.mutex);

  frame_callback_.func_ptr = callback;
  frame_callback_.private_data = private_data;

  senscord::osal::OSUnlockMutex(frame_callback_.mutex);

  return senscord::Status::OK();
}

senscord::Status Synchronizer::UnregisterSyncFrameCallback() {
  CheckStateEq(kSynchronizerStateNoInit, "Not initialized.");

  senscord::Status status = senscord::Status::OK();

  senscord::osal::OSLockMutex(frame_callback_.mutex);

  if (frame_callback_.func_ptr == NULL) {
    status = SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,
        senscord::Status::kCauseTimeout, "not registerd");
  }
  frame_callback_.func_ptr = NULL;
  frame_callback_.private_data = NULL;

  senscord::osal::OSUnlockMutex(frame_callback_.mutex);
  return SENSCORD_STATUS_TRACE(status);
}

senscord::Status Synchronizer::RegisterEventCallback(
    const std::string &event_type,
    const SynchronizerOnEventReceivedCallback callback, void *private_data) {
  CheckStateEq(kSynchronizerStateNoInit, "Not initialized.");

  if (callback == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,
        senscord::Status::kCauseTimeout, "callback is null");
  }

  EventCallbackConfig *event_callback = new EventCallbackConfig();
  event_callback->func_ptr = callback;
  event_callback->event_type = event_type;
  event_callback->user_private_data = private_data;

  senscord::osal::OSLockMutex(event_callbacks_mutex_);

  // aleady registerd
  if (event_callbacks_.count(event_type)) {
    UnregisterEventCallbackInternal(event_type);
  }

  // event relay
  for (auto stream : stream_list_) {
    senscord::Status status;
    EventCallbackPrivateData *src_private_data = new EventCallbackPrivateData();
    src_private_data->synchronizer = this;
    src_private_data->stream = stream.instance;

    status = stream.instance->RegisterEventCallback(event_type,
        SourceStreamEventCallbackOld, static_cast<void *>(src_private_data));

    if (status.ok()) {
      event_callback->source_private_datas.push_back(src_private_data);

      SYNCHRONIZER_LOG_DEBUG("[RegisterEventCallback] Register %s to stream=%s",
          event_type.c_str(), SyncPolicyApi::ToString(stream.instance).c_str());

    } else {
      SYNCHRONIZER_LOG_ERROR(
          "[RegisterEventCallback] Failed to register %s to stream=%s",
          event_type.c_str(), SyncPolicyApi::ToString(stream.instance).c_str());

      senscord::osal::OSUnlockMutex(event_callbacks_mutex_);

      delete src_private_data;
      delete event_callback;

      return SENSCORD_STATUS_TRACE(status);
    }
  }

  // register
  event_callbacks_[event_type] = event_callback;

  senscord::osal::OSUnlockMutex(event_callbacks_mutex_);

  return senscord::Status::OK();
}

senscord::Status Synchronizer::UnregisterEventCallbackInternal(
    const std::string &event_type) {
  senscord::Status ret = senscord::Status::OK();

  if (event_callbacks_.count(event_type) == 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,
        senscord::Status::kCauseNotFound, "Event not found.");

  } else if (event_callbacks_[event_type] == NULL) {
    event_callbacks_.erase(event_type);
    return SENSCORD_STATUS_FAIL(kStatusBlockSynchronizer,
        senscord::Status::kCauseNotFound,
        "[UnregisterEventCallback] Internal error: EventCallbackConfig is "
        "null. event=%s",
        event_type.c_str());
  }

  // destroy event callback private_data
  for (auto event_private_data :
      event_callbacks_[event_type]->source_private_datas) {
    if (event_private_data == NULL) {
      SYNCHRONIZER_LOG_ERROR(
          "[UnregisterEventCallback] Internal error: "
          "EventCallbackPrivateData is null");
      continue;
    }

    SYNCHRONIZER_LOG_DEBUG(
        "[UnregisterEventCallback] Unregister %s to stream=%s",
        event_type.c_str(),
        SyncPolicyApi::ToString(event_private_data->stream).c_str());

    if (event_private_data->stream != NULL) {
      senscord::Status s;
      s = event_private_data->stream->UnregisterEventCallback(event_type);
      if (!s.ok()) ret = SENSCORD_STATUS_TRACE(s);
    } else {
      SYNCHRONIZER_LOG_ERROR(
          "[UnregisterEventCallback] Internal error: "
          "EventCallbackPrivateData::stream is null");
    }

    // clear data
    event_private_data->stream = NULL;
    event_private_data->synchronizer = NULL;

    delete event_private_data;
  }
  event_callbacks_[event_type]->source_private_datas.clear();

  // destroy event callback config
  delete event_callbacks_[event_type];
  event_callbacks_.erase(event_type);

  return ret;
}

senscord::Status Synchronizer::UnregisterEventCallback(
    const std::string &event_type) {
  CheckStateEq(kSynchronizerStateNoInit, "Not initialized.");

  senscord::Status ret;
  senscord::osal::OSLockMutex(event_callbacks_mutex_);
  ret = UnregisterEventCallbackInternal(event_type);
  senscord::osal::OSUnlockMutex(event_callbacks_mutex_);

  return SENSCORD_STATUS_TRACE(ret);
}

//======================================================================
// source stream callbacks
int Synchronizer::AcquireFrame(Synchronizer *_this, senscord::Stream *stream) {
  senscord::Status status;

  auto local_state = _this->GetState();
  if (local_state != kSynchronizerStateRunning) {
    SYNCHRONIZER_LOG_ERROR(
        "[AcquireFrame] status is not running. The frame was not "
        "accepted. stream=%s state=%d",
        SyncPolicyApi::ToString(stream).c_str(), local_state);
    return 0;
  }

  std::vector<senscord::Frame *> frames;
  while (1) {
    senscord::Frame *tmp_frame;
    status = stream->GetFrame(&tmp_frame, senscord::kTimeoutPolling);
    if (status.ok()) {
      frames.push_back(tmp_frame);

    } else if (status.cause() == senscord::Status::kCauseTimeout) {
      break;

    } else {
      SYNCHRONIZER_LOG_ERROR(
          "[AcquireFrame] Failed to GetFrame to source stream. "
          "stream=%s, status=%s",
          SyncPolicyApi::ToString(stream).c_str(), status.ToString().c_str());

      return -1;
    }
#ifdef SYNCHRONIZER_GET_SOURCE_FRAME_LIMIT_SINGLE
    break;
#endif
  }

#ifdef SYNCHRONIZER_DEBUG_SOURCE_FRAME_CALLBACK
  SYNCHRONIZER_LOG_DEBUG("[AcquireFrame] stream=%s, frame count=%d",
      SyncPolicyApi::ToString(stream).c_str(), frames.size());
#endif

  if (!frames.empty()) {
    _this->policy_->EnterSourceFrame(stream, frames);
  }

  return static_cast<int>(frames.size());
}

void Synchronizer::SourceStreamFrameCallback(
    senscord::Stream *stream, void *private_data) {
  auto _this = reinterpret_cast<Synchronizer *>(private_data);

  if (_this->getframe_error_count_ >= kSynchronizerGetFrameErrorCountLimit) {
    return;
  }

  int ret = AcquireFrame(_this, stream);
  if (ret < 0) {
    _this->getframe_error_count_++;
    SYNCHRONIZER_LOG_ERROR(
        "[SourceStreamFrameCallback] AcquireFrame failed. error_count=%d/%d",
        _this->getframe_error_count_, kSynchronizerGetFrameErrorCountLimit);

    _this->policy_api_.RaiseEvent(stream, senscord::kEventError, NULL);
  }
}

void Synchronizer::SourceStreamEventCallbackOld(
    const std::string &event_type, const void *param, void *private_data) {
  auto event_data = reinterpret_cast<EventCallbackPrivateData *>(private_data);

  if (event_data->synchronizer->GetState() != kSynchronizerStateRunning) return;

  SYNCHRONIZER_LOG_DEBUG("[RelayEvent] stream:%s type:%s",
      SyncPolicyApi::ToString(event_data->stream).c_str(), event_type.c_str());

  // relay source stream event
  event_data->synchronizer->policy_api_.RaiseEvent(
      event_data->stream, event_type, param);
}

// =======================================================
// thread impl
senscord::osal::OSThreadResult Synchronizer::ProcessFrameThread(void *arg) {
  auto _this = reinterpret_cast<Synchronizer *>(arg);

  while (true) {
    if (_this->GetState() == Synchronizer::kSynchronizerStateWaitStop) break;

    senscord::osal::OSLockMutex(_this->process_frame_config_mutex_);

    uint64_t offset = _this->process_frame_config_.offset;
    uint64_t period = _this->process_frame_config_.period;

    uint64_t now = 0;
    senscord::osal::OSGetTime(&now);

    uint64_t mul, timeout;
    if (now < offset) {
      // The offset is in the future.
      //   -> wait offset;
      timeout = offset;
      mul = 0;
    } else {
      // The offset is in the past.
      //   -> Call ProcessFrame() at polling_period intervals.
      mul = ((now - offset) / period) + 1;
      timeout = offset + period * mul;
    }

#ifdef SYNCHRONIZER_DEBUG_PROCESS_FRAME_THREAD
    SYNCHRONIZER_LOG_DEBUG("[PFT] now: " NS_PRINT " timeout: " NS_PRINT
                           ", mul=%" PRIu64,
        NS_PRINT_ARG(now), NS_PRINT_ARG(timeout), mul);
#endif

    int32_t ret =
        senscord::osal::OSTimedWaitCond(_this->process_frame_config_cond_,
            _this->process_frame_config_mutex_, timeout);
    senscord::osal::OSUnlockMutex(_this->process_frame_config_mutex_);

    if (ret != 0) {
      // timeout
      _this->policy_->ProcessFrame();
    } else {
      // option changed
      //  do nothing
#ifdef SYNCHRONIZER_DEBUG_PROCESS_FRAME_THREAD
      SYNCHRONIZER_LOG_DEBUG("[PFT] wakeup reason: config update");
#endif
    }
  }
  return static_cast<senscord::osal::OSThreadResult>(0);
}

senscord::osal::OSThreadResult Synchronizer::UserFrameCallbackThread(
    void *arg) {
  auto _this = reinterpret_cast<Synchronizer *>(arg);

  while (1) {
    if (_this->GetState() == Synchronizer::kSynchronizerStateWaitStop) break;

    senscord::osal::OSLockMutex(_this->syncframe_queue_mutex_);

    bool call = false;
    if (_this->syncframe_queue_.size() > 0) {
      call = true;
    } else {
      senscord::osal::OSWaitCond(
          _this->syncframe_queue_enqueued_cond_, _this->syncframe_queue_mutex_);
    }

    senscord::osal::OSUnlockMutex(_this->syncframe_queue_mutex_);

    if (call) {
      senscord::osal::OSLockMutex(_this->frame_callback_.mutex);
      if (_this->frame_callback_.func_ptr) {
        _this->frame_callback_.func_ptr(
            _this, _this->frame_callback_.private_data);
      }
      senscord::osal::OSUnlockMutex(_this->frame_callback_.mutex);
    }
  }
  return static_cast<senscord::osal::OSThreadResult>(0);
}

senscord::osal::OSThreadResult Synchronizer::UserEventCallbackThread(
    void *arg) {
  auto _this = reinterpret_cast<Synchronizer *>(arg);

  std::deque<EventQueueEntry> event_queue_internal;
  while (1) {
    if (_this->GetState() == Synchronizer::kSynchronizerStateWaitStop) break;

    senscord::osal::OSLockMutex(_this->event_queue_mutex_);

    if (!_this->event_queue_.empty()) {
      // todo use move
      event_queue_internal = _this->event_queue_;
      _this->event_queue_.clear();
    } else {
      senscord::osal::OSWaitCond(
          _this->event_queue_cond_, _this->event_queue_mutex_);
    }

    senscord::osal::OSUnlockMutex(_this->event_queue_mutex_);

    if (!event_queue_internal.empty()) {
      for (auto &evt : event_queue_internal) {
        _this->CallEventCallback(&evt);
      }

#ifdef SYNCHRONIZER_DEBUG_EVENT_CALL
      SYNCHRONIZER_LOG_DEBUG(
          "[EventCall] %d events called", event_queue_internal.size());
#endif

      event_queue_internal.clear();
    }
  }
  return static_cast<senscord::osal::OSThreadResult>(0);
}

// ================
// util
void Synchronizer::CallEventCallback(EventQueueEntry *evt) {
  // check disabled
  if (evt->stream) {
    std::string *disabled_event =
        &(stream_map_[evt->stream]->disabled_event_type);
    if (*disabled_event == senscord::kEventAny ||
        *disabled_event == evt->event_type)
      return;
  }

  senscord::osal::OSLockMutex(event_callbacks_mutex_);

  EventCallbackConfig *callback = NULL;
  if (event_callbacks_.count(evt->event_type)) {
    callback = event_callbacks_[evt->event_type];

  } else if (event_callbacks_.count(senscord::kEventAny)) {
    callback = event_callbacks_[senscord::kEventAny];

  } else {
    senscord::osal::OSUnlockMutex(event_callbacks_mutex_);
    return;
  }

  // call event_callback
  callback->func_ptr(
      this, evt->stream, evt->event_type, NULL, callback->user_private_data);

  senscord::osal::OSUnlockMutex(event_callbacks_mutex_);
}

Synchronizer::SynchronizerState Synchronizer::GetState() {
  senscord::osal::OSLockMutex(state_mutex_);
  SynchronizerState ret = state_;
  senscord::osal::OSUnlockMutex(state_mutex_);
  return ret;
}

void Synchronizer::SetState(SynchronizerState new_state) {
  senscord::osal::OSLockMutex(state_mutex_);
  state_ = new_state;
  senscord::osal::OSUnlockMutex(state_mutex_);
}

// SyncStreamInfo constructors
SyncStreamInfo::SyncStreamInfo() : SyncStreamInfo(NULL) {}
SyncStreamInfo::SyncStreamInfo(senscord::Stream *instance)
    : SyncStreamInfo(instance, kDefaultMainChannelId) {
  if (instance == NULL) return;

  // set main_channel_id automally
  senscord::ChannelInfoProperty ch_property;
  senscord::Status stat =
      instance->GetProperty(senscord::kChannelInfoPropertyKey, &ch_property);
  if (!stat.ok()) return;
  if (ch_property.channels.empty()) return;
  if (ch_property.channels.count(main_channel_id) == 0) {
    main_channel_id = ch_property.channels.begin()->first;

    SYNCHRONIZER_LOG_INFO(
        "[SyncStreamInfo] main_channel_id is auto selected to %d",
        main_channel_id);
  }
}

SyncStreamInfo::SyncStreamInfo(senscord::Stream *instance, int main_channel_id)
    : SyncStreamInfo(instance, main_channel_id, kDefaultDisabledEventType) {}
SyncStreamInfo::SyncStreamInfo(senscord::Stream *instance, int main_channel_id,
    std::string disabled_event_type)
    : instance(instance),
      main_channel_id(main_channel_id),
      disabled_event_type(disabled_event_type) {}
// ========================================================
