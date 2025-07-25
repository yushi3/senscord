/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/wasm_allocator_manager.h"

#include <stdint.h>
#include <inttypes.h>

#include <map>
#include <string>
#include <utility>

#include "senscord/osal.h"
#include "src/senscord_wamr_util.h"
#include "src/wasm_memory_allocator.h"

#if 0
#include <stdio.h>
#define LOG_E(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define LOG_W(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define LOG_I(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define LOG_D(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#else
#include "senscord/logger.h"
#define LOG_E(...) SENSCORD_LOG_ERROR_TAGGED(kBlockName, __VA_ARGS__)
#define LOG_W(...) SENSCORD_LOG_WARNING_TAGGED(kBlockName, __VA_ARGS__)
#define LOG_I(...) SENSCORD_LOG_INFO_TAGGED(kBlockName, __VA_ARGS__)
#define LOG_D(...) SENSCORD_LOG_DEBUG_TAGGED(kBlockName, __VA_ARGS__)
#endif

namespace {

// status and log block name
const char kBlockName[] = "wasm";

}  // namespace

namespace senscord {

static WasmAllocatorManager* g_allocator_manager = NULL;

/**
 * @brief Create singleton instance.
 */
WasmAllocatorManager* WasmAllocatorManager::CreateInstance() {
  if (g_allocator_manager == NULL) {
    g_allocator_manager = new WasmAllocatorManager;
  }
  return g_allocator_manager;
}

/**
 * @brief Get singleton instance.
 */
WasmAllocatorManager* WasmAllocatorManager::GetInstance() {
  return g_allocator_manager;
}

/**
 * @brief Delete singleton instance.
 */
void WasmAllocatorManager::DeleteInstance() {
  delete g_allocator_manager;
  g_allocator_manager = NULL;
}

struct WasmAllocatorInfo {
  WasmMemoryAllocator* allocator;
  wasm_module_t owner_module;
  int32_t open_count;
  int32_t total_open_count;
};

struct WasmAllocatorManager::Impl {
  osal::OSMutex* mutex;
  // Key=stream key, Value=allocator info
  std::map<std::string, WasmAllocatorInfo> allocators;
};

/**
 * @brief Constructor.
 */
WasmAllocatorManager::WasmAllocatorManager() : pimpl_(new Impl()) {
  osal::OSCreateMutex(&pimpl_->mutex);
}

/**
 * @brief Destructor.
 */
WasmAllocatorManager::~WasmAllocatorManager() {
  osal::OSDestroyMutex(pimpl_->mutex);
  delete pimpl_;
}

/**
 * @brief Create WasmMemoryAllocator.
 * @param[in] (stream_key) Stream key for search.
 * @param[in] (allocator_key) Allocator key for MemoryAllocator.
 * @param[out] (allocator) Generated MemoryAllocator.
 * @return Status object.
 */
Status WasmAllocatorManager::CreateAllocator(
    const std::string& stream_key,
    const std::string& allocator_key,
    MemoryAllocator** allocator) {
  SENSCORD_STATUS_ARGUMENT_CHECK(allocator == NULL);
  LOG_I("CreateAllocator: '%s'", allocator_key.c_str());
  LockGuard _lock(pimpl_->mutex);
  WasmAllocatorInfo& allocator_info = pimpl_->allocators[stream_key];
  if (allocator_info.allocator == NULL) {
    allocator_info.allocator =
        new WasmMemoryAllocator(allocator_key, stream_key);
  }
  *allocator = allocator_info.allocator;
  return Status::OK();
}

/**
 * @brief Delete WasmMemoryAllocator.
 * @param[in] (allocator) MemoryAllocator to delete.
 * @return Status object.
 */
Status WasmAllocatorManager::DeleteAllocator(MemoryAllocator* allocator) {
  SENSCORD_STATUS_ARGUMENT_CHECK(allocator == NULL);
  LOG_I("DeleteAllocator: '%s'", allocator->GetKey().c_str());
  WasmMemoryAllocator* wasm_allocator =
      static_cast<WasmMemoryAllocator*>(allocator);
  LockGuard _lock(pimpl_->mutex);
  std::map<std::string, WasmAllocatorInfo>::iterator itr =
      pimpl_->allocators.find(wasm_allocator->GetStreamKey());
  if (itr != pimpl_->allocators.end()) {
    delete itr->second.allocator;
    pimpl_->allocators.erase(itr);
  }
  return Status::OK();
}

/**
 * @brief Register Wasm environment to Allocator linked to stream key.
 * @param[in] (stream_key) Stream key for search.
 * @param[in] (module_inst) Wasm moudule instance to register.
 * @return Status object.
 */
Status WasmAllocatorManager::RegisterWasm(
    const std::string& stream_key,
    wasm_module_inst_t module_inst) {
  LOG_I("RegisterWasm: stream='%s'", stream_key.c_str());
  Status status;
  LockGuard _lock(pimpl_->mutex);
  std::map<std::string, WasmAllocatorInfo>::iterator itr =
      pimpl_->allocators.find(stream_key);
  if (itr != pimpl_->allocators.end()) {
    WasmAllocatorInfo& info = itr->second;
    wasm_module_t module = wasm_runtime_get_module(module_inst);
    if (info.total_open_count == 0 && info.owner_module == NULL) {
      wasm_exec_env_t exec_env =
          wasm_runtime_get_exec_env_singleton(module_inst);
      status = info.allocator->RegisterWasm(exec_env);
      SENSCORD_STATUS_TRACE(status);
      if (status.ok()) {
        info.owner_module = module;
      }
    }
    if (info.owner_module == module) {
      ++info.open_count;
    }
    if (status.ok()) {
      ++info.total_open_count;
    }
    LOG_D("RegisterWasm:\n"
          "  owner_module = %p\n"
          "  open_count = %" PRIu32 "\n"
          "  total_open_count = %" PRIu32,
          info.owner_module, info.open_count, info.total_open_count);
  }
  return status;
}

/**
 * @brief Unregister Wasm environment from Allocator.
 * @param[in] (stream_key) Stream key for search.
 * @param[in] (module_inst) Wasm moudule instance to unregister.
 * @return Status object.
 */
Status WasmAllocatorManager::UnregisterWasm(
    const std::string& stream_key,
    wasm_module_inst_t module_inst) {
  LOG_I("UnregisterWasm: stream='%s'", stream_key.c_str());
  Status status;
  LockGuard _lock(pimpl_->mutex);
  std::map<std::string, WasmAllocatorInfo>::iterator itr =
      pimpl_->allocators.find(stream_key);
  if (itr != pimpl_->allocators.end()) {
    WasmAllocatorInfo& info = itr->second;
    wasm_module_t module = wasm_runtime_get_module(module_inst);
    if (info.owner_module == module) {
      if (info.open_count == 1) {
        status = info.allocator->UnregisterWasm();
        SENSCORD_STATUS_TRACE(status);
        if (status.ok()) {
          info.owner_module = NULL;
        }
      }
      if (status.ok()) {
        --info.open_count;
      }
    }
    if (status.ok()) {
      --info.total_open_count;
    }
    LOG_D("UnregisterWasm:\n"
          "  owner_module = %p\n"
          "  open_count = %" PRIu32 "\n"
          "  total_open_count = %" PRIu32,
          info.owner_module, info.open_count, info.total_open_count);
  }
  return status;
}

/**
 * @brief Get the state of the WASM Allocator.
 * @param[in] (stream_key) Stream key for search.
 * @param[in] (module_inst) Wasm moudule instance.
 * @return the state of the WASM Allocator.
 */
WasmAllocatorState WasmAllocatorManager::GetAllocatorState(
    const std::string& stream_key,
    wasm_module_inst_t module_inst) const {
  LockGuard _lock(pimpl_->mutex);
  std::map<std::string, WasmAllocatorInfo>::iterator itr =
      pimpl_->allocators.find(stream_key);
  if (itr != pimpl_->allocators.end()) {
    WasmAllocatorInfo& info = itr->second;
    wasm_module_t module = wasm_runtime_get_module(module_inst);
    if (info.owner_module == module) {
      return kOwnedWasm;
    } else {
      return kNotOwnedWasm;
    }
  }
  return kNotWasm;
}

}  // namespace senscord
