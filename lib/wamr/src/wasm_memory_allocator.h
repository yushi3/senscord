/*
 * SPDX-FileCopyrightText: 2023-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_WAMR_SRC_WASM_MEMORY_ALLOCATOR_H_
#define LIB_WAMR_SRC_WASM_MEMORY_ALLOCATOR_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "senscord/memory_allocator.h"

#include "wasm_export.h"

namespace senscord {

/**
 * @brief WASM Memory allocator.
 */
class WasmMemoryAllocator : public MemoryAllocator {
 public:
  /**
   * @brief Constructor.
   * @param[in] (allocator_key) Allocator key.
   * @param[in] (stream_key) Stream key.
   */
  explicit WasmMemoryAllocator(
      const std::string& allocator_key, const std::string& stream_key);

  /**
   * @brief Destructor.
   */
  ~WasmMemoryAllocator();

  /**
   * @brief Register the Wasm module.
   * @param[in] (exec_env) Wasm execution environment.
   * @return Status object.
   */
  Status RegisterWasm(wasm_exec_env_t exec_env);

  /**
   * @brief Unregister the Wasm module.
   * @return Status object.
   */
  Status UnregisterWasm();

  /**
   * @brief Allocate memory block.
   * @param[in] (size) Size to allocate.
   * @param[out] (memory) Allocated Memory.
   * @return Status object.
   */
  virtual Status Allocate(size_t size, Memory** memory);

  /**
   * @brief Free memory block.
   * @param[in] (memory) Memory to free.
   * @return Status object.
   */
  virtual Status Free(Memory* memory);

  /**
   * @brief Map memory block.
   * @param[in] (memory) Memory to map.
   * @return Status object.
   */
  virtual Status Map(Memory* memory);

  /**
   * @brief Unmap memory block.
   * @param[in] (memory) Memory to unmap.
   * @return Status object.
   */
  virtual Status Unmap(Memory* memory);

#ifdef SENSCORD_SERVER
  /**
   * @brief Serialize the raw data memory area.
   * @param[in] (rawdata_memory) Memory information for raw data.
   * @param[out] (serialized) Serialized memory information.
   * @return Status object.
   */
  virtual Status ServerSerialize(
      const RawDataMemory& rawdata_memory,
      std::vector<uint8_t>* serialized) const;

  /**
   * @brief Initialize the mapping area.
   * @return Status object.
   */
  virtual Status ClientInitMapping();

  /**
   * @brief Deinitialize the mapping area.
   * @return Status object.
   */
  virtual Status ClientExitMapping();

  /**
   * @brief Mapping memory with serialized memory information.
   * @param[in] (serialized) Serialized memory information.
   * @param[out] (rawdata_memory) Memory information for raw data.
   * @return Status object.
   */
  virtual Status ClientMapping(
      const std::vector<uint8_t>& serialized,
      RawDataMemory* rawdata_memory);

  /**
   * @brief Release the mapped area.
   * @param[in] (rawdata_memory) Memory information for raw data.
   * @return Status object.
   */
  virtual Status ClientUnmapping(const RawDataMemory& rawdata_memory);
#endif  // SENSCORD_SERVER

  /**
   * @brief Invalidate of cache.
   * @param[in] (address) Start virtual address to invalidate.
   * @param[in] (size) Size to invalidate.
   * @return Status object.
   */
  virtual Status InvalidateCache(uintptr_t address, size_t size);

  /**
   * @brief Clean of cache.
   * @param[in] (address) Start virtual address to clean.
   * @param[in] (size) Size to clean.
   * @return Status object.
   */
  virtual Status CleanCache(uintptr_t address, size_t size);

  /**
   * @brief Get allocator key.
   * @return Allocator key.
   */
  virtual const std::string& GetKey() const;

  /**
   * @brief Get allocator type.
   * @return Allocator type.
   */
  virtual const std::string& GetType() const;

  /**
   * @brief Whether the memory is shared.
   * @return Always returns false.
   */
  virtual bool IsMemoryShared() const;

  /**
   * @brief Is cacheable allocator.
   * @return Always returns false.
   */
  virtual bool IsCacheable() const;

  /**
   * @brief Get stream key.
   * @return Stream key.
   */
  const std::string& GetStreamKey() const;

 private:
  struct Impl;
  Impl* pimpl_;
};

}  // namespace senscord

#endif  // LIB_WAMR_SRC_WASM_MEMORY_ALLOCATOR_H_
