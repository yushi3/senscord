/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>     /* printf */
#include <stdlib.h>    /* malloc, free */
#include <string.h>    /* strcat, strcmp */
#include <inttypes.h>  /* PRIu64 */

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
#else
#define TEST_PRINT(...) \
  do { printf("[L%d] ", __LINE__); printf(__VA_ARGS__); } while (0)
#endif  /* _WIN32 */
#define TEST_STRTOULL(str, endptr, base) strtoull(str, endptr, base)
#else
#define TEST_PRINT(...) \
  do { printf("[L%d] ", __LINE__); printf(__VA_ARGS__); } while (0)
#define TEST_STRTOULL(str, endptr, base) strtoul(str, endptr, base)
#endif  /* __wasm__ */

/* execute values. */
static const char* stream_key_ = NULL;
static uint64_t getframe_count_ = 0;

static void PrintError(void);
static int ParseArguments(int argc, const char* argv[]);

/* main */
int main(int argc, const char* argv[]) {
  int32_t ret = 0;
  senscord_core_t core;
  senscord_stream_t stream;

  TEST_PRINT("=== SeneCordSimpleStream Player ===\n");

  /* parse arguments. */
  {
    ret = ParseArguments(argc, argv);
    if (ret < 0) {
      TEST_PRINT(
          "Usage: %s [-k stream_key][-f getframe_num]\n", argv[0]);
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
    TEST_PRINT(" - get frame count: %" PRIu64 "\n", getframe_count_);
  }

  /* init Core */
  ret = senscord_core_init(&core);
  TEST_PRINT("senscord_core_init(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  /* get version */
  {
    struct senscord_version_t version;
    ret = senscord_core_get_version(core, &version);
    TEST_PRINT("senscord_core_get_version(): ret=%" PRId32 "\n", ret);
    if (ret == 0) {
      TEST_PRINT(
          "version=%s %" PRIu32 ".%" PRIu32 ".%" PRIu32 " %s\n",
          version.senscord_version.name,
          version.senscord_version.major,
          version.senscord_version.minor,
          version.senscord_version.patch,
          version.senscord_version.description);
    } else {
      PrintError();
    }
  }

  /* open stream */
  ret = senscord_core_open_stream(core, stream_key_, &stream);
  TEST_PRINT("senscord_core_open_stream(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  /* get and print the properties. */
  {
    uint32_t count = 0;
    uint32_t index = 0;
    ret = senscord_stream_get_property_count(stream, &count);
    TEST_PRINT("senscord_stream_get_property_count(): "
               "ret=%" PRId32 ", count=%" PRIu32 "\n", ret, count);
    if (ret != 0) {
      PrintError();
      return -1;
    }
    for (index = 0; index < count; ++index) {
      char key[64];
      uint32_t length = sizeof(key);
      ret = senscord_stream_get_property_key_string(
          stream, index, key, &length);
      if (ret != 0) {
        TEST_PRINT("senscord_stream_get_property_key(): "
                   "%" PRIu32 ": failed. ret=%" PRId32 "\n", index, ret);
        PrintError();
        return -1;
      }
      TEST_PRINT(" - %" PRIu32 ": key=%s\n", index, key);
    }
  }

  /* start stream. */
  ret = senscord_stream_start(stream);
  TEST_PRINT("senscord_stream_start(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  uint64_t cnt = 0;
  for (cnt = 0; cnt < getframe_count_; ++cnt) {
    /* get frame */
    senscord_frame_t frame;
    ret = senscord_stream_get_frame(
        stream, &frame, GET_FRAME_WAIT_MSEC);
    TEST_PRINT("senscord_stream_get_frame(): ret=%" PRId32 "\n", ret);
    if (ret == 0) {
      uint32_t count = 0;
      uint32_t index = 0;
      uint64_t seq_num = 0;
      const char* type = NULL;
      ret = senscord_frame_get_sequence_number(frame, &seq_num);
      if (ret != 0) {
        TEST_PRINT("senscord_frame_get_sequence_number(): ret=%"
                   PRId32 "\n", ret);
        PrintError();
      }
      ret = senscord_frame_get_type(frame, &type);
      if (ret != 0) {
        TEST_PRINT("senscord_frame_get_type(): ret=%" PRId32 "\n", ret);
        PrintError();
      }
      ret = senscord_frame_get_channel_count(frame, &count);
      if (ret != 0) {
        TEST_PRINT("senscord_frame_get_channel_count(): ret=%" PRId32 "\n",
                   ret);
        PrintError();
      }
      TEST_PRINT("frame[%" PRIu64 "] type=%s, channels=%" PRIu32 "\n",
                 seq_num, type, count);

      for (index = 0; index < count; ++index) {
        senscord_channel_t channel;
        struct senscord_raw_data_t raw_data;
        ret = senscord_frame_get_channel(frame, index, &channel);
        TEST_PRINT("senscord_frame_get_channel(): "
                   "ret=%" PRId32 ", index=%" PRIu32 "\n", ret, index);

        ret = senscord_channel_get_raw_data(channel, &raw_data);
        TEST_PRINT("senscord_channel_get_raw_data(): ret=%" PRId32 "\n", ret);
        if (ret == 0) {
          TEST_PRINT("  - address   : %p\n", raw_data.address);
          TEST_PRINT("  - size      : %" PRIuS "\n", raw_data.size);
          TEST_PRINT("  - type      : %s\n", raw_data.type);
          TEST_PRINT("  - timestamp : %" PRIu64 "\n", raw_data.timestamp);
        } else {
          PrintError();
        }
      }

      ret = senscord_stream_release_frame(stream, frame);
      TEST_PRINT("senscord_stream_release_frame(): ret=%" PRId32 "\n", ret);
      if (ret != 0) {
        PrintError();
      }
    } else {
      PrintError();
    }
  }
  TEST_PRINT("senscord_stream_get_frame(s) done!\n");

  /* stop stream. */
  ret = senscord_stream_stop(stream);
  TEST_PRINT("senscord_stream_stop(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  /* close stream. */
  ret = senscord_core_close_stream(core, stream);
  TEST_PRINT("senscord_core_close_stream(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  /* exit Core */
  ret = senscord_core_exit(core);
  TEST_PRINT("senscord_core_exit(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  TEST_PRINT("=== SensCordSimpleStream End ===\n");

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
  /* parse */
  int i = 0;
  for (i = 1; i < argc; ++i) {
    const char* arg = argv[i];
    const char* next = NULL;
    if ((i + 1) < argc) {
      next = argv[i + 1];
    }

    if (strcmp(arg, "-k") == 0) {
      /* stream key */
      if (next != NULL) {
        stream_key_ = next;
        ++i;
      }
    } else if (strcmp(arg, "-f") == 0) {
      /* count of getframe */
      if (next != NULL) {
        getframe_count_ = TEST_STRTOULL(next, NULL, 0);
        ++i;
      }
    } else {
      /* other input */
      return -1;
    }
  }
  return 0;
}
