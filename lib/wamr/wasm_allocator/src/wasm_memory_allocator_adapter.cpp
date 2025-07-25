/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "wasm_memory_allocator_adapter.h"

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "src/wasm_allocator_manager.h"

/**
 * @brief Create allocator instance.
 * @return Created allocator instance. In case of failure, it returns NULL.
 */
extern "C" void* CreateAllocator() {
  return new senscord::WasmMemoryAllocatorAdapter();
}

/**
 * @brief Destroy allocator instance.
 * @param[in] allocator  Instance created in CreateAllocator().
 */
extern "C" void DestroyAllocator(void* allocator) {
  delete reinterpret_cast<senscord::WasmMemoryAllocatorAdapter*>(allocator);
}

namespace {

// status and log block name
const char kBlockName[] = "wasm";

}  // namespace

namespace senscord {

struct WasmMemoryAllocatorAdapter::Impl {
  MemoryAllocator* target;
};

WasmMemoryAllocatorAdapter::WasmMemoryAllocatorAdapter() : pimpl_(new Impl()) {
}

WasmMemoryAllocatorAdapter::~WasmMemoryAllocatorAdapter() {
  delete pimpl_;
}

/**
 * @brief Initialization process.
 * @param[in] (config) Allocator config.
 * @return Status object.
 */
Status WasmMemoryAllocatorAdapter::Init(const AllocatorConfig& config) {
  std::map<std::string, std::string>::const_iterator itr =
      config.arguments.find("stream_key");
  if (itr == config.arguments.end()) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, Status::kCauseNotFound,
        "WasmMemoryAllocator.Init: 'stream_key' argument not found");
  }
  Status status = WasmAllocatorManager::GetInstance()->CreateAllocator(
      itr->second, config.key, &pimpl_->target);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    AllocatorConfig new_config = {};
    new_config.key = "wasm";
    new_config.type = config.type;
    new_config.cacheable = config.cacheable;
    MemoryAllocatorCore::Init(new_config);
  }
  return status;
}

/**
 * @brief Termination process.
 * @return Status object.
 */
Status WasmMemoryAllocatorAdapter::Exit() {
  Status status;
  if (pimpl_->target != NULL) {
    status = WasmAllocatorManager::GetInstance()->DeleteAllocator(
        pimpl_->target);
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      pimpl_->target = NULL;
    }
  }
  return status;
}

/**
 * @brief Allocate memory block.
 * @param[in] (size) Size to allocate.
 * @param[out] (memory) Allocated Memory.
 * @return Status object.
 */
Status WasmMemoryAllocatorAdapter::Allocate(size_t size, Memory** memory) {
  return pimpl_->target->Allocate(size, memory);
}

/**
 * @brief Free memory block.
 * @param[in] (memory) Memory to free.
 * @return Status object.
 */
Status WasmMemoryAllocatorAdapter::Free(Memory* memory) {
  return pimpl_->target->Free(memory);
}

/**
 * @brief Map memory block.
 * @param[in] (memory) Memory to map.
 * @return Status object.
 */
Status WasmMemoryAllocatorAdapter::Map(Memory* memory) {
  return pimpl_->target->Map(memory);
}

/**
 * @brief Unmap memory block.
 * @param[in] (memory) Memory to unmap.
 * @return Status object.
 */
Status WasmMemoryAllocatorAdapter::Unmap(Memory* memory) {
  return pimpl_->target->Unmap(memory);
}

#ifdef SENSCORD_SERVER
/**
 * @brief Serialize the raw data memory area.
 * @param[in] (rawdata_memory) Memory information for raw data.
 * @param[out] (serialized) Serialized memory information.
 * @return Status object.
 */
Status WasmMemoryAllocatorAdapter::ServerSerialize(
    const RawDataMemory& rawdata_memory,
    std::vector<uint8_t>* serialized) const {
  return pimpl_->target->ServerSerialize(rawdata_memory, serialized);
}

/**
 * @brief Initialize the mapping area.
 * @return Status object.
 */
Status WasmMemoryAllocatorAdapter::ClientInitMapping() {
  return pimpl_->target->ClientInitMapping();
}

/**
 * @brief Deinitialize the mapping area.
 * @return Status object.
 */
Status WasmMemoryAllocatorAdapter::ClientExitMapping() {
  return pimpl_->target->ClientExitMapping();
}

/**
 * @brief Mapping memory with serialized memory information.
 * @param[in] (serialized) Serialized memory information.
 * @param[out] (rawdata_memory) Memory information for raw data.
 * @return Status object.
 */
Status WasmMemoryAllocatorAdapter::ClientMapping(
    const std::vector<uint8_t>& serialized,
    RawDataMemory* rawdata_memory) {
  return pimpl_->target->ClientMapping(serialized, rawdata_memory);
}

/**
 * @brief Release the mapped area.
 * @param[in] (rawdata_memory) Memory information for raw data.
 * @return Status object.
 */
Status WasmMemoryAllocatorAdapter::ClientUnmapping(
    const RawDataMemory& rawdata_memory) {
  return pimpl_->target->ClientUnmapping(rawdata_memory);
}
#endif  // SENSCORD_SERVER

/**
 * @brief Whether the memory is shared.
 * @return Always returns false.
 */
bool WasmMemoryAllocatorAdapter::IsMemoryShared() const {
  return pimpl_->target->IsMemoryShared();
}

}  // namespace senscord
