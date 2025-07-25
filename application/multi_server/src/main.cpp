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
#include "senscord/environment.h"
#include "multi_server.h"

// TODO: private header access
#include "util/senscord_utils.h"
#include "core/internal_types.h"

// the flag of signaled.
static volatile int end_flag = 0;

static const char kDefaultConfigFile[] = "senscord_server.xml";

/**
 * @brief The handler for signals.
 * @param[in] (arg) The signal number.
 */
static void SignalHandle(int arg) {
  senscord::osal::OSPrintf("[SensCordServer] catch signal: %d\n", arg);
  ++end_flag;
  return;
}

/**
 * @brief Print the usage.
 */
static void PrintUsage(const char* process_name) {
  senscord::osal::OSPrintf("[SensCordServer] Usage: %s [-f config_path]\n",
      process_name);
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
    senscord::osal::OSPrintf("[SensCordServer] config_path is null\n");
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
        senscord::osal::OSPrintf(
            "[SensCordServer] [-f config_path] is empty\n");
        error = true;
        continue;
      }
      *config_path = next;
      ++i;
    } else {
      // other input
      senscord::osal::OSPrintf("[SensCordServer] invalid argument: %s\n",
          arg.c_str());
      error = true;
    }
  }

  if (error) {
    PrintUsage(argv[0]);
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

  if (config_path.empty()) {
    // Get paths from environment variable.
    std::vector<std::string> env_paths;
    senscord::Status status = senscord::util::GetEnvironmentPaths(
        senscord::kSensCordFilePathEnvStr, &env_paths);
    if (!status.ok() || env_paths.empty()) {
      senscord::osal::OSPrintf(
          "[SensCordServer] failed to get environment (%s)\n",
          senscord::kSensCordFilePathEnvStr);
      PrintUsage(argv[0]);
      return -1;
    }
    senscord::Environment::SetSensCordFilePath(env_paths);

    // search the default config file from SENSCORD_FILE_PATH
    if (!senscord::util::SearchFileFromEnv(kDefaultConfigFile, &config_path)) {
      senscord::osal::OSPrintf(
          "[SensCordServer] default config file not found. (%s)\n",
          kDefaultConfigFile);
      for (std::vector<std::string>::const_iterator itr = env_paths.begin(),
          end = env_paths.end(); itr != end; ++itr) {
        senscord::osal::OSPrintf(" - %s\n", itr->c_str());
      }
      PrintUsage(argv[0]);
      return -1;
    }
  }

  {
    // register signal handler
    if (signal(SIGINT, SignalHandle) == SIG_ERR) {
      senscord::osal::OSPrintf(
          "[SensCordServer] failure to register the signal handler."
          " (SIGINT)\n");
      return -1;
    }
    if (signal(SIGTERM, SignalHandle) == SIG_ERR) {
      senscord::osal::OSPrintf(
          "[SensCordServer] failure to register the signal handler."
          " (SIGTERM)\n");
      return -1;
    }
  }

  senscord::Status status;

  // create server
  senscord::server::MultiServer server;

  // open server
  senscord::osal::OSPrintf("[SensCordServer] start %s (%s)\n",
      argv[0], config_path.c_str());
  status = server.Open(config_path);
  if (!status.ok()) {
    senscord::osal::OSPrintf("[SensCordServer] error: %s\n",
        status.ToString().c_str());
    return -1;
  }

  // wait stop signal
  while (!end_flag) {
    senscord::osal::OSSleep(1000000000);
  }

  server.Close();
  senscord::osal::OSPrintf("[SensCordServer] end %s\n", argv[0]);
  return 0;
}
