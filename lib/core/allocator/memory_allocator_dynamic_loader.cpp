/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "allocator/memory_allocator_dynamic_loader.h"

#include "allocator/memory_allocator_dynamic_factory.h"
#include "senscord/develop/memory_allocator_core.h"
#include "senscord/osal.h"
#include "core/internal_types.h"

namespace senscord {

// Name of the function to be obtained from the library.
const char kCreateInstance[] = "CreateAllocator";
const char kDestroyInstance[] = "DestroyAllocator";

/**
 * @brief Constructor.
 */
MemoryAllocatorDynamicLoader::MemoryAllocatorDynamicLoader() {
}

/**
 * @brief Destructor.
 */
MemoryAllocatorDynamicLoader::~MemoryAllocatorDynamicLoader() {
}

/**
 * @brief Generate an instance based on the memory allocator name of the
          argument.
 * @param[in]  (name) Name of memory allocator.
 * @param[out] (memory_allocator) Where to store the created MemoryAllocator.
 * @return Status object.
 */
Status MemoryAllocatorDynamicLoader::Create(
    const std::string& name,
    MemoryAllocatorCore** memory_allocator) {
  Status ret = ClassDynamicLoader::Create(
      name, reinterpret_cast<void**>(memory_allocator));
  return SENSCORD_STATUS_TRACE(ret);
}

/**
 * @brief Delete the memory allocator passed in the argument.
 * @param[in] (name) Name of memory allocator.
 * @param[in] (memory_allocator) MemoryAllocator to delete.
 * @return Status object.
 */
Status MemoryAllocatorDynamicLoader::Destroy(
    const std::string& name,
    MemoryAllocatorCore* memory_allocator) {
  Status ret = ClassDynamicLoader::Destroy(name, memory_allocator);
  return SENSCORD_STATUS_TRACE(ret);
}

/**
 * @brief A function that loads a library based on the argument name.
 * @param[in] (name) Key name of library.
 * @return Status object.
 */
Status MemoryAllocatorDynamicLoader::Load(const std::string& name) {
  std::string file_path;
  Status ret = GetLibraryPath(name, &file_path);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    return ret;
  }

  // Register allocator factory as loader.
  MemoryAllocatorDynamicFactory* factory = new MemoryAllocatorDynamicFactory();

  ret = LoadAndRegisterLibrary(file_path, kCreateInstance,
      kDestroyInstance, factory);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    delete factory;
    return ret;
  }

  SetFactory(name, factory);

  return Status::OK();
}

}   // namespace senscord
