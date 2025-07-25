/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_ALLOCATOR_MEMORY_ALLOCATOR_DYNAMIC_FACTORY_H_
#define LIB_CORE_ALLOCATOR_MEMORY_ALLOCATOR_DYNAMIC_FACTORY_H_

#include "loader/class_dynamic_factory.h"

namespace senscord {

/**
 * @brief MemoryAllocator dynamic factory.
 */
class MemoryAllocatorDynamicFactory : public ClassDynamicFactory {
 public:
  /**
   * @brief Constructor.
   */
  MemoryAllocatorDynamicFactory();

  /**
   * @brief Destructor.
   */
  ~MemoryAllocatorDynamicFactory();

 protected:
  /**
   * @brief Call a function that creates instance.
   * @param[in]  (handle) Pointer to function to create instance.
   * @param[out] (instance) Instance to delete.
   * @return Status object.
   */
  Status CallCreateInstance(void* handle, void** instance);

  /**
   * @brief Call a function that delete instance.
   * @param[in] (handle) Pointer to function to delete instance.
   * @param[in] (instance) Instance to delete.
   * @return Status object.
   */
  Status CallDestroyInstance(void* handle, void* instance);
};

}   // namespace senscord

#endif  //  LIB_CORE_ALLOCATOR_MEMORY_ALLOCATOR_DYNAMIC_FACTORY_H_
