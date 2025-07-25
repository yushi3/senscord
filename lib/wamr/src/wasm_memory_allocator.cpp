/*
 * SPDX-FileCopyrightText: 2023-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/wasm_memory_allocator.h"

#include <stdint.h>
#include <inttypes.h>

#include <string>
#include <map>
#include <vector>
#include <set>

#include "senscord/osal.h"
#include "senscord/logger.h"
#include "src/senscord_wamr_util.h"
#include "src/wasm_memory.h"

namespace {

// status and log block name
const char kBlockName[] = "wasm";

}  // namespace

namespace senscord {

struct WasmMemoryAllocator::Impl {
  wasm_exec_env_t exec_env;  // spawned exec_env
  senscord::osal::OSMutex* mutex;
  std::string allocator_key;
  std::string allocator_type;
  std::string stream_key;
  std::set<WasmMemory*> memory_list;
};

/**
 * @brief Constructor.
 * @param[in] (allocator_key) Allocator key.
 * @param[in] (stream_key) Stream key.
 */
WasmMemoryAllocator::WasmMemoryAllocator(
    const std::string& allocator_key, const std::string& stream_key)
    : pimpl_(new Impl()) {
  senscord::osal::OSCreateMutex(&pimpl_->mutex);
  pimpl_->allocator_key = allocator_key;
  pimpl_->allocator_type = "wasm_allocator";
  pimpl_->stream_key = stream_key;
}

/**
 * @brief Destructor.
 */
WasmMemoryAllocator::~WasmMemoryAllocator() {
  UnregisterWasm();
  {
    LockGuard _lock(pimpl_->mutex);
    for (std::set<WasmMemory*>::const_iterator
        itr = pimpl_->memory_list.begin(), end = pimpl_->memory_list.end();
        itr != end; ++itr) {
      delete *itr;
    }
    pimpl_->memory_list.clear();
  }
  senscord::osal::OSDestroyMutex(pimpl_->mutex);
  delete pimpl_;
}

/**
 * @brief Register the Wasm module.
 * @param[in] (exec_env) Wasm execution environment.
 * @return Status object.
 */
Status WasmMemoryAllocator::RegisterWasm(wasm_exec_env_t exec_env) {
  wasm_exec_env_t spawned_exec_env = wasm_runtime_spawn_exec_env(exec_env);
  if (spawned_exec_env == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseResourceExhausted,
        "[%s] wasm_runtime_spawn_exec_env(%p) failed",
        pimpl_->allocator_key.c_str(), exec_env);
  }
  LockGuard _lock(pimpl_->mutex);
  pimpl_->exec_env = spawned_exec_env;
  SENSCORD_LOG_INFO_TAGGED(
      kBlockName, "[%s] wasm_runtime_spawn_exec_env: %p (input=%p)",
      pimpl_->allocator_key.c_str(), pimpl_->exec_env, exec_env);
  return Status::OK();
}

/**
 * @brief Unregister the Wasm module.
 * @return Status object.
 */
Status WasmMemoryAllocator::UnregisterWasm() {
  wasm_exec_env_t spawned_exec_env = NULL;
  {
    LockGuard _lock(pimpl_->mutex);
    spawned_exec_env = pimpl_->exec_env;
    pimpl_->exec_env = NULL;
    if (spawned_exec_env != NULL) {
      for (std::set<WasmMemory*>::const_iterator
          itr = pimpl_->memory_list.begin(), end = pimpl_->memory_list.end();
          itr != end; ++itr) {
        WasmMemory* wasm_memory = *itr;
        uint32_t wasm_address = wasm_memory->GetWasmAddress();
        if (wasm_address != 0) {
          wasm_module_inst_t inst =
              wasm_runtime_get_module_inst(spawned_exec_env);
          wasm_runtime_module_free(inst, wasm_address);
          wasm_memory->SetAddress(0);
          wasm_memory->SetWasmAddress(0);
          // NOTE: Memory objects are not deleted here,
          // they are deleted at `WasmMemoryAllocator::Free`.
        }
      }
    }
  }
  if (spawned_exec_env != NULL) {
    wasm_runtime_destroy_spawned_exec_env(spawned_exec_env);
    SENSCORD_LOG_INFO_TAGGED(
        kBlockName, "[%s] wasm_runtime_destroy_spawned_exec_env: %p",
        pimpl_->allocator_key.c_str(), spawned_exec_env);
  }
  return Status::OK();
}

/**
 * @brief Allocate memory block.
 * @param[in] (size) Size to allocate.
 * @param[out] (memory) Allocated Memory.
 * @return Status object.
 */
Status WasmMemoryAllocator::Allocate(size_t size, Memory** memory) {
  SENSCORD_STATUS_ARGUMENT_CHECK(memory == NULL);
  SENSCORD_STATUS_ARGUMENT_CHECK((size == 0) || (size > 0xffffffff));
  uint32_t alloc_size = static_cast<uint32_t>(size);
  void* native_address = NULL;
  uint32_t wasm_address = 0;

  LockGuard _lock(pimpl_->mutex);
  if (pimpl_->exec_env == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidOperation,
        "[%s] wasm module is not registered.",
        pimpl_->allocator_key.c_str());
  }
  {
    WasmThreadEnv _env;
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(pimpl_->exec_env);
    wasm_address = wasm_runtime_module_malloc(
        inst, alloc_size, &native_address);
  }
  if ((native_address == NULL) && (wasm_address == 0)) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseResourceExhausted,
        "[%s] wasm_runtime_module_malloc(%" PRIu32 ") failed",
        pimpl_->allocator_key.c_str(), alloc_size);
  }
  WasmMemory* wasm_memory = new WasmMemory(
      reinterpret_cast<uintptr_t>(native_address), wasm_address, alloc_size,
      this);
  pimpl_->memory_list.insert(wasm_memory);
  *memory = wasm_memory;
  return Status::OK();
}

/**
 * @brief Free memory block.
 * @param[in] (memory) Memory to free.
 * @return Status object.
 */
Status WasmMemoryAllocator::Free(Memory* memory) {
  LockGuard _lock(pimpl_->mutex);
  std::set<WasmMemory*>::iterator itr =
      pimpl_->memory_list.find(static_cast<WasmMemory*>(memory));
  if (itr != pimpl_->memory_list.end()) {
    WasmMemory* wasm_memory = *itr;
    uint32_t wasm_address = wasm_memory->GetWasmAddress();
    if ((wasm_address != 0) && (pimpl_->exec_env != NULL)) {
      WasmThreadEnv _env;
      wasm_module_inst_t inst = wasm_runtime_get_module_inst(pimpl_->exec_env);
      wasm_runtime_module_free(inst, wasm_address);
    }
    delete memory;
    pimpl_->memory_list.erase(itr);
  }
  return Status::OK();
}

/**
 * @brief Map memory block.
 * @param[in] (memory) Memory to map.
 * @return Status object.
 */
Status WasmMemoryAllocator::Map(Memory* memory) {
  return Status::OK();
}

/**
 * @brief Unmap memory block.
 * @param[in] (memory) Memory to unmap.
 * @return Status object.
 */
Status WasmMemoryAllocator::Unmap(Memory* memory) {
  return Status::OK();
}

#ifdef SENSCORD_SERVER
/**
 * @brief Serialize the raw data memory area.
 * @param[in] (rawdata_memory) Memory information for raw data.
 * @param[out] (serialized) Serialized memory information.
 * @return Status object.
 */
Status WasmMemoryAllocator::ServerSerialize(
    const RawDataMemory& rawdata_memory,
    std::vector<uint8_t>* serialized) const {
  return SENSCORD_STATUS_FAIL(
      kBlockName, Status::kCauseNotSupported, "not supported");
}

/**
 * @brief Initialize the mapping area.
 * @return Status object.
 */
Status WasmMemoryAllocator::ClientInitMapping() {
  // do nothing
  return Status::OK();
}

/**
 * @brief Deinitialize the mapping area.
 * @return Status object.
 */
Status WasmMemoryAllocator::ClientExitMapping() {
  // do nothing
  return Status::OK();
}

/**
 * @brief Mapping memory with serialized memory information.
 * @param[in] (serialized) Serialized memory information.
 * @param[out] (rawdata_memory) Memory information for raw data.
 * @return Status object.
 */
Status WasmMemoryAllocator::ClientMapping(
    const std::vector<uint8_t>& serialized,
    RawDataMemory* rawdata_memory) {
  SENSCORD_STATUS_ARGUMENT_CHECK(rawdata_memory == NULL);
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
Status WasmMemoryAllocator::ClientUnmapping(
    const RawDataMemory& rawdata_memory) {
  Status status = Free(rawdata_memory.memory);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_SERVER

/**
 * @brief Invalidate of cache.
 * @param[in] (address) Start virtual address to invalidate.
 * @param[in] (size) Size to invalidate.
 * @return Status object.
 */
Status WasmMemoryAllocator::InvalidateCache(uintptr_t address, size_t size) {
  return SENSCORD_STATUS_FAIL(
      kBlockName, Status::kCauseNotSupported, "not supported");
}

/**
 * @brief Clean of cache.
 * @param[in] (address) Start virtual address to clean.
 * @param[in] (size) Size to clean.
 * @return Status object.
 */
Status WasmMemoryAllocator::CleanCache(uintptr_t address, size_t size) {
  return SENSCORD_STATUS_FAIL(
      kBlockName, Status::kCauseNotSupported, "not supported");
}

/**
 * @brief Get allocator key.
 * @return Allocator key.
 */
const std::string& WasmMemoryAllocator::GetKey() const {
  return pimpl_->allocator_key;
}

/**
 * @brief Get allocator type.
 * @return Allocator type.
 */
const std::string& WasmMemoryAllocator::GetType() const {
  return pimpl_->allocator_type;
}

/**
 * @brief Whether the memory is shared.
 * @return Always returns false.
 */
bool WasmMemoryAllocator::IsMemoryShared() const {
  return false;
}

/**
 * @brief Is cacheable allocator.
 * @return Always returns false.
 */
bool WasmMemoryAllocator::IsCacheable() const {
  return false;
}

/**
 * @brief Get stream key.
 * @return Stream key.
 */
const std::string& WasmMemoryAllocator::GetStreamKey() const {
  return pimpl_->stream_key;
}

}  // namespace senscord
