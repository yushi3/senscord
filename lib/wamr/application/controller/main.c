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

#include "controller/senscord_iwasmctrl.h"

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
#define DEFAULT_STACK_SIZE  (64 * 1024)
#define DEFAULT_HEAP_SIZE   (16 * 1024)

static int print_usage(void) {
  PRINT("Usage: senscord_iwasmctrl exec [options] wasm_path [args...]\n");
  PRINT("exec options:\n");
  PRINT(" --addr=address   Set server address, default is %s\n",
        DEFAULT_ADDRESS);
  PRINT(" --port=port      Set server port, default is %u\n",
        DEFAULT_PORT);
  PRINT(" --stack-size=n   "
        "Set maximum stack size in bytes, default is %u KB\n",
        DEFAULT_STACK_SIZE / 1024);
  PRINT(" --heap-size=n    "
        "Set maximum heap size in bytes, default is %u KB\n",
        DEFAULT_HEAP_SIZE / 1024);
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

/**
 * iwasmctrl main
 */
int main(int argc, char* argv[]) {
  int main_result = -1;
  char address[32];
  uint16_t port = DEFAULT_PORT;
  uint32_t stack_size = DEFAULT_STACK_SIZE;
  uint32_t heap_size = DEFAULT_HEAP_SIZE;
  char* wasm_path = NULL;
  RuntimeInitArgs init_args;

  strncpy(address, DEFAULT_ADDRESS, sizeof(address));

  // skip executable name & "exec" argument
  --argc, ++argv;
  if ((argc > 0) && (strncmp(argv[0], "exec", 4) == 0)) {
    --argc, ++argv;
  }
  // get option
  for (; argc > 0 && argv[0][0] == '-'; --argc, ++argv) {
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
    } else if (strncmp(argv[0], "--stack-size=", 13) == 0) {
      char* ptr = &argv[0][13];
      if (strto_uint32(ptr, &stack_size) != 0) {
        return print_usage();
      }
    } else if (strncmp(argv[0], "--heap-size=", 12) == 0) {
      char* ptr = &argv[0][12];
      if (strto_uint32(ptr, &heap_size) != 0) {
        return print_usage();
      }
    } else {
      return print_usage();
    }
  }
  if (argc == 0) {
    return print_usage();
  }
  wasm_path = argv[0];
  PRINT("address        : %s\n", address);
  PRINT("port           : %" PRIu16 "\n", port);
  PRINT("wasm path      : [%s]\n", wasm_path);
  PRINT("stack size     : %" PRIu32 "\n", stack_size);
  PRINT("heap size      : %" PRIu32 "\n", heap_size);

  memset(&init_args, 0, sizeof(RuntimeInitArgs));
  init_args.mem_alloc_type = Alloc_With_Allocator;
  init_args.mem_alloc_option.allocator.malloc_func = malloc;
  init_args.mem_alloc_option.allocator.realloc_func = realloc;
  init_args.mem_alloc_option.allocator.free_func = free;

  LOG_D("wasm_runtime_full_init\n");
  if (!wasm_runtime_full_init(&init_args)) {
    PRINT("wasm_runtime_full_init() failed.\n");
    return -1;
  }

  main_result = senscord_iwasm_send_exec_parameter(
      address, port, wasm_path,
      stack_size, heap_size,
      argc, argv);

  LOG_D("wasm_runtime_destroy\n");
  wasm_runtime_destroy();

  return main_result;
}
