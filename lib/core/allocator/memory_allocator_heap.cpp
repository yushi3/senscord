/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <vector>
#include "allocator/memory_allocator_heap.h"
#include "allocator/memory_core.h"
#include "senscord/osal.h"

namespace senscord {

/**
 * @brief Allocate memory block.
 * @param[in]  (size) Size to allocate.
 * @param[out] (memory) Allocated Memory.
 * @return Status object.
 */
Status MemoryAllocatorHeap::Allocate(size_t size, Memory** memory) {
  if (memory == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  void* ptr = reinterpret_cast<void*>(osal::OSMalloc(size));
  if (ptr == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseResourceExhausted, "heap memory allocation failed");
  }
  uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
  *memory = new MemoryCore(addr, size, *this);
  return Status::OK();
}

/**
 * @brief Free memory block.
 * @param[in] (memory) Memory to free.
 * @return Status object.
 */
Status MemoryAllocatorHeap::Free(Memory* memory) {
  if (memory) {
    uintptr_t address = memory->GetAddress();
    osal::OSFree(reinterpret_cast<void*>(address));
    delete memory;
  }
  return Status::OK();
}

#ifdef SENSCORD_SERVER
/**
 * @brief Serialize the raw data memory area.
 * @param[in] (rawdata_memory) Memory information for raw data.
 * @param[out] (serialized) Serialized memory information.
 * @return Status object.
 */
Status MemoryAllocatorHeap::ServerSerialize(
    const RawDataMemory& rawdata_memory,
    std::vector<uint8_t>* serialized) const {
  return SENSCORD_STATUS_FAIL(
      kStatusBlockCore, Status::kCauseNotSupported, "not supported");
}

/**
 * @brief Initialize the mapping area.
 * @return Status object.
 */
Status MemoryAllocatorHeap::ClientInitMapping() {
  // do nothing
  return Status::OK();
}

/**
 * @brief Deinitialize the mapping area.
 * @return Status object.
 */
Status MemoryAllocatorHeap::ClientExitMapping() {
  // do nothing
  return Status::OK();
}

/**
 * @brief Mapping memory with serialized memory information.
 * @param[in] (serialized) Serialized memory information.
 * @param[out] (rawdata_memory) Memory information for raw data.
 * @return Status object.
 */
Status MemoryAllocatorHeap::ClientMapping(
    const std::vector<uint8_t>& serialized,
    RawDataMemory* rawdata_memory) {
  if (rawdata_memory == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument, "invalid parameter");
  }

  // same as Allocate
  Status status = Allocate(serialized.size(), &rawdata_memory->memory);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    rawdata_memory->size = serialized.size();
    rawdata_memory->offset = 0;
  }
  return status;
}

/**
 * @brief Release the mapped area.
 * @param[in] (rawdata_memory) Memory information for raw data.
 * @return Status object.
 */
Status MemoryAllocatorHeap::ClientUnmapping(
    const RawDataMemory& rawdata_memory) {
  Status status = Free(rawdata_memory.memory);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_SERVER

/**
 * @brief Whether the memory is shared.
 * @return True means sharing between other process, false means local.
 */
bool MemoryAllocatorHeap::IsMemoryShared() const {
  return false;
}

}   // namespace senscord
