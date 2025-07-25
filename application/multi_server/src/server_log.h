/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APPLICATION_MULTI_SERVER_SERVER_LOG_H_
#define APPLICATION_MULTI_SERVER_SERVER_LOG_H_

#include "senscord/logger.h"

namespace senscord {
namespace server {

// if stop server log, disable this define.
#define USE_SERVER_LOGS   1

// server log macros.
#ifdef USE_SERVER_LOGS
/** Logger tag: SensCoreServer internal */
const char kLoggerTagServer[] = "server";

#define SENSCORD_SERVER_LOG_ERROR(...)  \
  SENSCORD_LOG_ERROR_TAGGED(senscord::server::kLoggerTagServer, __VA_ARGS__)
#define SENSCORD_SERVER_LOG_WARNING(...)  \
  SENSCORD_LOG_WARNING_TAGGED(senscord::server::kLoggerTagServer, __VA_ARGS__)
#define SENSCORD_SERVER_LOG_INFO(...)  \
  SENSCORD_LOG_INFO_TAGGED(senscord::server::kLoggerTagServer, __VA_ARGS__)
#define SENSCORD_SERVER_LOG_DEBUG(...)  \
  SENSCORD_LOG_DEBUG_TAGGED(senscord::server::kLoggerTagServer, __VA_ARGS__)
#else
#define SENSCORD_SERVER_LOG_ERROR(...)
#define SENSCORD_SERVER_LOG_WARNING(...)
#define SENSCORD_SERVER_LOG_INFO(...)
#define SENSCORD_SERVER_LOG_DEBUG(...)
#endif

}   // namespace server
}   // namespace senscord

#endif  // APPLICATION_MULTI_SERVER_SERVER_LOG_H_
