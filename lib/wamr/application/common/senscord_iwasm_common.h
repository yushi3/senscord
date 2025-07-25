/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_WAMR_APPLICATION_COMMON_SENSCORD_IWASM_COMMON_H_
#define LIB_WAMR_APPLICATION_COMMON_SENSCORD_IWASM_COMMON_H_

#include <stdint.h>

// Header signature.
const uint8_t kHeaderSignature[] = { 0xDE, 0xAD, 0xC0, 0xDE };

// command type.
const uint8_t kCommandTypeExec[] = { 'e', 'x', 'e', 'c' };

struct Header {
  uint32_t signature;
  uint32_t payload_size;
};

struct ExecHeader {
  uint32_t type;
  uint32_t stack_size;
  uint32_t heap_size;
  uint32_t module_data_size;
  uint32_t args_size;
};

#endif  // LIB_WAMR_APPLICATION_COMMON_SENSCORD_IWASM_COMMON_H_
