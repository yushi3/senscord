/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <pthread.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "senscord/c_api/senscord_c_api.h"
#include "senscord/osal_inttypes.h"

#define PRINT(...) printf(__VA_ARGS__)

static const int32_t THREAD_NUM = 2;
static const char STREAM_KEY[] = "pseudo_image_stream.0";
static const int32_t LOOP_COUNT = 100;

static void* thread_func(void* args) {
  int32_t ret = 0;
  senscord_core_t core;
  senscord_stream_t stream;
  senscord_frame_t frame;
  uint64_t seq_num = 0;
  int32_t i = 0;

  ret = senscord_core_init(&core);
  if (ret != 0) {
    PRINT("[tid=%d] senscord_core_init(): cause=%d\n",
          (int)pthread_self(), senscord_get_last_error_cause());
    return NULL;
  }

  ret = senscord_core_open_stream(core, STREAM_KEY, &stream);
  if (ret != 0) {
    PRINT("[tid=%d] senscord_core_open_stream(): cause=%d\n",
          (int)pthread_self(), senscord_get_last_error_cause());
    return NULL;
  }

  ret = senscord_stream_start(stream);
  if (ret != 0) {
    PRINT("[tid=%d] senscord_stream_start(): cause=%d\n",
          (int)pthread_self(), senscord_get_last_error_cause());
    return NULL;
  }

  for (i = 0; i < LOOP_COUNT; ++i) {
    ret = senscord_stream_get_frame(stream, &frame, SENSCORD_TIMEOUT_FOREVER);
    if (ret != 0) {
      PRINT("[tid=%d] senscord_stream_get_frame(): cause=%d\n",
            (int)pthread_self(), senscord_get_last_error_cause());
      break;
    }

    ret = senscord_frame_get_sequence_number(frame, &seq_num);
    if (ret != 0) {
      PRINT("[tid=%d] senscord_frame_get_sequence_number(): cause=%d\n",
            (int)pthread_self(), senscord_get_last_error_cause());
      return NULL;
    }
    PRINT("[tid=%d] seq_num=%" PRIu64 "\n", (int)pthread_self(), seq_num);

    ret = senscord_stream_release_frame(stream, frame);
    if (ret != 0) {
      PRINT("[tid=%d] senscord_stream_release_frame(): cause=%d\n",
            (int)pthread_self(), senscord_get_last_error_cause());
      return NULL;
    }
  }

  ret = senscord_stream_stop(stream);
  if (ret != 0) {
    PRINT("[tid=%d] senscord_stream_stop(): cause=%d\n",
          (int)pthread_self(), senscord_get_last_error_cause());
    return NULL;
  }

  ret = senscord_core_close_stream(core, stream);
  if (ret != 0) {
    PRINT("[tid=%d] senscord_core_close_stream(): cause=%d\n",
          (int)pthread_self(), senscord_get_last_error_cause());
    return NULL;
  }

  ret = senscord_core_exit(core);
  if (ret != 0) {
    PRINT("[tid=%d] senscord_core_exit(): cause=%d\n",
          (int)pthread_self(), senscord_get_last_error_cause());
    return NULL;
  }

  return NULL;
}

int32_t main(int32_t argc, const char* argv[]) {
  int32_t ret = 0;
  pthread_t thread[THREAD_NUM];
  int i = 0;

  for (i = 0; i < THREAD_NUM; ++i) {
    ret = pthread_create(&thread[i], NULL, thread_func, NULL);
    if (ret != 0) {
      PRINT("pthread_create(): ret=%d\n", ret);
      exit(-1);
    }
  }

  for (i = 0; i < THREAD_NUM; ++i) {
    ret = pthread_join(thread[i], NULL);
    if (ret != 0) {
      PRINT("pthread_join(): ret=%d\n", ret);
      exit(-1);
    }
  }

  return 0;
}
