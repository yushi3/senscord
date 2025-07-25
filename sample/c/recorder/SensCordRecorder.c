/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>      /* printf */
#include <stdlib.h>     /* malloc, free */
#include <string.h>     /* strcat, strcmp */
#include <inttypes.h>   /* PRIu64 */

#include "senscord/c_api/senscord_c_api.h"
#include "senscord/osal_inttypes.h"

/* ===============================================================
 * Default values.
 * =============================================================== */
#define DEFAULT_STREAM_KEY             ("pseudo_image_stream.0")
#define DEFAULT_GET_FRAME_COUNT        (20)
#define GET_FRAME_WAIT_MSEC            (3000)

/* ===============================================================
 * Macros
 * =============================================================== */
#ifndef __wasm__
#ifdef _WIN32
#define TEST_PRINT(...) \
  do { printf_s("[L%d] ", __LINE__); printf_s(__VA_ARGS__); } while (0)
#define STRNCPY(dst, src, size)  strncpy_s(dst, size, src, size)
#else
#define TEST_PRINT(...) \
  do { printf("[L%d] ", __LINE__); printf(__VA_ARGS__); } while (0)
#define STRNCPY(dst, src, size)  strncpy(dst, src, size)
#endif  /* _WIN32 */
#define TEST_STRTOULL(str, endptr, base) strtoull(str, endptr, base)
#else
#define TEST_PRINT(...) \
  do { printf("[L%d] ", __LINE__); printf(__VA_ARGS__); } while (0)
#define STRNCPY(dst, src, size)  strncpy(dst, src, size)
#define TEST_STRTOULL(str, endptr, base) strtoul(str, endptr, base)
#endif  /* __wasm__ */

// execute values.
static const char* stream_key_ = NULL;
static const char* format_type_char_ = "raw";
static const char* output_path_char_ = ".";
static const char* name_rules_char_ = "";
static uint64_t getframe_count_ = 0;
static int32_t no_vendor_flag_ = 0;  // record user channel. 0=OFF, other=ON
static int32_t silent_flag_ = 0;     // get frame log. 0=OFF, other=ON

// print error status.
static void PrintError(void);
// parse argumens.
static int ParseArguments(int argc, const char* argv[]);

// main
int main(int argc, const char* argv[]) {
  int32_t ret = 0;
  senscord_core_t core;
  senscord_stream_t stream;
  struct senscord_channel_info_property_t channel_info = {0};

  TEST_PRINT("=== SensCord Stream Recorder ===\n");

  // parse arguments.
  {
    ret = ParseArguments(argc, argv);
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

    /* set executing values. */
    if (stream_key_ == NULL) {
      stream_key_ = DEFAULT_STREAM_KEY;
    }

    if (getframe_count_ == 0) {
      getframe_count_ = DEFAULT_GET_FRAME_COUNT;
    }

    TEST_PRINT(" - stream key: \"%s\"\n", stream_key_);
    TEST_PRINT(" - format: \"%s\"\n", format_type_char_);
    TEST_PRINT(" - output path: \"%s\"\n", output_path_char_);
    TEST_PRINT(" - top directory name rule: \"%s\"\n", name_rules_char_);
    TEST_PRINT(" - get frame count: %" PRIu64 "\n", getframe_count_);
    TEST_PRINT(" - enabled vendor's channels: %d\n", !no_vendor_flag_);
    TEST_PRINT(" - enabled silent: %d\n", silent_flag_);
  }

  // init Core
  ret = senscord_core_init(&core);
  TEST_PRINT("senscord_core_init(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  // get version
  {
    struct senscord_version_t version;
    ret = senscord_core_get_version(core, &version);
    TEST_PRINT("senscord_core_get_version(): ret=%" PRId32 ", version=%s "
               "%" PRIu32 ".%" PRIu32 ".%" PRIu32 " %s\n", ret,
        version.senscord_version.name,
        version.senscord_version.major,
        version.senscord_version.minor,
        version.senscord_version.patch,
        version.senscord_version.description);
    if (ret != 0) {
      PrintError();
      return -1;
    }
  }

  // open stream.
  ret = senscord_core_open_stream(core, stream_key_, &stream);
  TEST_PRINT("senscord_core_open_stream(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  // get formats
  {
    uint32_t index = 0;
    struct senscord_recorder_list_property_t list = {0};
    ret = senscord_stream_get_property(
        stream, SENSCORD_RECORDER_LIST_PROPERTY_KEY,
        &list, sizeof(list));
    TEST_PRINT("senscord_stream_get_property(%s): ret=%" PRId32 "\n",
               SENSCORD_RECORDER_LIST_PROPERTY_KEY, ret);
    if (ret != 0) {
      PrintError();
      return -1;
    }
    for (; index != list.count; ++index) {
      TEST_PRINT(" - type : \"%s\"\n", list.formats[index].name);
    }
  }

  // get channel info (total channel num)
  ret = senscord_stream_get_property(
      stream, SENSCORD_CHANNEL_INFO_PROPERTY_KEY,
      &channel_info, sizeof(channel_info));
  TEST_PRINT("senscord_stream_get_property(%s): ret=%" PRId32 "\n",
             SENSCORD_CHANNEL_INFO_PROPERTY_KEY, ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }
  TEST_PRINT(" - channel num: %" PRIu32 "\n", channel_info.count);

  // start stream.
  ret = senscord_stream_start(stream);
  TEST_PRINT("senscord_stream_start(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  // start recording
  {
    uint32_t index = 0;
    struct senscord_record_property_t rec = {0};
    rec.enabled = 1;  // ON
    STRNCPY(rec.path, output_path_char_, sizeof(rec.path));
    rec.buffer_num = 5;

    // channels
    for (; index < channel_info.count; ++index) {
      uint32_t channel_id = channel_info.channels[index].channel_id;
      // TODO: C API (kChannelIdVendorBase)
      if ((no_vendor_flag_ != 0) && (channel_id >= 0x80000000)) {
        continue;
      }
      rec.info_array[index].channel_id = channel_id;
      STRNCPY(rec.info_array[index].format.name, format_type_char_,
              sizeof(rec.info_array[index].format.name));
      ++rec.info_count;
    }
    if (rec.info_count == 0) {
      TEST_PRINT("no recording target.\n");
      return -1;
    }
    // name rules
    rec.name_rules_count = 1;
    STRNCPY(rec.name_rules[0].directory_type, SENSCORD_RECORD_DIRECTORY_TOP,
            sizeof(rec.name_rules[0].directory_type));
    STRNCPY(rec.name_rules[0].format, name_rules_char_,
            sizeof(rec.name_rules[0].format));

    ret = senscord_stream_set_property(
        stream, SENSCORD_RECORD_PROPERTY_KEY,
        &rec, sizeof(rec));
    TEST_PRINT("senscord_stream_set_property(%s): ret=%" PRId32 "\n",
               SENSCORD_RECORD_PROPERTY_KEY, ret);
    if (ret != 0) {
      return -1;
    }
  }
  TEST_PRINT("Start recording.\n");

  {
    // get frames
    uint64_t cnt = 0;
    for (; cnt < getframe_count_; ++cnt) {
      // get frame
      senscord_frame_t frame;
      ret = senscord_stream_get_frame(stream, &frame, GET_FRAME_WAIT_MSEC);
      if (silent_flag_ == 0) {
        TEST_PRINT("senscord_stream_get_frame(): ret=%" PRId32 "\n", ret);
      }
      if (ret == 0) {
        // release frame.
        ret = senscord_stream_release_frame(stream, frame);
        if (silent_flag_ == 0) {
          TEST_PRINT("senscord_stream_release_frame(): ret=%" PRId32 "\n",
                     ret);
        }
      }
    }
  }
  TEST_PRINT("senscord_stream_get_frame(s) done!\n");

  // stop stream.
  ret = senscord_stream_stop(stream);
  TEST_PRINT("senscord_stream_stop(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  // close stream.
  ret = senscord_core_close_stream(core, stream);
  TEST_PRINT("senscord_core_close_stream(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  // exit Core
  ret = senscord_core_exit(core);
  TEST_PRINT("senscord_core_exit(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  TEST_PRINT("=== SensCord Stream Recorder End ===\n");
  return 0;
}

/**
 * @brief Print status.
 */
static void PrintError(void) {
  enum senscord_error_cause_t cause = senscord_get_last_error_cause();
  if (cause == SENSCORD_ERROR_NONE) {
    TEST_PRINT("status: OK\n");
  } else {
    enum senscord_error_level_t level = senscord_get_last_error_level();
    TEST_PRINT("status: level=%d, cause=%d\n", level, cause);
    char buffer[256];
    uint32_t length = 0;
    // message
    length = sizeof(buffer);
    if (senscord_get_last_error_string(
        SENSCORD_STATUS_PARAM_MESSAGE, buffer, &length) == 0) {
      TEST_PRINT(" - message: %s\n", buffer);
    }
    // block
    length = sizeof(buffer);
    if (senscord_get_last_error_string(
        SENSCORD_STATUS_PARAM_BLOCK, buffer, &length) == 0) {
      TEST_PRINT(" - block  : %s\n", buffer);
    }
    // trace
    length = sizeof(buffer);
    if (senscord_get_last_error_string(
        SENSCORD_STATUS_PARAM_TRACE, buffer, &length) == 0) {
      TEST_PRINT(" - trace  : %s\n", buffer);
    }
  }
}

/**
 * @brief Parse the application's arguments.
 * @param[in] (argc) The number of arguments.
 * @param[in] (argv) The argumens.
 * @return 0 is success.
 */
static int ParseArguments(int argc, const char* argv[]) {
  int i = 0;
  // parse
  for (i = 1; i < argc; ++i) {
    const char* next = NULL;
    if ((i + 1) < argc) {
      next = argv[i + 1];
    }

    if (strcmp(argv[i], "-k") == 0) {
      // stream key
      if (next != NULL) {
        stream_key_ = next;
        ++i;
      }
    } else if (strcmp(argv[i], "-n") == 0) {
      // count of getframe
      if (next != NULL) {
        getframe_count_ = TEST_STRTOULL(next, NULL, 0);
        ++i;
      }
    } else if (strcmp(argv[i], "-f") == 0) {
      // recording format
      if (next != NULL) {
        format_type_char_ = next;
        ++i;
      }
    } else if (strcmp(argv[i], "-o") == 0) {
      // output path
      if (next != NULL) {
        output_path_char_ = next;
        ++i;
      }
    } else if (strcmp(argv[i], "-t") == 0) {
      // name_rules
      if (next != NULL) {
        name_rules_char_ = next;
        ++i;
      }
    } else if (strcmp(argv[i], "--no-vendor") == 0) {
      no_vendor_flag_ = 1;  // ON
    } else if (strcmp(argv[i], "--silent") == 0) {
      silent_flag_ = 1;  // ON
    } else {
      // other input
      return -1;
    }
  }
  return 0;
}
