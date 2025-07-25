/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_ALLOCATOR_SHARED_ALLOCATION_FIRSTFIT_H_
#define LIB_CORE_ALLOCATOR_SHARED_ALLOCATION_FIRSTFIT_H_

#include <list>
#include <map>
#include <utility>
#include "allocator/shared_allocation_method.h"

namespace senscord {

/**
 * @brief First fit allocation.
 */
class FirstFitAllocation : public AllocationMethod {
 public:
  /**
   * @brief Constructor.
   */
  FirstFitAllocation() : total_size_() {}

  /**
   * @brief Destructor.
   */
  virtual ~FirstFitAllocation() {}

  /**
   * @brief Initialization.
   * @param[in] (total_size) Total size of memory.
   * @return Status object.
   */
  virtual Status Init(int32_t total_size) {
    if (total_size <= 0) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument, "total size is too small.");
    }
    total_size_ = total_size;
    used_list_.clear();
    free_list_.clear();
    OffsetParam offset = { 0, total_size };
    free_list_.push_back(offset);
    return Status::OK();
  }

  /**
   * @brief Allocate memory block.
   * @param[in] (size) Size to allocate.
   * @param[out] (offset) Offset information.
   * @return Status object.
   */
  virtual Status Allocate(int32_t size, OffsetParam* offset) {
    if (offset == NULL) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument, "offset == null");
    }
    // Allocation from free space.
    std::list<OffsetParam>::iterator itr = free_list_.begin();
    std::list<OffsetParam>::iterator end = free_list_.end();
    for (; itr != end; ++itr) {
      if (size <= itr->size) {
        offset->offset = itr->offset;
        offset->size = size;
        itr->offset += size;
        itr->size -= size;
        if (itr->size == 0) {
          free_list_.erase(itr);
        }
        used_list_.insert(std::make_pair(offset->offset, *offset));
        return Status::OK();
      }
    }
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseResourceExhausted, "Cannot allocate from free space");
  }

  /**
   * @brief Free memory block.
   * @param[in] (offset) Offset information to free.
   * @return Status object.
   */
  virtual Status Free(const OffsetParam& offset) {
    std::map<int32_t, const OffsetParam>::iterator pos =
        used_list_.find(offset.offset);
    if (pos == used_list_.end()) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseNotFound, "specified offset is not in the used_list");
    }
    // Search the previous and next free space.
    std::list<OffsetParam>::iterator prev, next, end;
    prev = next = end = free_list_.end();
    for (std::list<OffsetParam>::iterator itr = free_list_.begin();
        itr != end; ++itr) {
      if (itr->offset < pos->first) {
        prev = itr;
      } else if (itr->offset > pos->first) {
        next = itr;
        break;
      }
    }
    if (prev != end && (prev->offset + prev->size) == pos->first) {
      // Adjacent to the previous element.
      prev->size += pos->second.size;
      if (next != end && (prev->offset + prev->size) == next->offset) {
        // Adjacent to the next element.
        prev->size += next->size;
        free_list_.erase(next);
      }
    } else if (next != end &&
        (pos->second.offset + pos->second.size) == next->offset) {
      // Adjacent to the next element.
      next->offset = pos->second.offset;
      next->size += pos->second.size;
    } else {
      free_list_.insert(next, pos->second);
    }
    used_list_.erase(pos);
    return Status::OK();
  }

 private:
  std::map<int32_t, const OffsetParam> used_list_;
  std::list<OffsetParam> free_list_;
  int32_t total_size_;
};

}  // namespace senscord

#endif  // LIB_CORE_ALLOCATOR_SHARED_ALLOCATION_FIRSTFIT_H_
