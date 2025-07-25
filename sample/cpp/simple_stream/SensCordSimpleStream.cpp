/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>  // PRIu64
#include <vector>
#include <string>
#include <algorithm>    // std::find

#include "senscord/senscord.h"
#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"

// using
using senscord::Core;
using senscord::Stream;
using senscord::Frame;
using senscord::Channel;
using senscord::Status;

// for print wrapper
#define TEST_PRINT(...) \
  do { senscord::osal::OSPrintf("[L%d] ", __LINE__); \
       senscord::osal::OSPrintf(__VA_ARGS__); } while (0)

// default values.
static const char kDefaultStreamKey[] = "pseudo_image_stream.0";
static const uint64_t kDefaultGetFrameCount = 20;
static const uint32_t kGetFrameWaitMsec = 3000;

// execute values.
static const char* stream_key_char_ = NULL;
static uint64_t getframe_count_ = 0;

// parse argumens.
static int ParseArguments(int argc, const char* argv[]);

// main
int main(int argc, const char* argv[]) {
  TEST_PRINT("=== SensCordSimpleStream Player ===\n");

  std::string stream_key;

  // parse arguments.
  {
    int ret = ParseArguments(argc, argv);
    if (ret < 0) {
      TEST_PRINT(
          "Usage: %s [-k stream_key][-f getframe_num]\n", argv[0]);
      return -1;
    }

    // set executing values.
    if (stream_key_char_ != NULL) {
      stream_key = stream_key_char_;
    } else {
      stream_key = kDefaultStreamKey;
    }

    if (getframe_count_ == 0) {
      getframe_count_ = kDefaultGetFrameCount;
    }

    TEST_PRINT(" - stream key: \"%s\"\n", stream_key.c_str());
    TEST_PRINT(" - get frame count: %" PRIu64 "\n", getframe_count_);
  }

  Status status;
  Core core;

  // init Core
  status = core.Init();
  TEST_PRINT("Init(): status=%s\n", status.ToString().c_str());
  if (!status.ok()) {
    return -1;
  }

  // get version
  {
    senscord::SensCordVersion version;
    status = core.GetVersion(&version);
    TEST_PRINT("GetVersion(): status=%s, version=%s %" PRIu32 ".%" PRIu32
               ".%" PRIu32 " %s\n",
        status.ToString().c_str(),
        version.senscord_version.name.c_str(),
        version.senscord_version.major,
        version.senscord_version.minor,
        version.senscord_version.patch,
        version.senscord_version.description.c_str());
    if (!status.ok()) {
      return -1;
    }
  }
  // open stream.
  Stream* stream = NULL;
  status = core.OpenStream(stream_key, &stream);
  TEST_PRINT("OpenStream(): status=%s, stream=%p\n",
             status.ToString().c_str(), stream);
  if (!status.ok()) {
    return -1;
  }

  // get and print the properties.
  {
    std::vector<std::string> list;
    status = stream->GetPropertyList(&list);
    TEST_PRINT("GetPropertyList(): status=%s, size=%" PRIuS "\n",
                status.ToString().c_str(), list.size());
    if (!status.ok()) {
      return -1;
    }
    for (uint32_t i = 0; i < list.size(); ++i) {
      TEST_PRINT(" - %" PRIu32 ": key=%s\n", i, list[i].c_str());
    }
  }

  // start stream.
  status = stream->Start();
  TEST_PRINT("Start(): status=%s\n", status.ToString().c_str());
  if (!status.ok()) {
    return -1;
  }

  // get frames
  for (uint64_t cnt = 0; cnt < getframe_count_; ++cnt) {
    // get frame
    Frame* frame = NULL;
    status = stream->GetFrame(&frame, kGetFrameWaitMsec);
    if (status.ok()) {
      // sequence number
      uint64_t sequence_number = 0;
      frame->GetSequenceNumber(&sequence_number);
      TEST_PRINT("GetFrame(): status=%s, seq_num=%" PRIu64 "\n",
          status.ToString().c_str(), sequence_number);

      // get channel
      senscord::ChannelList list;
      status = frame->GetChannelList(&list);
      TEST_PRINT(" - GetChannelList(): status=%s, size=%" PRIuS "\n",
                  status.ToString().c_str(), list.size());

      // release frame.
      status = stream->ReleaseFrame(frame);
      TEST_PRINT("ReleaseFrame(): status=%s\n", status.ToString().c_str());
    } else {
      TEST_PRINT("GetFrame(): status=%s\n", status.ToString().c_str());
    }
  }
  TEST_PRINT("GetFrames done.\n");

  // stop stream.
  status = stream->Stop();
  TEST_PRINT("Stop(): status=%s\n", status.ToString().c_str());
  if (!status.ok()) {
    return -1;
  }

  // close stream.
  status = core.CloseStream(stream);
  TEST_PRINT("CloseStream(): status=%s\n", status.ToString().c_str());
  if (!status.ok()) {
    return -1;
  }

  // exit Core
  status = core.Exit();
  TEST_PRINT("Exit(): status=%s\n", status.ToString().c_str());
  if (!status.ok()) {
    return -1;
  }

  TEST_PRINT("=== SensCordSimpleStream End ===\n");

  // for valgrind, wait 1 sec for free all detached thread resources.
  senscord::osal::OSSleep(static_cast<uint64_t>(1) * 1000 * 1000 * 1000);
  return 0;
}

/**
 * @brief Parse the application's arguments.
 * @param[in] (argc) The number of arguments.
 * @param[in] (argv) The argumens.
 * @param[out] (port) Port number.
 * @param[out] (is_enable_client) Flag for client enabling.
 * @return 0 is success.
 */
static int ParseArguments(int argc, const char* argv[]) {
  // parse
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    const char* next = NULL;
    // std::string next;
    if ((i + 1) < argc) {
      next = argv[i + 1];
    }

    if (arg == "-k") {
      // stream key
      if (next != NULL) {
        stream_key_char_ = next;
        ++i;
      }
    } else if (arg == "-f") {
      // count of getframe
      if (next != NULL) {
        senscord::osal::OSStrtoull(next, NULL, 0, &getframe_count_);
        ++i;
      }
    } else {
      // other input
      return -1;
    }
  }
  return 0;
}
