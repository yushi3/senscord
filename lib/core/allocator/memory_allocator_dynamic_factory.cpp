/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "allocator/memory_allocator_dynamic_factory.h"

#include <stdint.h>

#include "senscord/develop/memory_allocator_core.h"

namespace senscord {

typedef void* (*CreateMemoryAllocator) ();
typedef void (*DeleteMemoryAllocator) (void*);

/**
 * @brief Constructor.
 */
MemoryAllocatorDynamicFactory::MemoryAllocatorDynamicFactory() {
}

/**
 * @brief Destructor.
 */
MemoryAllocatorDynamicFactory::~MemoryAllocatorDynamicFactory() {
}

 /**
  * @brief Call a function that creates instance.
  * @param[in]  (handle) Pointer to function to create instance.
  * @param[out] (instance) Instance to delete.
  * @return Status object.
 */
Status MemoryAllocatorDynamicFactory::CallCreateInstance(void* handle,
                                                         void** instance) {
  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  if (handle == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  CreateMemoryAllocator create_handle
    = reinterpret_cast<CreateMemoryAllocator>(handle);

  // call function.
  *instance = create_handle();
  if (*instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "create allocator failed : instance_name=%s", instance_name_.c_str());
  }

  return Status::OK();
}

/**
 * @brief Call a function that delete instance.
 * @param[in] (handle) Pointer to function to delete instance.
 * @param[in] (instance) Instance to delete.
 * @return Status object.
*/
Status MemoryAllocatorDynamicFactory::CallDestroyInstance(void* handle,
                                                          void* instance) {
  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  if (handle == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  DeleteMemoryAllocator delete_handle
    = reinterpret_cast<DeleteMemoryAllocator>(handle);

  // call function.
  delete_handle(instance);

  return Status::OK();
}

}  //  namespace senscord
