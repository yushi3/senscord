/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "server/senscord_iwasmserver.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include <string>
#include <vector>

#include "senscord/osal.h"
#include "common/senscord_iwasm_common.h"

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

static int32_t RecvWithTimeout(
    int32_t socket, void* buffer, uint32_t* recv_size, int64_t timeout_nsec) {
  int32_t ret = 0;
  uint8_t* ptr = reinterpret_cast<uint8_t*>(buffer);
  size_t total_received_size = 0;
  size_t remaining_size = *recv_size;
  *recv_size = 0;
  while (remaining_size > 0) {
    if (timeout_nsec >= 0) {
      fd_set rfds;
      FD_ZERO(&rfds);
      FD_SET(socket, &rfds);
      struct timeval tv;
      tv.tv_sec = timeout_nsec / 1000000000;
      tv.tv_usec = (timeout_nsec % 1000000000) / 1000;
      int retval = select(socket + 1, &rfds, NULL, NULL, &tv);
      if (retval == -1) {
        PRINT("select() failed. error=%s\n", strerror(errno));
        ret = -1;
        break;
      } else if (retval == 0) {
        ret = -2;  // timeout
        break;
      }
    }
    ssize_t received_size = recv(
        socket, &ptr[total_received_size], remaining_size, 0);
    if (received_size < 0) {
      PRINT("recv() failed. error=%s\n", strerror(errno));
      return -1;
    } else if (received_size == 0) {
      PRINT("disconnect\n");
      return -1;
    }
    total_received_size += static_cast<size_t>(received_size);
    remaining_size -= static_cast<size_t>(received_size);
  }
  *recv_size = static_cast<uint32_t>(total_received_size);
  return ret;
}

static int32_t RecvHeader(int32_t socket, Header* header) {
  int64_t timeout_nsec = 5000000000;  // 5sec
  uint8_t buffer[sizeof(Header)];
  uint32_t read_size = sizeof(buffer);
  while (true) {
    uint32_t offset = static_cast<uint32_t>(sizeof(buffer) - read_size);
    uint64_t start_time = 0;
    senscord::osal::OSGetTime(&start_time);
    int32_t ret = RecvWithTimeout(
        socket, &buffer[offset], &read_size, timeout_nsec);
    if (ret != 0) {
      return ret;
    }
    if (timeout_nsec >= 0) {
      uint64_t end_time = 0;
      senscord::osal::OSGetTime(&end_time);
      int64_t elapsed = (end_time < start_time) ?
          0 : static_cast<int64_t>(end_time - start_time);
      timeout_nsec = (timeout_nsec < elapsed) ? 0 : (timeout_nsec - elapsed);
#if 0
      PRINT("timeout=%" PRId64 " (elapsed=%" PRId64 ")\n",
            timeout_nsec, elapsed);
#endif
    }
    // find signature
    const uint32_t count = sizeof(buffer) - sizeof(kHeaderSignature);
    for (offset = 0; offset < count; ++offset) {
      if ((buffer[offset + 0] == kHeaderSignature[0]) &&
          (buffer[offset + 1] == kHeaderSignature[1]) &&
          (buffer[offset + 2] == kHeaderSignature[2]) &&
          (buffer[offset + 3] == kHeaderSignature[3])) {
        break;
      }
    }
    if (offset == 0) {
      break;
    }
    senscord::osal::OSMemmove(
        &buffer[0], sizeof(buffer),
        &buffer[offset], sizeof(buffer) - offset);
    read_size = offset;
  }
  *header = *reinterpret_cast<Header*>(buffer);
  return 0;
}

struct ApplicationInfo {
  uint32_t stack_size;
  uint32_t heap_size;
  std::vector<uint8_t> module_data;
  std::vector<uint8_t> args_data;
  std::vector<char*> args;
  wasm_module_t module;
  wasm_module_inst_t module_inst;

  pthread_t thread;
  bool running;  // need mutex ?
};

/**
 * @brief Application thread.
 */
static void* ApplicationThread(void* args) {
  uintptr_t result = 0;
  ApplicationInfo* info = reinterpret_cast<ApplicationInfo*>(args);
  wasm_runtime_init_thread_env();

  LOG_D("wasm_application_execute_main <start>\n");
  wasm_application_execute_main(
      info->module_inst,
      static_cast<int32_t>(info->args.size()), &info->args[0]);
  LOG_D("wasm_application_execute_main <finish>\n");
  const char* exception = wasm_runtime_get_exception(info->module_inst);
  if (exception != NULL) {
    PRINT("call wasm function main failed. error: %s\n", exception);
    result = 1;
  } else {
    result = 0;
  }

#if WASM_ENABLE_LIBC_WASI != 0
  if (result == 0) {
    result = wasm_runtime_get_wasi_exit_code(info->module_inst);
  }
#endif

  wasm_runtime_destroy_thread_env();

  info->running = false;

  return reinterpret_cast<void*>(result);
}

static void ExecApplication(ApplicationInfo* info) {
  char error_buf[128];

  PRINT("stack size     : %" PRIu32 "\n", info->stack_size);
  PRINT("heap size      : %" PRIu32 "\n", info->heap_size);
  PRINT("wasm data size : %" PRIuS "\n", info->module_data.size());
  PRINT("args           : ");
  for (size_t offset = 0; offset < info->args_data.size();) {
    char* str = reinterpret_cast<char*>(&info->args_data[offset]);
    PRINT("'%s', ", str);
    info->args.push_back(str);
    offset += strlen(str) + 1;
  }
  PRINT("\n");

  do {
    LOG_D("wasm_runtime_load\n");
    info->module = wasm_runtime_load(
        &info->module_data[0], static_cast<uint32_t>(info->module_data.size()),
        error_buf, sizeof(error_buf));
    if (info->module == NULL) {
      PRINT("Load wasm module failed. error: %s\n", error_buf);
      break;
    }

#if WASM_ENABLE_LIBC_WASI != 0
    LOG_D("wasm_runtime_set_wasi_args\n");
    wasm_runtime_set_wasi_args(
        info->module,
        NULL, 0,  // dir
        NULL, 0,  // map_dir
        NULL, 0,  // env
        &info->args[0], static_cast<int32_t>(info->args.size()));
#endif

    LOG_D("wasm_runtime_instantiate\n");
    info->module_inst = wasm_runtime_instantiate(
        info->module,
        info->stack_size, info->heap_size,
        error_buf, sizeof(error_buf));
    if (info->module_inst == NULL) {
      PRINT("Instantiate wasm module failed. error: %s\n", error_buf);
      break;
    }

    info->running = true;
    LOG_D("pthread_create\n");
    if (pthread_create(
        &info->thread,
        NULL,
        ApplicationThread,
        info) != 0) {
      PRINT("Create thread failed.\n");
      info->running = false;
      break;
    }
  } while (false);
}

static int32_t CreateListenSocket(const char* address, uint16_t port) {
  // socket
  int32_t listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    PRINT("socket() failed. error=%s\n", strerror(errno));
    return -1;
  }

  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_aton(address, &addr.sin_addr) == 0) {
    PRINT("inet_aton() failed. address=%s\n", address);
    close(listen_fd);
    return -1;
  }

  // bind
  if (bind(listen_fd, reinterpret_cast<const sockaddr*>(&addr),
            sizeof(addr)) != 0) {
    PRINT("bind() failed. error=%s\n", strerror(errno));
    close(listen_fd);
    return -1;
  }

  // listen
  if (listen(listen_fd, 5) != 0) {
    PRINT("listen() failed. error=%s\n", strerror(errno));
    close(listen_fd);
    return -1;
  }

  return listen_fd;
}

static void ReleaseApplication(
    std::vector<ApplicationInfo*>* app_list, bool force) {
  for (std::vector<ApplicationInfo*>::iterator
      itr = app_list->begin(); itr != app_list->end();) {
    ApplicationInfo* info = *itr;
    if (force || !info->running) {
      if (info->thread != 0) {
        LOG_D("pthread_join\n");
        pthread_join(info->thread, NULL);
      }
      if (info->module_inst != NULL) {
        LOG_D("wasm_runtime_deinstantiate\n");
        wasm_runtime_deinstantiate(info->module_inst);
      }
      if (info->module != NULL) {
        LOG_D("wasm_runtime_unload\n");
        wasm_runtime_unload(info->module);
      }
      LOG_D("delete app=%p\n", info);
      delete info;
      itr = app_list->erase(itr);
    } else {
      ++itr;
    }
  }
}

static void ForceExitApplication(
    std::vector<ApplicationInfo*>* app_list) {
  for (std::vector<ApplicationInfo*>::iterator
      itr = app_list->begin(); itr != app_list->end(); ++itr) {
    ApplicationInfo* info = *itr;
    if (info->running) {
      wasm_runtime_terminate(info->module_inst);
    }
  }
}

/**
 * @brief Run the server.
 * @param[in] address  Server address.
 * @param[in] port  Server port.
 * @return 0 indicates success, while a negative number indicates failure.
 */
extern "C" int32_t senscord_iwasm_run_server(
    const char* address, uint16_t port) {
  int32_t listen_fd = CreateListenSocket(address, port);
  if (listen_fd < 0) {
    return -1;
  }

  int32_t result = -1;
  std::vector<ApplicationInfo*> app_list;
  bool wait_print = true;

  while (true) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(listen_fd, &rfds);
    FD_SET(STDIN_FILENO, &rfds);

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (wait_print) {
      PRINT("Waiting...  (Enter 'q' to quit) \n");
      wait_print = false;
    }
    int retval = select(listen_fd + 1, &rfds, NULL, NULL, &tv);
    if (retval == -1) {
      PRINT("select() failed. error=%s\n", strerror(errno));
      break;
    } else if (retval == 0) {
      // timeout
      size_t count = app_list.size();
      ReleaseApplication(&app_list, false);
      if (count != app_list.size()) {
        wait_print = true;
      }
      continue;
    }
    if (FD_ISSET(STDIN_FILENO, &rfds)) {
      char buffer[10] = {};
      if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        continue;
      }
      size_t len = strlen(buffer);
      if ((len > 0) && (buffer[len - 1] == '\n')) {
        buffer[len - 1] = '\0';
      } else {
        while (getchar() != '\n') {}
      }
      LOG_D("Input: '%s'\n", buffer);
      if (strcmp(buffer, "q") == 0) {
        ForceExitApplication(&app_list);
        result = 0;
        break;
      }
    }
    if (FD_ISSET(listen_fd, &rfds)) {
      int32_t accept_fd = accept(listen_fd, NULL, NULL);
      if (accept_fd < 0) {
        PRINT("accept() failed. error=%s\n", strerror(errno));
        continue;
      }
      Header header = {};
      if (RecvHeader(accept_fd, &header) == 0) {
        uint32_t payload_size = ntohl(header.payload_size);
        std::vector<uint8_t> payload(payload_size);
        int32_t ret = RecvWithTimeout(
            accept_fd, &payload[0], &payload_size, 5000000000);
        if (ret == 0 && payload_size == payload.size()) {
          PRINT("payload_size=%" PRIu32 "\n", payload_size);
          ApplicationInfo* info = new ApplicationInfo();
          LOG_D("create app=%p\n", info);
          size_t offset = 0;
          uint8_t type[4];
          memcpy(type, &payload[offset], sizeof(type));
          if (memcmp(type, kCommandTypeExec, sizeof(type)) == 0) {
            ExecHeader exec_header;
            memcpy(&exec_header, &payload[offset], sizeof(ExecHeader));
            info->stack_size = ntohl(exec_header.stack_size);
            info->heap_size = ntohl(exec_header.heap_size);
            uint32_t module_data_size = ntohl(exec_header.module_data_size);
            uint32_t args_size = ntohl(exec_header.args_size);
            offset += sizeof(ExecHeader);
            info->module_data.assign(
                &payload[offset], &payload[offset + module_data_size]);
            offset += module_data_size;
            info->args_data.assign(
                &payload[offset], &payload[offset + args_size]);
            offset += args_size;
            ExecApplication(info);
            app_list.push_back(info);
          } else {
            PRINT("Invalid command type.\n");
            delete info;
          }
        }
      }
      close(accept_fd);
      wait_print = true;
    }
  }

  ReleaseApplication(&app_list, true);

  close(listen_fd);

  return result;
}
