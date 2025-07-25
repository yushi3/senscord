/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "controller/senscord_iwasmctrl.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "senscord/osal.h"
#include "common/senscord_iwasm_common.h"

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

static uint8_t* read_file_to_buffer(
    const char* file_path, uint32_t* size) {
  FILE* file = NULL;
  struct stat stat;
  uint32_t file_size = 0;
  uint32_t read_size = 0;
  uint8_t* buffer = NULL;
  file = fopen(file_path, "rb");
  if (file == NULL) {
    PRINT("Open wasm file failed.\n");
    return NULL;
  }
  if (fstat(fileno(file), &stat) != 0) {
    PRINT("Get wasm file size failed.\n");
    fclose(file);
    return NULL;
  }
  file_size = static_cast<uint32_t>(stat.st_size);
  // PRINT("wasm file: %s, size=%d\n", file_path, file_size);
  buffer = reinterpret_cast<uint8_t*>(malloc(file_size));
  if (buffer == NULL) {
    PRINT("Allocate memory failed.\n");
    fclose(file);
    return NULL;
  }
  read_size = static_cast<uint32_t>(fread(buffer, 1, file_size, file));
  fclose(file);
  if (read_size < file_size) {
    PRINT("Read wasm file failed.\n");
    free(buffer);
    return NULL;
  }
  *size = file_size;
  return buffer;
}

/**
 * @brief Send exec command.
 * @param[in] address  Server address.
 * @param[in] port  Server port.
 * @param[in] wasm_path  Path to the WASM module.
 * @param[in] stack_size  Stack size of the module instance.
 * @param[in] heap_size  Heap size of module instance.
 * @param[in] argc  Number of elements in argv.
 * @param[in] argv  List of command line arguments.
 * @return 0 indicates success, while a negative number indicates failure.
 */
extern "C" int32_t senscord_iwasm_send_exec_parameter(
    const char* address, uint16_t port,
    const char* wasm_path,
    uint32_t stack_size, uint32_t heap_size,
    int32_t argc, char** argv) {
  // socket
  int32_t socket = ::socket(AF_INET, SOCK_STREAM, 0);
  if (socket < 0) {
    PRINT("socket() failed: %s\n", strerror(errno));
    return -1;
  }

  uint8_t* wasm_data = NULL;
  int32_t ret = -1;
  do {
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_aton(address, &addr.sin_addr) == 0) {
      PRINT("inet_aton() failed: address=%s\n", address);
      break;
    }

    // connect
    if (connect(socket, reinterpret_cast<const sockaddr*>(&addr),
                sizeof(addr)) != 0) {
      PRINT("connect() failed: %s\n", strerror(errno));
      break;
    }

    uint32_t file_size = 0;
    wasm_data = read_file_to_buffer(wasm_path, &file_size);
    if (wasm_data == NULL) {
      PRINT("Open wasm file failed: '%s'\n", wasm_path);
      break;
    }
    LOG_D("wasm file: %s, size=%" PRIu32 "\n", wasm_path, file_size);

    // Argument concatenation
    std::vector<uint8_t> args;
    {
      size_t args_size = 0;
      for (int32_t i = 0; i < argc; ++i) {
        args_size += strlen(argv[i]) + 1;  // +1 for '\0'
      }
      args.resize(args_size);
      size_t offset = 0;
      for (int32_t i = 0; i < argc; ++i) {
        size_t len = strlen(argv[i]) + 1;
        memcpy(&args[offset], argv[i], len);
        offset += len;
      }
    }
    LOG_D("args_size=%" PRIuS "\n", args.size());

    Header header = {};
    memcpy(&header.signature, kHeaderSignature, sizeof(kHeaderSignature));
    header.payload_size = htonl(
        static_cast<uint32_t>(sizeof(ExecHeader) + file_size + args.size()));
    ExecHeader exec_header = {};
    memcpy(&exec_header.type, kCommandTypeExec, sizeof(kCommandTypeExec));
    exec_header.stack_size = htonl(stack_size);
    exec_header.heap_size = htonl(heap_size);
    exec_header.module_data_size = htonl(file_size);
    exec_header.args_size = htonl(static_cast<uint32_t>(args.size()));

    iovec vec[4];
    vec[0].iov_base = &header;
    vec[0].iov_len = sizeof(header);
    vec[1].iov_base = &exec_header;
    vec[1].iov_len = sizeof(exec_header);
    vec[2].iov_base = wasm_data;
    vec[2].iov_len = file_size;
    vec[3].iov_base = &args[0];
    vec[3].iov_len = args.size();
    msghdr msg = {};
    msg.msg_iov = vec;
    msg.msg_iovlen = 4;

    // send
    int32_t flags = 0;
    flags |= MSG_NOSIGNAL;  // Do not generate SIGPIPE.
    ssize_t size = sendmsg(socket, &msg, flags);
    if (size < 0) {
      PRINT("sendmsg() failed: %s\n", strerror(errno));
      break;
    }

    ret = 0;  // success
  } while (false);

  if (wasm_data != NULL) {
    free(wasm_data);
  }

  if (socket != -1) {
    close(socket);
  }

  return ret;
}
