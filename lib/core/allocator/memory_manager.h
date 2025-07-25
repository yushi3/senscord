/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_ALLOCATOR_MEMORY_MANAGER_H_
#define LIB_CORE_ALLOCATOR_MEMORY_MANAGER_H_

#include <string>
#include <vector>
#include <map>

#include "senscord/noncopyable.h"
#include "senscord/develop/memory_allocator_core.h"
#include "util/mutex.h"
#include "allocator/memory_allocator_dynamic_loader.h"

namespace senscord {

/**
 * @brief MemoryAllocator manager.
 */
class MemoryManager : private util::Noncopyable {
 public:
  /**
   * @brief Get MemoryManager instance.
   * @return MemoryManager instance.
   */
  static MemoryManager* GetInstance();

  /**
   * @brief Initialize.
   * @param[in] (config_list) allocator config list.
   * @return Status object.
   */
  Status Init(const std::vector<AllocatorConfig>& config_list);

  /**
   * @brief Get MemoryAllocator instance.
   * @param[in]  (key) Search by allocator key with allocator.
   * @param[out] (allocator) Return MemoryAllocator instance
   * @return Status object.
   */
  Status GetAllocator(const std::string& key, MemoryAllocator** allocator);

 private:
  /**
   * @brief Constructor.
   */
  MemoryManager();

  /**
   * @brief Destructor.
   */
  ~MemoryManager();

  /**
   * @brief Add allocator to manage map.
   * @param[in] (config_list) allocator config list.
   * @return Status object.
   */
  Status AddMemoryAllocators(const std::vector<AllocatorConfig>& config_list);

  /**
   * @brief Add allocator.
   * @param[in] (config_list) allocator config.
   * @return Status object.
   */
  Status AddMemoryAllocator(const AllocatorConfig& config);

  /**
   * @brief Add default allocator to manage map.
   * @param[in] (config_list) allocator config list.
   * @return Status object.
   */
  Status AddDefaultMemoryAllocator(
      const std::vector<AllocatorConfig>& config_list);

  /**
   * @brief Delete all allocators.
   */
  void DeleteMemoryAllocators();

  /**
   * @brief Delete allocator.
   * @param[in] (type) allocator type.
   * @param[in] (allocator) allocator to delete.
   */
  void DeleteMemoryAllocator(
      const std::string& type,
      MemoryAllocatorCore* allocator);

  /**
   * @brief Create user defined allocator.
   * @param[in] (config) allocator config.
   * @param[out] (allocator) created allocator.
   * @return Status object.
   */
  Status CreateUserMemoryAllocator(
      const AllocatorConfig& config,
      MemoryAllocatorCore** allocator);

  /**
   * @brief Delete user defined allocator.
   * @param[in] (type) allocator type.
   * @param[in] (allocator) allocator to delete.
   */
  void DeleteUserMemoryAllocator(
      const std::string& type,
      MemoryAllocatorCore* allocator);

 private:
  struct AllocatorInstance {
    MemoryAllocatorCore* allocator;
    std::string type;  // built-in type or library name
  };
  typedef std::map<std::string, AllocatorInstance> AllocatorMap;
  AllocatorMap allocator_map_;
  MemoryAllocatorDynamicLoader loader_;
  mutable util::Mutex mutex_;
};

}   // namespace senscord
#endif  // LIB_CORE_ALLOCATOR_MEMORY_MANAGER_H_
