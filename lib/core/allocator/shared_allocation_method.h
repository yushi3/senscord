/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_ALLOCATOR_SHARED_ALLOCATION_METHOD_H_
#define LIB_CORE_ALLOCATOR_SHARED_ALLOCATION_METHOD_H_

#include "senscord/memory_allocator.h"

namespace senscord {

/**
 * @brief Offset information.
 */
struct OffsetParam {
  int32_t offset;
  int32_t size;
};

/**
 * @brief Allocation method interface.
 */
class AllocationMethod {
 public:
  /**
   * @brief Constructor.
   */
  AllocationMethod() {}

  /**
   * @brief Destructor.
   */
  virtual ~AllocationMethod() {}

  /**
   * @brief Initialization.
   * @param[in] (total_size) Total size of memory.
   * @return Status object.
   */
  virtual Status Init(int32_t total_size) = 0;

  /**
   * @brief Allocate memory block.
   * @param[in] (size) Size to allocate.
   * @param[out] (offset) Offset information.
   * @return Status object.
   */
  virtual Status Allocate(int32_t size, OffsetParam* offset) = 0;

  /**
   * @brief Free memory block.
   * @param[in] (memory) Offset information to free.
   * @return Status object.
   */
  virtual Status Free(const OffsetParam& offset) = 0;
};

}  // namespace senscord

#endif  // LIB_CORE_ALLOCATOR_SHARED_ALLOCATION_METHOD_H_
