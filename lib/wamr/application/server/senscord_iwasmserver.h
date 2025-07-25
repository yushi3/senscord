/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_WAMR_APPLICATION_SERVER_SENSCORD_IWASMSERVER_H_
#define LIB_WAMR_APPLICATION_SERVER_SENSCORD_IWASMSERVER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * @brief Run the server.
 * @param[in] address  Server address.
 * @param[in] port  Server port.
 * @return 0 indicates success, while a negative number indicates failure.
 */
int32_t senscord_iwasm_run_server(
    const char* address, uint16_t port);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // LIB_WAMR_APPLICATION_SERVER_SENSCORD_IWASMSERVER_H_
