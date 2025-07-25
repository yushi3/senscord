/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_CLIENT_CLIENT_LOG_H_
#define LIB_COMPONENT_CLIENT_CLIENT_LOG_H_

#include "senscord/logger.h"

namespace client {

// if stop server log, disable this define.
#define USE_CLIENT_LOGS   1

// server log macros.
#ifdef USE_CLIENT_LOGS
#define SENSCORD_CLIENT_LOG_ERROR(...)    SENSCORD_LOG_ERROR(__VA_ARGS__)
#define SENSCORD_CLIENT_LOG_WARNING(...)  SENSCORD_LOG_WARNING(__VA_ARGS__)
#define SENSCORD_CLIENT_LOG_INFO(...)     SENSCORD_LOG_INFO(__VA_ARGS__)
#define SENSCORD_CLIENT_LOG_DEBUG(...)    SENSCORD_LOG_DEBUG(__VA_ARGS__)
#else
#define SENSCORD_CLIENT_LOG_ERROR(...)
#define SENSCORD_CLIENT_LOG_WARNING(...)
#define SENSCORD_CLIENT_LOG_INFO(...)
#define SENSCORD_CLIENT_LOG_DEBUG(...)
#endif

}   // namespace client
#endif  // LIB_COMPONENT_CLIENT_CLIENT_LOG_H_
