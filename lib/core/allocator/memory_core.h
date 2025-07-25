/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_ALLOCATOR_MEMORY_CORE_H_
#define LIB_CORE_ALLOCATOR_MEMORY_CORE_H_

#include "senscord/memory.h"
#include "senscord/memory_allocator.h"

namespace senscord {

/**
 * @brief Memory core.
 */
class MemoryCore : public Memory {
 public:
  /**
   * @brief Constructor.
   * @param[in] (address) Memory block address.
   * @param[in] (size) Memory block size.
   * @param[in] (allocator) Depend allocator.
   */
  explicit MemoryCore(uintptr_t address, size_t size,
                      const MemoryAllocator& allocator)
      : address_(address), size_(size) {
    allocator_ = const_cast<MemoryAllocator*>(&allocator);
  }

  /**
   * @brief Destructor.
   */
  virtual ~MemoryCore() {}

  /**
   * @brief Get memory block address.
   * @return Memory block address.
   */
  uintptr_t GetAddress() const {
    return address_;
  }

  /**
   * @brief Get memory block size.
   * @return Memory block size.
   */
  size_t GetSize() const {
    return size_;
  }

  /**
   * @brief Invalidate of memory block.
   * @return Status object.
   */
  Status Invalidate() {
    Status status = allocator_->InvalidateCache(address_, size_);
    return SENSCORD_STATUS_TRACE(status);
  }

  /**
   * @brief Get depend allocator instance.
   * @return Allocator instance.
   */
  MemoryAllocator* GetAllocator() const {
    return allocator_;
  }

 private:
  uintptr_t address_;
  size_t size_;
  MemoryAllocator* allocator_;
};

}  // namespace senscord
#endif  // LIB_CORE_ALLOCATOR_MEMORY_CORE_H_
