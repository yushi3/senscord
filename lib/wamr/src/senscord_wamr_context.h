/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_WAMR_SRC_SENSCORD_WAMR_CONTEXT_H_
#define LIB_WAMR_SRC_SENSCORD_WAMR_CONTEXT_H_

#include <stdint.h>

#include "senscord/c_api/senscord_c_api.h"
#include "src/senscord_wamr_types.h"

#include "wasm_export.h"

/**
 * @brief Operation type for context.
 */
enum senscord_context_op_t {
  SENSCORD_CONTEXT_OP_ENTER,
  SENSCORD_CONTEXT_OP_EXIT,
};

/** Context memory handle */
typedef senscord_handle_t senscord_context_memory_t;
/** Frame memory handle */
typedef senscord_handle_t senscord_frame_memory_t;
/** Wasm memory object handle */
typedef senscord_handle_t senscord_wasm_memory_t;

/**
 * @brief Memory area.
 */
struct senscord_wasm_memory_area_t {
  senscord_wasm_memory_t memory;
  uint32_t offset;
  uint32_t size;
};

/**
 * @brief Memory pool information.
 */
struct senscord_wasm_memory_pool_info_t {
  uint32_t num;
  uint32_t size;
};

/**
 * @brief Initializes the senscord context.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_init();

/**
 * @brief Exits the senscord context.
 */
void senscord_context_exit();

/**
 * @brief Sets the config handle to context.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] config      Config handle.
 * @param[in] operation   Operation type.
 */
void senscord_context_set_config(
    wasm_exec_env_t exec_env,
    senscord_config_t config,
    enum senscord_context_op_t operation);

/**
 * @brief Sets the core handle to context.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] core        Core handle.
 * @param[in] operation   Operation type.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_set_core(
    wasm_exec_env_t exec_env,
    senscord_core_t core,
    enum senscord_context_op_t operation);

/**
 * @brief Sets the stream handle to context.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] stream      Stream handle.
 * @param[in] parent_core Parent core handle.
 * @param[in] operation   Operation type.
 */
void senscord_context_set_stream(
    wasm_exec_env_t exec_env,
    senscord_stream_t stream,
    senscord_core_t parent_core,
    enum senscord_context_op_t operation);

/**
 * @brief Sets the blocking stream to context.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] stream      Stream handle.
 * @param[in] operation   Operation type.
 */
void senscord_context_set_blocking_stream(
    wasm_exec_env_t exec_env,
    senscord_stream_t stream,
    enum senscord_context_op_t operation);

/**
 * @brief Sets the stream state.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] stream      Stream handle.
 * @param[in] operation   Operation type. (enter=Running, exit=Ready)
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_set_stream_running(
    wasm_exec_env_t exec_env,
    senscord_stream_t stream,
    enum senscord_context_op_t operation);

/**
 * @brief Configures the memory pool.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] stream      Stream handle.
 * @param[in] num         Number of memory chunks.
 * @param[in] size        Memory chunk size.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_set_memory_pool(
    wasm_exec_env_t exec_env,
    senscord_stream_t stream,
    uint32_t num, uint32_t size);

/**
 * @brief Reserves frame memory in memory pool.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] frame       Frame handle.
 * @param[out] frame_memory Frame memory handle.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_reserve_frame_memory(
    wasm_exec_env_t exec_env,
    senscord_frame_t frame,
    senscord_frame_memory_t* frame_memory);

/**
 * @brief Releases frame memory.
 * @param[in] frame_memory Frame memory handle.
 */
void senscord_context_release_frame_memory(
    senscord_frame_memory_t frame_memory);

/**
 * @brief Obtains channel memory from memory pool.
 * @param[in] exec_env     WASM execution environment.
 * @param[in] frame        Frame handle.
 * @param[in] channel      Channel handle.
 * @param[out] memory_area Memory area.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_get_channel_memory(
    wasm_exec_env_t exec_env,
    senscord_frame_t frame,
    senscord_channel_t channel,
    struct senscord_wasm_memory_area_t* memory_area);

/**
 * @brief Gets memory pool information.
 * @param[in] exec_env  WASM execution environment.
 * @param[in] stream    Stream handle.
 * @param[out] info     Memory pool information.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_get_memory_pool_info(
    wasm_exec_env_t exec_env,
    senscord_stream_t stream,
    struct senscord_wasm_memory_pool_info_t* info);

/**
 * @brief Allocates memory and copy data.
 * @param[in] exec_env  WASM execution environment.
 * @param[in] data      Data to copy.
 * @param[in] size      Size of data.
 * @param[out] memory   Context memory handle.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_duplicate_memory(
    wasm_exec_env_t exec_env,
    const void* data,
    uint32_t size,
    senscord_context_memory_t* memory);

/**
 * @brief Frees memory.
 * @param[in] memory  Context memory handle.
 */
void senscord_context_free_memory(
    senscord_context_memory_t memory);

/**
 * @brief Gets the Wasm address.
 * @param[in] memory  Context memory handle.
 */
wasm_addr_t senscord_context_get_wasm_address(
    senscord_context_memory_t memory);

#endif  // LIB_WAMR_SRC_SENSCORD_WAMR_CONTEXT_H_
