/*
 * SPDX-FileCopyrightText: 2020-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
// public header

#ifndef SENSCORD_SYNCHRONIZER_SYNCHRONIZER_H_
#define SENSCORD_SYNCHRONIZER_SYNCHRONIZER_H_

#include <deque>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "senscord/logger.h"
#include "senscord/osal.h"
#include "senscord/senscord.h"

// ==== debug switch ====
// if enabled, use senttime instead of Frame timestamp(sensor timestamp)
// #define USE_SENTTIME

// if those switches enabled, a large amount of logs will be output. and affects
// performance.
// #define SYNCHRONIZER_DEBUG_PROCESS_FRAME_THREAD
// #define SYNCHRONIZER_DEBUG_SOURCE_FRAME_CALLBACK
// #define SYNCHRONIZER_DEBUG_DROP_FRAME
// #define SYNCHRONIZER_DEBUG_EVENT_CALL

// if enebled, get signle frame at SourceStreamFrameCallback
// #define SYNCHRONIZER_GET_SOURCE_FRAME_LIMIT_SINGLE
// ======================

const char kStatusBlockSynchronizer[] = "Synchronizer";

const uint32_t kDefaultMainChannelId = 0;
const char kDefaultDisabledEventType[] = "";

const uint64_t kDefaultPollingOffset = 0;
const uint64_t kDefaultPollingPeriod = 1000 * 1000 * 1000;

const int kSynchronizerGetFrameErrorCountLimit = 10;
constexpr senscord::Stream *kSynchronizerEvent = NULL;

inline uint64_t ns2ms(uint64_t x) { return (x / 1000000ULL) % (1000000ULL); }

// prototype
class SynchronizerInterface;

using SynchronizerOnFrameReceivedCallback = void (*)(
    SynchronizerInterface *synchronizer, void *private_data);

using SynchronizerOnEventReceivedCallback = void (*)(
    SynchronizerInterface *synchronizer, senscord::Stream *stream,
    const std::string &event_type, void *param, void *private_data);

struct SynchronizerFrameInfo {
  senscord::Frame *frame;
  uint64_t timestamp;
  SynchronizerFrameInfo() : frame(NULL), timestamp(0) {}
  SynchronizerFrameInfo(senscord::Frame *_frame, uint64_t _timestamp)
      : frame(_frame), timestamp(_timestamp) {}
};

using SyncFrame =
    std::unordered_map<senscord::Stream *, std::vector<SynchronizerFrameInfo>>;

// types for Synchronizer
struct SyncStreamInfo {
  senscord::Stream *instance;
  uint32_t main_channel_id;  // channel_id uses to get timestamp
  std::string disabled_event_type;

  SyncStreamInfo();
  explicit SyncStreamInfo(senscord::Stream *instance);
  SyncStreamInfo(senscord::Stream *instance, int main_channel_id);
  SyncStreamInfo(senscord::Stream *instance, int main_channel_id,
      std::string disabled_event_type);
};

// define of policy interfaces
class SyncPolicyApi {
 public:
  virtual void SendSyncFrame(const SyncFrame &frame) = 0;
  virtual void DropFrame(senscord::Stream *stream, senscord::Frame *frame,
      bool drop_frame_event) = 0;
  virtual void RaiseEvent(senscord::Stream *stream,
      const std::string &event_type, const void *param) = 0;
  virtual void SetProcessConfig(uint64_t polling_offset,
      uint64_t polling_period, bool apply_immediate) = 0;
  virtual void GetSourceStreamList(
      std::vector<SyncStreamInfo> *stream_list) = 0;
  virtual bool GetTimeStamp(senscord::Frame *frame, senscord::Stream *stream,
      uint64_t *timestamp) = 0;
  static std::string ToString(senscord::Stream *stream);
};

class SyncPolicy {
 public:
  virtual senscord::Status Start(SyncPolicyApi *policy_api) = 0;
  virtual senscord::Status Stop() = 0;
  virtual void EnterSourceFrame(senscord::Stream *stream,
      const std::vector<senscord::Frame *> &source_frames) = 0;
  virtual void ProcessFrame() = 0;
};

// interface
class SynchronizerInterface {
 public:
  virtual senscord::Status Start() = 0;
  virtual senscord::Status Stop() = 0;

  virtual senscord::Status GetSyncFrame(
      SyncFrame *sync_frame, int32_t timeout_msec) = 0;
  virtual senscord::Status ReleaseSyncFrame(SyncFrame *sync_frame) = 0;

  virtual senscord::Status RegisterSyncFrameCallback(
      const SynchronizerOnFrameReceivedCallback callback,
      void *private_data) = 0;
  virtual senscord::Status UnregisterSyncFrameCallback() = 0;

  virtual senscord::Status RegisterEventCallback(const std::string &event_type,
      const SynchronizerOnEventReceivedCallback callback,
      void *private_data) = 0;
  virtual senscord::Status UnregisterEventCallback(
      const std::string &event_type) = 0;
};

class Synchronizer : public SynchronizerInterface {
 public:
  Synchronizer();
  ~Synchronizer();

  senscord::Status Init(SyncPolicy *policy,
      const std::vector<SyncStreamInfo> &streams, bool auto_start,
      uint32_t frames_queue_num);
  senscord::Status Exit();

  senscord::Status Start();
  senscord::Status Stop();

  senscord::Status GetSyncFrame(SyncFrame *sync_frame, int32_t timeout_msec);
  senscord::Status ReleaseSyncFrame(SyncFrame *sync_frame);

  senscord::Status RegisterSyncFrameCallback(
      const SynchronizerOnFrameReceivedCallback callback, void *private_data);
  senscord::Status UnregisterSyncFrameCallback();
  senscord::Status RegisterEventCallback(const std::string &event_type,
      const SynchronizerOnEventReceivedCallback callback, void *private_data);
  senscord::Status UnregisterEventCallback(const std::string &event_type);

 private:
  // without mutex lock
  senscord::Status UnregisterEventCallbackInternal(
      const std::string &event_type);

  // SourceStreamCallbacks
  static int AcquireFrame(Synchronizer *_this, senscord::Stream *stream);
  static void SourceStreamFrameCallback(
      senscord::Stream *stream, void *private_data);
  static void SourceStreamEventCallbackOld(
      const std::string &event_type, const void *param, void *private_data);

  // Policy util
  class SyncPolicyApiCore : public SyncPolicyApi {
   private:
    Synchronizer *parent_;

   public:
    explicit SyncPolicyApiCore(Synchronizer *synchronizer);
    ~SyncPolicyApiCore();
    void SendSyncFrame(const SyncFrame &frame);
    void DropFrame(senscord::Stream *stream, senscord::Frame *frame,
        bool drop_frame_event);
    void RaiseEvent(senscord::Stream *stream, const std::string &event_type,
        const void *param);
    void SetProcessConfig(
        uint64_t polling_offset, uint64_t polling_period, bool apply_immediate);
    void GetSourceStreamList(std::vector<SyncStreamInfo> *stream_list);
    bool GetTimeStamp(
        senscord::Frame *frame, senscord::Stream *stream, uint64_t *timestamp);
  };

  // class vars
  SyncPolicyApiCore policy_api_;  // new ~ delete
  SyncPolicy *policy_;            // init ~ exit

  std::vector<SyncStreamInfo> stream_list_;  // init ~ exit
  std::map<senscord::Stream *, SyncStreamInfo *>
      stream_map_;  // index of stream_list_

  bool auto_start_;
  int getframe_error_count_;

  // sync frame queue
  uint32_t syncframe_queue_num_;
  std::deque<SyncFrame> syncframe_queue_;
  senscord::osal::OSMutex *syncframe_queue_mutex_;
  senscord::osal::OSCond *syncframe_queue_enqueued_cond_;

  // event queue
  struct EventQueueEntry {
    senscord::Stream *stream;
    std::string event_type;
    const void *param;

    EventQueueEntry(senscord::Stream *_stream, const std::string &_event_type,
        const void *_param)
        : stream(_stream), param(_param) {
      event_type = _event_type;
    }
  };
  std::deque<EventQueueEntry> event_queue_;
  senscord::osal::OSMutex *event_queue_mutex_;
  senscord::osal::OSCond *event_queue_cond_;

  // Thread things
  static senscord::osal::OSThreadResult ProcessFrameThread(void *arg);
  static senscord::osal::OSThreadResult UserFrameCallbackThread(void *arg);
  static senscord::osal::OSThreadResult UserEventCallbackThread(void *arg);
  senscord::osal::OSThread *process_frame_thread_;
  senscord::osal::OSThread *user_frame_callback_thread_;
  senscord::osal::OSThread *user_event_callback_thread_;

  // state defs
  // ref: stream_core
  enum SynchronizerState {
    kSynchronizerStateNoInit = 0,
    kSynchronizerStateReady,     // before Start
    kSynchronizerStateRunning,   // Start ~ Stop
    kSynchronizerStateWaitStop,  // wait threads stop
  };
  SynchronizerState state_;
  senscord::osal::OSMutex *state_mutex_;

  SynchronizerState GetState();
  void SetState(SynchronizerState new_state);

  // callback thins
  struct FrameCallbackConfig {
    SynchronizerOnFrameReceivedCallback func_ptr;
    void *private_data;
    senscord::osal::OSMutex *mutex;
  };
  FrameCallbackConfig frame_callback_;

  struct EventCallbackPrivateData {
    Synchronizer *synchronizer;
    senscord::Stream *stream;
  };

  struct EventCallbackConfig {
    SynchronizerOnEventReceivedCallback func_ptr;
    std::string event_type;
    void *user_private_data;
    std::vector<EventCallbackPrivateData *> source_private_datas;
  };
  std::map<std::string, EventCallbackConfig *> event_callbacks_;
  // event_callbacks_ key is event_type.

  // mutex target: event_callbacks_, event_callback_private_datas_
  senscord::osal::OSMutex *event_callbacks_mutex_;

  // ProcessFrame config
  struct ProcessFrameConfig {
    uint64_t offset;
    uint64_t period;
  };
  ProcessFrameConfig process_frame_config_;
  senscord::osal::OSMutex *process_frame_config_mutex_;
  senscord::osal::OSCond *process_frame_config_cond_;

  void CallEventCallback(EventQueueEntry *evt);
};

// macros
#define SYNCHRONIZER_LOG_ERROR(...)     SENSCORD_LOG_ERROR(__VA_ARGS__)
#define SYNCHRONIZER_LOG_WARNING(...)   SENSCORD_LOG_WARNING(__VA_ARGS__)
#define SYNCHRONIZER_LOG_INFO(...)      SENSCORD_LOG_INFO(__VA_ARGS__)
#define SYNCHRONIZER_LOG_DEBUG(...)     SENSCORD_LOG_DEBUG(__VA_ARGS__)

#define NS_PRINT_LONG "%" PRIu64 "ns (%" PRIu64 ".%03" PRIu64 "s)"
#define NS_PRINT_LONG_ARG(x) x, ns2ms(x) / 1000ULL, ns2ms(x) % 1000ULL

#define NS_PRINT "%" PRIu64 ".%03" PRIu64 "s"
#define NS_PRINT_ARG(x) ns2ms(x) / 1000ULL, ns2ms(x) % 1000ULL

#endif  // SENSCORD_SYNCHRONIZER_SYNCHRONIZER_H_
