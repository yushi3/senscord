/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <signal.h>
#include <string.h>
#include <string>
#include <vector>
#include "senscord/osal.h"
#include "senscord/senscord.h"
#include "senscord/server/server.h"
#include "senscord/connection/tcp_connection.h"

// the flag of signaled.
static volatile int end_flag = 0;

/**
 * @brief The handler for signals.
 * @param[in] (arg) The signal number.
 */
static void SignalHandle(int arg) {
  senscord::osal::OSPrintf("catch signal: %d\n", arg);
  ++end_flag;
  return;
}

/**
 * @brief The assign char to string.
 * @param[in] (src) The assign char.
 * @param[out] (dest) The assigned string.
 * @return true is success.
 */
static bool AssignString(const char* src, std::string* dest) {
  if ((strlen(src) + 1) > dest->max_size()) {
    senscord::osal::OSPrintf("insufficient resource: %s\n", src);
    return false;
  }
  *dest = src;
  return true;
}

/**
 * @brief Parse the application's arguments.
 * @param[in] (argc) The number of arguments.
 * @param[in] (argv) The argumens.
 * @param[out] (config_path) The path of configuration file.
 * @return 0 is success.
 */
static int ParseArguments(int argc, char* argv[], std::string* config_path) {
  // initialize
  if (config_path == NULL) {
    senscord::osal::OSPrintf("config_path is null\n");
    return -1;
  }
  config_path->clear();

  // parse
  bool error = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg;
    if (!AssignString(argv[i], &arg)) {
      return -1;
    }
    std::string next;
    if ((i + 1) < argc) {
      if (!AssignString(argv[i + 1], &next)) {
        return -1;
      }
    }

    if (arg == "-f") {
      // config path
      if (next.empty()) {
        senscord::osal::OSPrintf("[-f config_path] is empty\n");
        error = true;
        continue;
      }
      *config_path = next;
      ++i;
    } else {
      // other input
      senscord::osal::OSPrintf("invalid argument: %s\n", arg.c_str());
      error = true;
    }
  }

  if (error) {
    senscord::osal::OSPrintf("Usage: %s [-f config_path]\n", argv[0]);
    return -1;
  }

  return 0;
}

/**
 * @brief The main function.
 */
int main(int argc, char* argv[]) {
  std::string config_path;

  // parse opt
  int ret = ParseArguments(argc, argv, &config_path);
  if (ret != 0) {
    return ret;
  }

  {
    // register signal handler
    if (signal(SIGINT, SignalHandle) == SIG_ERR) {
      senscord::osal::OSPrintf(
          "failure to register the signal handler. (SIGINT)\n");
      return -1;
    }
    if (signal(SIGTERM, SignalHandle) == SIG_ERR) {
      senscord::osal::OSPrintf(
          "failure to register the signal handler. (SIGTERM)\n");
      return -1;
    }
  }

  senscord::Status status;

  // create server
  senscord::server::Server* server = new senscord::server::Server();
  senscord::TcpConnection conn;
  senscord::TcpConnection conn2;

  // open server
  if (!config_path.empty()) {
    senscord::osal::OSPrintf("start %s (%s)\n", argv[0],
        config_path.c_str());
    status = server->Open(&conn, config_path, &conn2);
  } else {
    // use default address
    senscord::server::ServerConfig config = {};
    config.bind_config = "127.0.0.1:8080";
    config.is_enabled_client = false;

    senscord::osal::OSPrintf("start %s (%s)\n", argv[0],
        config.bind_config.c_str());
    status = server->Open(&conn, config);
  }
  if (!status.ok()) {
    senscord::osal::OSPrintf("error: %s\n", status.ToString().c_str());
    delete server;
    return -1;
  }

  // wait stop signal
  while (!end_flag) {
    senscord::osal::OSSleep(1000000000);
  }

  server->Close();
  delete server;
  senscord::osal::OSPrintf("end %s\n", argv[0]);
  return 0;
}
