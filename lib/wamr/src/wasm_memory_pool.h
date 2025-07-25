/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_WAMR_SRC_WASM_MEMORY_POOL_H_
#define LIB_WAMR_SRC_WASM_MEMORY_POOL_H_

#include <stdint.h>

#include "senscord/status.h"
#include "senscord/c_api/senscord_c_api.h"
#include "src/senscord_wamr_context.h"
#include "src/wasm_memory.h"

#include "wasm_export.h"

namespace senscord {

struct WasmMemoryArea {
  WasmMemory* memory;
  uint32_t offset;
  uint32_t size;
};

/**
 * @brief WASM Memory Pool.
 */
class WasmMemoryPool {
 public:
  /**
   * @brief Constructor.
   */
  WasmMemoryPool();

  /**
   * @brief Destructor.
   */
  ~WasmMemoryPool();

  /**
   * @brief Sets the number of memory chunks.
   * @param[in] num  Number of memory chunks
   */
  void SetNum(uint32_t num);

  /**
   * @brief Sets the memory chunk size.
   * @param[in] size  Memory chunk size.
   */
  void SetSize(uint32_t size);

  /**
   * @brief Gets the number of memory chunks.
   * @return Number of memory chunks
   */
  uint32_t GetNum() const;

  /**
   * @brief Gets the memory chunk size.
   * @return Memory chunk size.
   */
  uint32_t GetSize() const;

  /**
   * @brief Returns true if memory pool is running.
   */
  bool IsRunning() const;

  /**
   * @brief Returns true if memory pool is closed.
   */
  bool IsClosed() const;

  /**
   * @brief Changes state when stream is opened.
   * @param[in] stream  Stream handle.
   */
  void Open(senscord_stream_t stream);

  /**
   * @brief Deletes memory pool when stream is closed.
   * @param[in] stream  Stream handle.
   * @param[in] module_inst  Wasm module instance.
   */
  void Close(
      senscord_stream_t stream,
      wasm_module_inst_t module_inst);

  /**
   * @brief Creates memory pool when stream is started.
   * @param[in] stream  Stream handle.
   * @param[in] module_inst  Wasm module instance.
   * @return Status object.
   */
  Status Start(
      senscord_stream_t stream,
      wasm_module_inst_t module_inst);

  /**
   * @brief Changes state when stream is stopped.
   * @param[in] stream  Stream handle.
   */
  void Stop(senscord_stream_t stream);

  /**
   * @brief Reserves frame memory in memory pool.
   * @param[in] module_inst  Wasm module instance.
   * @param[in] frame  Frame handle.
   * @return Status object.
   */
  Status ReserveFrameMemory(
      wasm_module_inst_t module_inst,
      senscord_frame_t frame);

  /**
   * @brief Releases frame memory.
   * @param[in] module_inst  Wasm module instance.
   * @param[in] frame  Frame handle.
   */
  void ReleaseFrameMemory(
      wasm_module_inst_t module_inst,
      senscord_frame_t frame);

  /**
   * @brief Obtains channel memory from memory pool.
   * @param[in] module_inst  Wasm module instance.
   * @param[in] frame  Frame handle.
   * @param[in] channel  Channel handle.
   * @param[out] memory_area  Memory area.
   * @return Status object.
   */
  Status GetChannelMemory(
      wasm_module_inst_t module_inst,
      senscord_frame_t frame,
      senscord_channel_t channel,
      WasmMemoryArea* memory_area);

 private:
  /**
   * @brief Creates memory pool.
   * @param[in] module_inst  Wasm module instance.
   * @return Status object.
   */
  Status CreatePool(wasm_module_inst_t module_inst);

  /**
   * @brief Deletes memory pool.
   * @param[in] module_inst  Wasm module instance.
   */
  void DeletePool(wasm_module_inst_t module_inst);

 private:
  struct Impl;
  Impl* pimpl_;
};

}  // namespace senscord

#endif  // LIB_WAMR_SRC_WASM_MEMORY_POOL_H_
