/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_MEMORY_ALLOCATOR_H_
#define SENSCORD_MEMORY_ALLOCATOR_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/memory.h"

namespace senscord {

/**
 * @brief Memory allocator interface.
 */
class MemoryAllocator : private util::Noncopyable {
 public:
  /**
   * @brief Allocate memory block.
   * @param[in]  (size) Size to allocate.
   * @param[out] (memory) Allocated Memory.
   * @return Status object.
   */
  virtual Status Allocate(size_t size, Memory** memory) = 0;

  /**
   * @brief Free memory block.
   * @param[in] (memory) Memory to free.
   * @return Status object.
   */
  virtual Status Free(Memory* memory) = 0;

  /**
   * @brief Map memory block.
   * @param[in] (memory) Memory to map.
   * @return Status object.
   */
  virtual Status Map(Memory* memory) {
    return Status::OK();
  }

  /**
   * @brief Unmap memory block.
   * @param[in] (memory) Memory to unmap.
   * @return Status object.
   */
  virtual Status Unmap(Memory* memory) {
    return Status::OK();
  }

#ifdef SENSCORD_SERVER
  /**
   * @brief Serialize the raw data memory area.
   * @param[in] (rawdata_memory) Memory information for raw data.
   * @param[out] (serialized) Serialized memory information.
   * @return Status object.
   */
  virtual Status ServerSerialize(
      const RawDataMemory& rawdata_memory,
      std::vector<uint8_t>* serialized) const {
    return Status::OK();
  }
  /** @deprecated Use `ServerSerialize` function. */
  virtual Status Serialize(
      const RawDataMemory& rawdata_memory,
      std::vector<uint8_t>* serialized) const {
    return ServerSerialize(rawdata_memory, serialized);
  }

  /**
   * @brief Initialize the mapping area.
   * @return Status object.
   */
  virtual Status ClientInitMapping() {
    return Status::OK();
  }
  /** @deprecated Use `ClientInitMapping` function. */
  virtual Status InitMapping() {
    return ClientInitMapping();
  }

  /**
   * @brief Deinitialize the mapping area.
   * @return Status object.
   */
  virtual Status ClientExitMapping() {
    return Status::OK();
  }
  /** @deprecated Use `ClientExitMapping` function. */
  virtual Status ExitMapping() {
    return ClientExitMapping();
  }

  /**
   * @brief Mapping memory with serialized memory information.
   * @param[in] (serialized) Serialized memory information.
   * @param[out] (rawdata_memory) Memory information for raw data.
   * @return Status object.
   */
  virtual Status ClientMapping(
      const std::vector<uint8_t>& serialized,
      RawDataMemory* rawdata_memory) {
    return Status::OK();
  }
  /** @deprecated Use `ClientMapping` function. */
  virtual Status Mapping(
      const std::vector<uint8_t>& serialized,
      RawDataMemory* rawdata_memory) {
    return ClientMapping(serialized, rawdata_memory);
  }

  /**
   * @brief Release the mapped area.
   * @param[in] (rawdata_memory) Memory information for raw data.
   * @return Status object.
   */
  virtual Status ClientUnmapping(const RawDataMemory& rawdata_memory) {
    return Status::OK();
  }
  /** @deprecated Use `ClientUnmapping` function. */
  virtual Status Unmapping(const RawDataMemory& rawdata_memory) {
    return ClientUnmapping(rawdata_memory);
  }
#endif  // SENSCORD_SERVER

  /**
   * @brief Invalidate of cache.
   * @param[in] (address) Start virtual address to invalidate.
   * @param[in] (size) Size to invalidate.
   * @return Status object.
   */
  virtual Status InvalidateCache(uintptr_t address, size_t size) = 0;

  /**
   * @brief Clean of cache.
   * @param[in] (address) Start virtual address to clean.
   * @param[in] (size) Size to clean.
   * @return Status object.
   */
  virtual Status CleanCache(uintptr_t address, size_t size) = 0;

  /**
   * @brief Get allocator key.
   * @return Allocator key.
   */
  virtual const std::string& GetKey() const = 0;

  /**
   * @brief Get allocator type.
   * @return Allocator type.
   */
  virtual const std::string& GetType() const = 0;

  /**
   * @brief Whether the memory is shared.
   * @return True means sharing between other process, false means local.
   */
  virtual bool IsMemoryShared() const = 0;

  /**
   * @brief Is cacheable allocator.
   * @return True is cacheable, false is not cacheable.
   */
  virtual bool IsCacheable() const = 0;

  /**
   * @brief Destructor.
   */
  virtual ~MemoryAllocator() {}
};

}  // namespace senscord
#endif  // SENSCORD_MEMORY_ALLOCATOR_H_
