/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "server/senscord_iwasmserver.h"

#include "wasm_export.h"

#define PRINT(...) fprintf(stdout, __VA_ARGS__)

#if 1
#define LOG_E(...) fprintf(stderr, __VA_ARGS__)
#define LOG_W(...) fprintf(stderr, __VA_ARGS__)
#define LOG_I(...) fprintf(stderr, __VA_ARGS__)
#define LOG_D(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOG_E(...)
#define LOG_W(...)
#define LOG_I(...)
#define LOG_D(...)
#endif

#define DEFAULT_ADDRESS     ("127.0.0.1")
#define DEFAULT_PORT        (8080U)
#define DEFAULT_THREAD_NUM  (4U)

extern uint32_t get_native_lib(
    char** module_name, NativeSymbol** native_symbols);
extern int init_native_lib(void);
extern void deinit_native_lib(void);

static int print_usage(void) {
  PRINT("Usage: senscord_iwasmserver [options]\n");
  PRINT("options:\n");
  PRINT(" --addr=address   Set server address, default is %s\n",
        DEFAULT_ADDRESS);
  PRINT(" --port=port      Set server port, default is %u\n",
        DEFAULT_PORT);
  PRINT(" --max-threads=n  "
        "Set maximum thread number per cluster, default is %u\n",
        DEFAULT_THREAD_NUM);
  return 1;
}

static int strto_uint16(const char* str, uint16_t* value) {
  char* end_ptr = NULL;
  uint64_t num = strtoull(str, &end_ptr, 10);
  if (end_ptr == str || end_ptr == NULL || *end_ptr != '\0') {
    return -1;
  }
  if (errno == ERANGE || num > UINT16_MAX) {
    return -1;
  }
  *value = (uint16_t)num;
  return 0;
}

static int strto_uint32(const char* str, uint32_t* value) {
  char* end_ptr = NULL;
  uint64_t num = strtoull(str, &end_ptr, 10);
  if (end_ptr == str || end_ptr == NULL || *end_ptr != '\0') {
    return -1;
  }
  if (errno == ERANGE || num > UINT32_MAX) {
    return -1;
  }
  *value = (uint32_t)num;
  return 0;
}

// native api: test_nanosleep
static int32_t test_nanosleep_wrapper(
    wasm_exec_env_t exec_env,
    uint64_t nanoseconds) {
  if (!wasm_runtime_begin_blocking_op(exec_env)) {
    return -1;
  }
  struct timespec ts;
  ts.tv_sec = (time_t)(nanoseconds / 1000000000);
  ts.tv_nsec = (long)(nanoseconds % 1000000000);
  int32_t ret = nanosleep(&ts, NULL);
  wasm_runtime_end_blocking_op(exec_env);
  return ret;
}

// native api: test_clock_gettime
static uint64_t test_clock_gettime_wrapper(
    wasm_exec_env_t exec_env) {
  struct timespec ts;
  int32_t ret = clock_gettime(CLOCK_MONOTONIC, &ts);
  if (ret != 0) {
    return 0;
  }
  return (uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
}

static NativeSymbol TEST_NATIVE_SYMBOLS[] = {
    EXPORT_WASM_API_WITH_SIG2(test_nanosleep, "(I)i"),
    EXPORT_WASM_API_WITH_SIG2(test_clock_gettime, "()I"),
};

/**
 * iwasmserver main
 */
int main(int argc, char* argv[]) {
  int main_result = -1;
  char address[32];
  uint16_t port = DEFAULT_PORT;
  uint32_t max_thread_num = DEFAULT_THREAD_NUM;
  int init_native_lib_result = -1;
  char* module_name = NULL;
  NativeSymbol* native_symbols = NULL;
  uint32_t n_native_symbols = 0;
  RuntimeInitArgs init_args;

  strncpy(address, DEFAULT_ADDRESS, sizeof(address));

  // get option
  for (--argc, ++argv; argc > 0 && argv[0][0] == '-'; --argc, ++argv) {
    if (strncmp(argv[0], "--addr=", 7) == 0) {
      char* ptr = &argv[0][7];
      if (*ptr == '\0') {
        return print_usage();
      }
      strncpy(address, ptr, sizeof(address) - 1);
      address[sizeof(address) - 1] = '\0';
    } else if (strncmp(argv[0], "--port=", 7) == 0) {
      char* ptr = &argv[0][7];
      if (strto_uint16(ptr, &port) != 0) {
        return print_usage();
      }
    } else if (strncmp(argv[0], "--max-threads=", 14) == 0) {
      char* ptr = &argv[0][14];
      if (strto_uint32(ptr, &max_thread_num) != 0) {
        return print_usage();
      }
    } else {
      return print_usage();
    }
  }
  PRINT("address        : %s\n", address);
  PRINT("port           : %" PRIu16 "\n", port);
  PRINT("max thread num : %u\n", max_thread_num);

  memset(&init_args, 0, sizeof(RuntimeInitArgs));
  init_args.mem_alloc_type = Alloc_With_Allocator;
  init_args.mem_alloc_option.allocator.malloc_func = malloc;
  init_args.mem_alloc_option.allocator.realloc_func = realloc;
  init_args.mem_alloc_option.allocator.free_func = free;
  init_args.max_thread_num = max_thread_num;

  LOG_D("wasm_runtime_full_init\n");
  if (!wasm_runtime_full_init(&init_args)) {
    PRINT("wasm_runtime_full_init() failed.\n");
    return -1;
  }

  do {
    LOG_D("wasm_runtime_register_natives(test)\n");
    if (!wasm_runtime_register_natives(
          "env", TEST_NATIVE_SYMBOLS,
          sizeof(TEST_NATIVE_SYMBOLS) / sizeof(NativeSymbol))) {
      PRINT("wasm_runtime_register_natives(test) failed.\n");
      break;
    }

    LOG_D("init_native_lib\n");
    init_native_lib_result = init_native_lib();
    if (init_native_lib_result != 0) {
      PRINT("init_native_lib failed.\n");
      break;
    }

    LOG_D("get_native_lib\n");
    n_native_symbols = get_native_lib(&module_name, &native_symbols);
    LOG_D("wasm_runtime_register_natives(senscord)\n");
    if (!wasm_runtime_register_natives(
        module_name, native_symbols, n_native_symbols)) {
      PRINT("wasm_runtime_register_natives(senscord) failed.\n");
      break;
    }

    main_result = senscord_iwasm_run_server(address, port);
  } while (0);

  if (native_symbols != NULL) {
    LOG_D("wasm_runtime_unregister_natives\n");
    wasm_runtime_unregister_natives(module_name, native_symbols);
  }

  if (init_native_lib_result == 0) {
    LOG_D("deinit_native_lib\n");
    deinit_native_lib();
  }

  LOG_D("wasm_runtime_destroy\n");
  wasm_runtime_destroy();

  return main_result;
}
