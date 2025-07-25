/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CONNECTION_WS_LOG_MACRO_H_
#define LIB_CONNECTION_WS_LOG_MACRO_H_

#include "senscord/logger.h"

#define LOG_TAG "ws"
#define LOG_E(...) SENSCORD_LOG_ERROR_TAGGED(LOG_TAG, __VA_ARGS__)
#define LOG_W(...) SENSCORD_LOG_WARNING_TAGGED(LOG_TAG, __VA_ARGS__)
#define LOG_I(...) SENSCORD_LOG_INFO_TAGGED(LOG_TAG, __VA_ARGS__)
#define LOG_D(...) SENSCORD_LOG_DEBUG_TAGGED(LOG_TAG, __VA_ARGS__)

#endif  // LIB_CONNECTION_WS_LOG_MACRO_H_
