/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// SyncPolicyMasterSlave sample (polling)

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "senscord/osal.h"
#include "senscord/senscord.h"
#include "senscord/synchronizer/synchronizer.h"
#include "senscord/synchronizer/sync_policy_master_slave.h"

/////////////////////////////////////////////////////////////////////
// synchronizer configuration
/////////////////////////////////////////////////////////////////////
#define AUTO_START_ENABLE true
#define SYNC_FRAME_QUEUE_NUM 3

#define MASTER_STREAM_KEY "pseudo_image_stream.0"
#define SLAVE_STREAM_KEY_1 "pseudo_image_stream.0"
#define SLAVE_STREAM_KEY_2 "pseudo_image_stream.0"

// advanced configuration for SLAVE_STREAM_KEY_2
#define SLAVE_STREAM_2_MAIN_CH_ID 1
#define SLAVE_STREAM_2_DISABLE_EVENT senscord::kEventFrameDropped

/////////////////////////////////////////////////////////////////////
// Policy configuration
/////////////////////////////////////////////////////////////////////
#define OVERWRITE_MASTER_TIMESTAMP_ENABLE false
#define ONE_FRAME_PER_STREAM_ENABLE       true
#define WAIT_ALL_STREAMS_ON_START_ENABLE  true

#define ADDITIONAL_TIME_NS 5ULL * 1000 * 1000  // 5ms

// set time_range to one frame cycle of MASTER_STREAM_KEY.
#define FRAME_RATE 60ULL
#define SECOND_NS 1ULL * 1000 * 1000 * 1000  // 1s = 1,000,000,000ns
#define TIME_RANGE_NS (SECOND_NS / FRAME_RATE)

/////////////////////////////////////////////////////////////////////
// Sample Code
/////////////////////////////////////////////////////////////////////
void PrintSyncFrame(SyncFrame *syncframe);

#define TEST_PRINT(fmt, ...) \
  senscord::osal::OSPrintf(("[L%d] " fmt), __LINE__, ##__VA_ARGS__)

#define TEST_TIME_NS 5ULL * SECOND_NS  // 5sec

int main(int argc, char *argv[]) {
  TEST_PRINT("=== Synchronizer & SyncPolicyMasterSlave Sample ===\n");

  senscord::Status status;
  senscord::Core core;
  senscord::Stream *master_stream = NULL, *slave_stream1 = NULL,
                   *slave_stream2 = NULL;

  //===========================================
  // init core
  status = core.Init();
  if (!status.ok()) return -1;

  //===========================================
  // open streams
  status = core.OpenStream(MASTER_STREAM_KEY, &master_stream);
  if (!status.ok() || master_stream == NULL) return -1;

  status = core.OpenStream(SLAVE_STREAM_KEY_1, &slave_stream1);
  if (!status.ok() || slave_stream1 == NULL) return -1;

  status = core.OpenStream(SLAVE_STREAM_KEY_2, &slave_stream2);
  if (!status.ok() || slave_stream2 == NULL) return -1;

  //===========================================
  // initialize policy. pass the config to policy.
  SyncPolicyMasterSlave policy;
  status = policy.Init(TIME_RANGE_NS, ADDITIONAL_TIME_NS,
      OVERWRITE_MASTER_TIMESTAMP_ENABLE, ONE_FRAME_PER_STREAM_ENABLE,
      WAIT_ALL_STREAMS_ON_START_ENABLE);
  if (!status.ok()) return -1;

  //===========================================
  // create a configuration vector of Stream to be synchronized.
  Synchronizer synchronizer;
  std::vector<SyncStreamInfo> sync_streams;
  sync_streams.emplace_back(SyncStreamInfo(master_stream));
  sync_streams.emplace_back(SyncStreamInfo(slave_stream1));
  sync_streams.emplace_back(
      SyncStreamInfo(slave_stream2, SLAVE_STREAM_2_MAIN_CH_ID,
          SLAVE_STREAM_2_DISABLE_EVENT));  // advanced configuration for
                                           // slave_stream2

  //===========================================
  // initialize Synchronizer
  status = synchronizer.Init(
      &policy, sync_streams, AUTO_START_ENABLE, SYNC_FRAME_QUEUE_NUM);
  if (!status.ok()) return -1;

  //===========================================
  // start synchronizer (auto start streams)
  status = synchronizer.Start();
  if (!status.ok()) return -1;

  //===========================================
  // Frame collection loop
  SyncFrame syncframe;
  uint64_t start_time = 0, now_time = 0;
  uint64_t sync_frame_count = 0;
  senscord::osal::OSGetTime(&start_time);
  while (1) {
    TEST_PRINT("GetSyncFrame count:%d \n", sync_frame_count++);

    //===========================================
    // example of getting and using a SyncFrame
    status = synchronizer.GetSyncFrame(&syncframe, senscord::kTimeoutForever);
    if (!status.ok()) return -1;

    PrintSyncFrame(&syncframe);

    status = synchronizer.ReleaseSyncFrame(&syncframe);
    if (!status.ok()) return -1;

    //===========================================
    // check test time
    senscord::osal::OSGetTime(&now_time);
    if (now_time - start_time > TEST_TIME_NS) break;
  }

  //===========================================
  // stop synchronizer (auto stop streams)
  status = synchronizer.Stop();
  if (!status.ok()) return -1;

  //===========================================
  // deinitialize synchronizer & policy
  status = synchronizer.Exit();
  if (!status.ok()) return -1;

  status = policy.Exit();
  if (!status.ok()) return -1;

  //===========================================
  // close streams
  status = core.CloseStream(master_stream);
  if (!status.ok()) return -1;

  status = core.CloseStream(slave_stream1);
  if (!status.ok()) return -1;

  status = core.CloseStream(slave_stream2);
  if (!status.ok()) return -1;

  //===========================================
  // exit core
  status = core.Exit();
  if (!status.ok()) return -1;

  return 0;
}

void PrintSyncFrame(SyncFrame *syncframe) {
  senscord::Status status;
  senscord::StreamKeyProperty str_key;

  // loop of Stream in Syncframe
  for (auto &sp : *syncframe) {
    senscord::Stream *stream = sp.first;
    std::vector<SynchronizerFrameInfo> &frames = sp.second;

    // get stream key.
    status = stream->GetProperty(senscord::kStreamKeyPropertyKey, &str_key);
    if (!status.ok()) return;

    // print stream info
    TEST_PRINT(
        "  stream: %s (%dF)\n", str_key.stream_key.c_str(), frames.size());

    // loop of Frame in Stream
    for (auto &frame_info : frames) {
      // example of using senscord::Frame. get frame sequence number.
      uint64_t seqno = 0;
      status = frame_info.frame->GetSequenceNumber(&seqno);
      if (!status.ok()) return;

      // print frams info
      TEST_PRINT("    -> Frame*: %p, TimeStamp: %" PRIu64 ", SeqNo: %" PRIu64
                 "\n",
          frame_info.frame, frame_info.timestamp, seqno);
    }
  }

  TEST_PRINT("---end syncframe---\n\n");
  return;
}
