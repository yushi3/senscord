/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_WAMR_WASM_ALLOCATOR_SRC_WASM_MEMORY_ALLOCATOR_ADAPTER_H_
#define LIB_WAMR_WASM_ALLOCATOR_SRC_WASM_MEMORY_ALLOCATOR_ADAPTER_H_

#include <stdint.h>

#include <vector>

#include "senscord/develop/memory_allocator_core.h"

namespace senscord {

/**
 * @brief WASM Memory allocator. (adapter)
 */
class WasmMemoryAllocatorAdapter : public MemoryAllocatorCore {
 public:
  /**
   * @brief Constructor.
   */
  WasmMemoryAllocatorAdapter();

  /**
   * @brief Destructor.
   */
  ~WasmMemoryAllocatorAdapter();

  /**
   * @brief Initialization process.
   * @param[in] (config) Allocator config.
   * @return Status object.
   */
  virtual Status Init(const AllocatorConfig& config);

  /**
   * @brief Termination process.
   * @return Status object.
   */
  virtual Status Exit();

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
   * @brief Whether the memory is shared.
   * @return Always returns false.
   */
  virtual bool IsMemoryShared() const;

 private:
  struct Impl;
  Impl* pimpl_;
};

}  // namespace senscord

#endif  // LIB_WAMR_WASM_ALLOCATOR_SRC_WASM_MEMORY_ALLOCATOR_ADAPTER_H_
