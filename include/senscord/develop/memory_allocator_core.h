/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_MEMORY_ALLOCATOR_CORE_H_
#define SENSCORD_DEVELOP_MEMORY_ALLOCATOR_CORE_H_

#include <stdint.h>
#include <string>

#include "senscord/config.h"
#include "senscord/memory_allocator.h"
#include "senscord/develop/memory_allocator_types.h"

namespace senscord {

/**
 * @brief Memory allocator core.
 */
class MemoryAllocatorCore : public MemoryAllocator {
 public:
  /**
   * @brief Initialization.
   * @param[in] (config) Allocator config.
   * @return Status object.
   */
  virtual Status Init(const AllocatorConfig& config) {
    key_ = config.key;
    type_ = config.type;
    cacheable_ = config.cacheable;
    return Status::OK();
  }

  /**
   * @brief Exiting.
   * @return Status object.
   */
  virtual Status Exit() {
    return Status::OK();
  }

  /**
   * @brief Invalidate of cache.
   * @param[in] (address) Start virtual address to invalidate.
   * @param[in] (size) Size to invalidate.
   * @return Status object.
   */
  virtual Status InvalidateCache(uintptr_t address, size_t size) {
    /* default not implement */
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not supported");
  }

  /**
   * @brief Clean of cache.
   * @param[in] (address) Start virtual address to clean.
   * @param[in] (size) Size to clean.
   * @return Status object.
   */
  virtual Status CleanCache(uintptr_t address, size_t size) {
    /* default not implement */
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not supported");
  }

  /**
   * @brief Get allocator key.
   * @return Allocator key.
   */
  const std::string& GetKey(void) const {
    return key_;
  }

  /**
   * @brief Get allocator type.
   * @return Allocator type.
   */
  const std::string& GetType(void) const {
    return type_;
  }

  /**
   * @brief Is cacheable allocator.
   * @return True is cacheable, false is not cacheable.
   */
  virtual bool IsCacheable(void) const {
    return cacheable_;
  }

  /**
   * @brief Destructor.
   */
  virtual ~MemoryAllocatorCore() {}

 protected:
  /**
   * @brief Constructor.
   */
  MemoryAllocatorCore() : cacheable_() {}

 private:
  /* allocator type */
  std::string type_;

  /* allocator key */
  std::string key_;

  /* cacheable */
  bool cacheable_;
};

}  // namespace senscord
#endif  // SENSCORD_DEVELOP_MEMORY_ALLOCATOR_CORE_H_
