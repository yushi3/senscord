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
// static const char kDefaultStreamKey[] = "webcam_image_stream.0";
static const char kDefaultStreamKey[] = "pseudo_image_stream.0";
static const uint64_t kDefaultGetFrameCount = 20;
static const uint32_t kGetFrameWaitMsec = 3000;

// execute values.
static const char* stream_key_char_ = kDefaultStreamKey;
static const char* format_type_char_ = "raw";
static const char* output_path_char_ = ".";
static const char* name_rules_char_ = "";
static uint64_t getframe_count_ = kDefaultGetFrameCount;
static bool no_vendor_ = false;
static bool silent_ = false;

// parse argumens.
static int ParseArguments(int argc, const char* argv[]);

// main
int main(int argc, const char* argv[]) {
  TEST_PRINT("=== SensCord Stream Recorder ===\n");

  std::string stream_key;

  // parse arguments.
  {
    int ret = ParseArguments(argc, argv);
    if (ret < 0) {
      TEST_PRINT("Usage: %s "
          "[-k stream_key]"
          "[-f format]"
          "[-o output_path]"
          "[-n getframe_num]"
          "[-t top_directory name_rule]"
          "[--no-vendor]"
          "[--silent]\n",
          argv[0]);
      return -1;
    }

    TEST_PRINT(" - stream key: \"%s\"\n", stream_key_char_);
    TEST_PRINT(" - format: \"%s\"\n", format_type_char_);
    TEST_PRINT(" - output path: \"%s\"\n", output_path_char_);
    TEST_PRINT(" - top directory name rule: \"%s\"\n", name_rules_char_);
    TEST_PRINT(" - get frame count: %" PRIu64 "\n", getframe_count_);
    TEST_PRINT(" - enabled vendor's channels: %d\n", !no_vendor_);
    TEST_PRINT(" - enabled silent: %d\n", silent_);

    // set executing values.
    stream_key = stream_key_char_;
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

  // get formats
  {
    senscord::RecorderListProperty list = {};
    status = stream->GetProperty(senscord::kRecorderListPropertyKey, &list);
    TEST_PRINT("GetProperty(%s): status=%s\n",
        senscord::kRecorderListPropertyKey, status.ToString().c_str());
    if (!status.ok()) {
      return -1;
    }
    std::vector<std::string>::const_iterator itr = list.formats.begin();
    std::vector<std::string>::const_iterator end = list.formats.end();
    for (; itr != end; ++itr) {
      TEST_PRINT(" - type : \"%s\"\n", itr->c_str());
    }
  }

  // get channel info (total channel num)
  senscord::ChannelInfoProperty channel_info = {};
  status = stream->GetProperty(
      senscord::kChannelInfoPropertyKey, &channel_info);
  TEST_PRINT("GetProperty(): status=%s\n", status.ToString().c_str());
  if (!status.ok()) {
    return -1;
  }
  TEST_PRINT(" - channel num: %" PRIdS "\n", channel_info.channels.size());

  // start stream.
  status = stream->Start();
  TEST_PRINT("Start(): status=%s\n", status.ToString().c_str());
  if (!status.ok()) {
    return -1;
  }

  // start recording
  {
    senscord::RecordProperty rec = {};
    rec.enabled = true;
    rec.path = output_path_char_;
    rec.buffer_num = 5;
    rec.name_rules[senscord::kRecordDirectoryTop] = name_rules_char_;

    // channels
    std::map<uint32_t, senscord::ChannelInfo>::const_iterator itr =
        channel_info.channels.begin();
    std::map<uint32_t, senscord::ChannelInfo>::const_iterator end =
        channel_info.channels.end();
    for (; itr != end; ++itr) {
      if (no_vendor_ && itr->first >= senscord::kChannelIdVendorBase) {
        continue;
      }
      rec.formats[itr->first] = format_type_char_;
    }
    if (rec.formats.size() == 0) {
      TEST_PRINT("no recording target.\n");
      return -1;
    }

    status = stream->SetProperty(senscord::kRecordPropertyKey, &rec);
    TEST_PRINT("SetProperty(%s): status=%s\n",
        senscord::kRecordPropertyKey, status.ToString().c_str());
    if (!status.ok()) {
      return -1;
    }
  }
  TEST_PRINT("Start recording.\n");

  // get frames
  for (uint64_t cnt = 0; cnt < getframe_count_; ++cnt) {
    // get frame
    Frame* frame = NULL;
    status = stream->GetFrame(&frame, kGetFrameWaitMsec);
    if (!silent_) {
      TEST_PRINT("GetFrame(): status=%s\n", status.ToString().c_str());
    }
    if (status.ok()) {
      // release frame.
      status = stream->ReleaseFrame(frame);
      if (!silent_) {
        TEST_PRINT("ReleaseFrame(): status=%s\n", status.ToString().c_str());
      }
    }
  }
  TEST_PRINT("Done.\n");

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

  TEST_PRINT("=== SensCord Recorder End ===\n");

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
    } else if (arg == "-n") {
      // count of getframe
      if (next != NULL) {
        senscord::osal::OSStrtoull(next, NULL, 0, &getframe_count_);
        ++i;
      }
    } else if (arg == "-f") {
      // recording format
      if (next != NULL) {
        format_type_char_ = next;
        ++i;
      }
    } else if (arg == "-o") {
      // output path
      if (next != NULL) {
        output_path_char_ = next;
        ++i;
      }
    } else if (arg == "-t") {
      // name_rules
      if (next != NULL) {
        name_rules_char_ = next;
        ++i;
      }
    } else if (arg == "--no-vendor") {
      no_vendor_ = true;
    } else if (arg == "--silent") {
      silent_ = true;
    } else {
      // other input
      return -1;
    }
  }
  return 0;
}
