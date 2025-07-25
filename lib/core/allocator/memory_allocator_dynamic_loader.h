/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_ALLOCATOR_MEMORY_ALLOCATOR_DYNAMIC_LOADER_H_
#define LIB_CORE_ALLOCATOR_MEMORY_ALLOCATOR_DYNAMIC_LOADER_H_

#include <string>
#include <vector>

#include "loader/class_dynamic_loader.h"
#include "senscord/develop/memory_allocator_core.h"
#include "core/internal_types.h"

namespace senscord {

/**
 * @brief MemoryAllocator dynamic loader.
 */
class MemoryAllocatorDynamicLoader : public ClassDynamicLoader {
 public:
  /**
   * @brief Constructor.
   */
  MemoryAllocatorDynamicLoader();

  /**
   * @brief Destructor.
   */
  ~MemoryAllocatorDynamicLoader();

  /**
   * @brief Generate an instance based on the memory allocator name of the
            argument.
   * @param[in]  (name) Name of memory allocator.
   * @param[out] (memory_allocator) Where to store the created MemoryAllocator.
   * @return Status object.
   */
  Status Create(const std::string& name,
                 MemoryAllocatorCore** memory_allocator);

  /**
   * @brief Delete the component passed in the argument.
   * @param[in] (name) Name of memory allocator.
   * @param[in] (memory_allocator) MemoryAllocator to delete.
   * @return Status object.
   */
  Status Destroy(const std::string& name,
                  MemoryAllocatorCore* memory_allocator);

 protected:
  /**
   * @brief A function that loads a library based on the argument name.
   * @param[in] (name) Key name of library.
   * @return Status object.
   */
  Status Load(const std::string& name);
};

}   // namespace senscord

#endif  //  LIB_CORE_ALLOCATOR_MEMORY_ALLOCATOR_DYNAMIC_LOADER_H_
