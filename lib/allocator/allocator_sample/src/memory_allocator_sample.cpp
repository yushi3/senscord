/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "memory_allocator_sample.h"
#include <stdint.h>
#include <inttypes.h>
#include <string>
#include <map>
#include <vector>
#include "memory_sample.h"
#include "senscord/osal.h"

/**
 * @brief Create allocator instance.
 * @return Created allocator instance. In case of failure, it returns NULL.
 */
extern "C" void* CreateAllocator() {
  return new senscord::MemoryAllocatorSample();
}

/**
 * @brief Destroy allocator instance.
 * @param[in] allocator  Instance created in CreateAllocator().
 */
extern "C" void DestroyAllocator(void* allocator) {
  delete reinterpret_cast<senscord::MemoryAllocatorSample*>(allocator);
}

namespace senscord {

/**
 * @brief Initialization.
 * @param[in] (config) Allocator config.
 * @return Status object.
 */
Status MemoryAllocatorSample::Init(const AllocatorConfig& config) {
  // print arguments
  senscord::osal::OSPrintf("[sample allocator] Init args:\n");
  std::map<std::string, std::string>::const_iterator itr;
  for (itr = config.arguments.begin();
       itr != config.arguments.end(); ++itr) {
    senscord::osal::OSPrintf("  %s=%s\n", (itr->first).c_str(),
                             (itr->second).c_str());
  }

  // set allocator info
  MemoryAllocatorCore::Init(config);
  return Status::OK();
}

/**
 * @brief Allocate memory block.
 * @param[in]  (size) Size to allocate.
 * @param[out] (memory) Allocated Memory.
 * @return Status object.
 */
Status MemoryAllocatorSample::Allocate(size_t size, Memory** memory) {
  if (memory == NULL) {
    return SENSCORD_STATUS_FAIL("", Status::kCauseInvalidArgument,
        "invalid parameter");
  }
  void* ptr = reinterpret_cast<void*>(osal::OSMalloc(size));
  if (ptr == NULL) {
    return SENSCORD_STATUS_FAIL("", Status::kCauseResourceExhausted,
        "memory allocation failed");
  }
  uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
  *memory = new SampleMemory(addr, size, *this);
  return Status::OK();
}

/**
 * @brief Free memory block.
 * @param[in] (memory) Memory to free.
 * @return Status object.
 */
Status MemoryAllocatorSample::Free(Memory* memory) {
  if (memory) {
    uintptr_t addr = memory->GetAddress();
    osal::OSFree(reinterpret_cast<void*>(addr));
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
Status MemoryAllocatorSample::ServerSerialize(
    const RawDataMemory& memory,
    std::vector<uint8_t>* serialized) const {
  return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
      "not supported");
}

/**
 * @brief Initialize the mapping area.
 * @return Status object.
 */
Status MemoryAllocatorSample::ClientInitMapping() {
  // do nothing
  return Status::OK();
}

/**
 * @brief Deinitialize the mapping area.
 * @return Status object.
 */
Status MemoryAllocatorSample::ClientExitMapping() {
  // do nothing
  return Status::OK();
}

/**
 * @brief Mapping memory with serialized memory information.
 * @param[in] (serialized) Serialized memory information.
 * @param[out] (rawdata_memory) Memory information for raw data.
 * @return Status object.
 */
Status MemoryAllocatorSample::ClientMapping(
    const std::vector<uint8_t>& serialized,
    RawDataMemory* rawdata_memory) {
  if (rawdata_memory == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
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
Status MemoryAllocatorSample::ClientUnmapping(
    const RawDataMemory& rawdata_memory) {
  Status status = Free(rawdata_memory.memory);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_SERVER

/**
 * @brief Whether the memory is shared.
 * @return True means sharing between other process, false means local.
 */
bool MemoryAllocatorSample::IsMemoryShared() const {
  return false;
}

}  // namespace senscord
