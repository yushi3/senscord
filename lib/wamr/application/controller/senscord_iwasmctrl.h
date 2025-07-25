/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_WAMR_APPLICATION_CONTROLLER_SENSCORD_IWASMCTRL_H_
#define LIB_WAMR_APPLICATION_CONTROLLER_SENSCORD_IWASMCTRL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

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
int32_t senscord_iwasm_send_exec_parameter(
    const char* address, uint16_t port,
    const char* wasm_path,
    uint32_t stack_size, uint32_t heap_size,
    int32_t argc, char** argv);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // LIB_WAMR_APPLICATION_CONTROLLER_SENSCORD_IWASMCTRL_H_
