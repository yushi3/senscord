/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_SERVER_SERVER_LOG_H_
#define LIB_SERVER_SERVER_LOG_H_

#include "senscord/logger.h"

namespace senscord {
namespace server {

// if stop server log, disable this define.
#define USE_SERVER_LOGS   1

// server log macros.
#ifdef USE_SERVER_LOGS
#define SENSCORD_SERVER_LOG_ERROR(...)    SENSCORD_LOG_ERROR(__VA_ARGS__)
#define SENSCORD_SERVER_LOG_WARNING(...)    SENSCORD_LOG_WARNING(__VA_ARGS__)
#define SENSCORD_SERVER_LOG_INFO(...)    SENSCORD_LOG_INFO(__VA_ARGS__)
#define SENSCORD_SERVER_LOG_DEBUG(...)    SENSCORD_LOG_DEBUG(__VA_ARGS__)
#else
#define SENSCORD_SERVER_LOG_ERROR(...)
#define SENSCORD_SERVER_LOG_WARNING(...)
#define SENSCORD_SERVER_LOG_INFO(...)
#define SENSCORD_SERVER_LOG_DEBUG(...)
#endif

}   // namespace server
}   // namespace senscord

#endif  // LIB_SERVER_SERVER_LOG_H_
