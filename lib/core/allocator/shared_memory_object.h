/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_ALLOCATOR_SHARED_MEMORY_OBJECT_H_
#define LIB_CORE_ALLOCATOR_SHARED_MEMORY_OBJECT_H_

#include <string>
#include "senscord/memory_allocator.h"

namespace senscord {

/**
 * @brief Shared memory object.
 */
class SharedMemoryObject {
 public:
  /**
   * @brief Constructor.
   */
  SharedMemoryObject() {}

  /**
   * @brief Destructor.
   */
  virtual ~SharedMemoryObject() {}

  /**
   * @brief Get the size of the unit block of memory allocation.
   * @return Size of block in bytes.
   */
  virtual int32_t GetBlockSize() const = 0;

  /**
   * @brief Get the total size of shared memory.
   * @return Total size.
   */
  virtual int32_t GetTotalSize() const = 0;

  /**
   * @brief Opens or creates a memory object.
   * @param[in] (name) Name of memory object.
   * @param[in] (total_size) Total size of memory.
   * @return Status object.
   */
  virtual Status Open(const std::string& name, int32_t total_size) = 0;

  /**
   * @brief Closes the memory object.
   * @return Status object.
   */
  virtual Status Close() = 0;

  /**
   * @brief Map to memory.
   * @param[in] (offset) Starting offset for the mapping.
   * @param[in] (size) Size to map.
   * @param[out] (address) Mapped virtual address.
   * @return Status object.
   */
  virtual Status Map(int32_t offset, int32_t size, void** address) = 0;

  /**
   * @brief Unmap to memory.
   * @param[in] (address) Mapped virtual address.
   * @return Status object.
   */
  virtual Status Unmap(void* address) = 0;
};

}  // namespace senscord

#endif  // LIB_CORE_ALLOCATOR_SHARED_MEMORY_OBJECT_H_
