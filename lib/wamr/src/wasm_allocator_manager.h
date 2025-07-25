/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_WAMR_SRC_WASM_ALLOCATOR_MANAGER_H_
#define LIB_WAMR_SRC_WASM_ALLOCATOR_MANAGER_H_

#include <stdint.h>

#include <string>

#include "senscord/status.h"
#include "senscord/memory_allocator.h"

#include "wasm_export.h"

namespace senscord {

/**
 * @brief The state of the WASM Allocator.
 */
enum WasmAllocatorState {
  kNotWasm,
  kOwnedWasm,
  kNotOwnedWasm,
};

/**
 * @brief Manager of the WASM Allocator.
 */
class WasmAllocatorManager {
 public:
  /**
   * @brief Create singleton instance.
   */
  static WasmAllocatorManager* CreateInstance();

  /**
   * @brief Get singleton instance.
   */
  static WasmAllocatorManager* GetInstance();

  /**
   * @brief Delete singleton instance.
   */
  static void DeleteInstance();

  /**
   * @brief Create WasmMemoryAllocator.
   * @param[in] (stream_key) Stream key for search.
   * @param[in] (allocator_key) Allocator key for MemoryAllocator.
   * @param[out] (allocator) Generated MemoryAllocator.
   * @return Status object.
   */
  Status CreateAllocator(
      const std::string& stream_key,
      const std::string& allocator_key,
      MemoryAllocator** allocator);

  /**
   * @brief Delete WasmMemoryAllocator.
   * @param[in] (allocator) MemoryAllocator to delete.
   * @return Status object.
   */
  Status DeleteAllocator(MemoryAllocator* allocator);

  /**
   * @brief Register Wasm environment to Allocator linked to stream key.
   * @param[in] (stream_key) Stream key for search.
   * @param[in] (module_inst) Wasm moudule instance to register.
   * @return Status object.
   */
  Status RegisterWasm(
      const std::string& stream_key,
      wasm_module_inst_t module_inst);

  /**
   * @brief Unregister Wasm environment from Allocator.
   * @param[in] (stream_key) Stream key for search.
   * @param[in] (module_inst) Wasm moudule instance to unregister.
   * @return Status object.
   */
  Status UnregisterWasm(
      const std::string& stream_key,
      wasm_module_inst_t module_inst);

  /**
   * @brief Get the state of the WASM Allocator.
   * @param[in] (stream_key) Stream key for search.
   * @param[in] (module_inst) Wasm moudule instance.
   * @return the state of the WASM Allocator.
   */
  WasmAllocatorState GetAllocatorState(
      const std::string& stream_key,
      wasm_module_inst_t module_inst) const;

 private:
  WasmAllocatorManager();
  ~WasmAllocatorManager();

 private:
  struct Impl;
  Impl* pimpl_;
};

}  // namespace senscord

#endif  // LIB_WAMR_SRC_WASM_ALLOCATOR_MANAGER_H_
