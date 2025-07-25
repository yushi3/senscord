/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_ALLOCATOR_SHARED_MEMORY_H_
#define LIB_CORE_ALLOCATOR_SHARED_MEMORY_H_

#include "allocator/memory_core.h"

namespace senscord {

/**
 * @brief Shared memory.
 */
class SharedMemory : public MemoryCore {
 public:
  /**
   * @brief Constructor.
   * @param[in] (address) Memory block address.
   * @param[in] (physical_address) Physical address.
   * @param[in] (size) Memory block size.
   * @param[in] (allocator) Depend allocator.
   */
  explicit SharedMemory(
      uintptr_t address, int32_t physical_address, size_t size,
      MemoryAllocator* allocator) :
          MemoryCore(address, size, *allocator),
          physical_address_(physical_address) {}

  /**
   * @brief Gets the physical address.
   */
  int32_t GetPhysicalAddress() const { return physical_address_; }

 private:
  const int32_t physical_address_;
};

}  // namespace senscord

#endif  // LIB_CORE_ALLOCATOR_SHARED_MEMORY_H_
