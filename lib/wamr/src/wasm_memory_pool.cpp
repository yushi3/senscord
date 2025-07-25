/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/wasm_memory_pool.h"

#include <stdint.h>
#include <inttypes.h>

#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include "senscord/osal.h"
#include "src/senscord_wamr_util.h"
#include "src/wasm_memory.h"

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

/**
 * @brief Information about wasm memory in use.
 */
struct WasmMemoryInfo {
  senscord::WasmMemory* acquired_memory;
  uint32_t current_offset;
  std::map<uint32_t, senscord::WasmMemoryArea> channel_area;
  std::set<senscord_frame_t> ref_frames;
};

/**
 * @brief Converts memory information to a string.
 */
std::string ToString(const std::map<uint64_t, WasmMemoryInfo>& list) {
  std::ostringstream buf;
  buf << '{';
  for (std::map<uint64_t, WasmMemoryInfo>::const_iterator
      itr = list.begin(), end = list.end(); itr != end; ++itr) {
    buf << "(seq_num=" << itr->first;
    buf << ",memory=" << itr->second.acquired_memory;
    buf << ",ref=" << itr->second.ref_frames.size();
    buf << ")";
  }
  buf << '}';
  return buf.str();
}

}  // namespace

namespace senscord {

struct WasmMemoryPool::Impl {
  senscord::osal::OSMutex* mutex;
  std::map<uint64_t, WasmMemoryInfo> used_memory;
  std::list<WasmMemory*> free_memory;
  std::set<senscord_stream_t> opened_stream;
  std::set<senscord_stream_t> running_stream;
  uint32_t num;
  uint32_t size;
  uint32_t reserved_num;
  uint32_t reserved_size;
};

/**
 * @brief Constructor.
 */
WasmMemoryPool::WasmMemoryPool() : pimpl_(new Impl()) {
  senscord::osal::OSCreateMutex(&pimpl_->mutex);
}

/**
 * @brief Destructor.
 */
WasmMemoryPool::~WasmMemoryPool() {
  {
    LockGuard _lock(pimpl_->mutex);
    DeletePool(NULL);
  }
  senscord::osal::OSDestroyMutex(pimpl_->mutex);
  delete pimpl_;
}

/**
 * @brief Sets the number of memory chunks.
 * @param[in] num  Number of memory chunks
 */
void WasmMemoryPool::SetNum(uint32_t num) {
  LockGuard _lock(pimpl_->mutex);
  pimpl_->reserved_num = num;
}

/**
 * @brief Sets the memory chunk size.
 * @param[in] size  Memory chunk size.
 */
void WasmMemoryPool::SetSize(uint32_t size) {
  LockGuard _lock(pimpl_->mutex);
  pimpl_->reserved_size = size;
}

/**
 * @brief Gets the number of memory chunks.
 * @return Number of memory chunks
 */
uint32_t WasmMemoryPool::GetNum() const {
  LockGuard _lock(pimpl_->mutex);
  return IsRunning() ? pimpl_->num : pimpl_->reserved_num;
}

/**
 * @brief Gets the memory chunk size.
 * @return Memory chunk size.
 */
uint32_t WasmMemoryPool::GetSize() const {
  LockGuard _lock(pimpl_->mutex);
  return IsRunning() ? pimpl_->size : pimpl_->reserved_size;
}

/**
 * @brief Returns true if memory pool is running.
 */
bool WasmMemoryPool::IsRunning() const {
  LockGuard _lock(pimpl_->mutex);
  return !pimpl_->running_stream.empty();
}

/**
 * @brief Returns true if memory pool is closed.
 */
bool WasmMemoryPool::IsClosed() const {
  LockGuard _lock(pimpl_->mutex);
  return pimpl_->opened_stream.empty();
}

/**
 * @brief Changes state when stream is opened.
 * @param[in] stream  Stream handle.
 */
void WasmMemoryPool::Open(senscord_stream_t stream) {
  LOG_D("Open: stream='%s'", senscord_stream_get_key(stream));
  LockGuard _lock(pimpl_->mutex);
  pimpl_->opened_stream.insert(stream);
}

/**
 * @brief Deletes memory pool when stream is closed.
 * @param[in] stream  Stream handle.
 * @param[in] module_inst  Wasm module instance.
 */
void WasmMemoryPool::Close(
    senscord_stream_t stream,
    wasm_module_inst_t module_inst) {
  LOG_D("Close: stream='%s'", senscord_stream_get_key(stream));
  LockGuard _lock(pimpl_->mutex);
  pimpl_->opened_stream.erase(stream);
  if (pimpl_->opened_stream.empty()) {
    DeletePool(module_inst);
  }
}

/**
 * @brief Creates memory pool when stream is started.
 * @param[in] stream  Stream handle.
 * @param[in] module_inst  Wasm module instance.
 * @return Status object.
 */
Status WasmMemoryPool::Start(
    senscord_stream_t stream,
    wasm_module_inst_t module_inst) {
  LOG_D("Start: stream='%s'", senscord_stream_get_key(stream));
  LockGuard _lock(pimpl_->mutex);
  if (pimpl_->running_stream.empty()) {
    if (!pimpl_->used_memory.empty()) {
      return SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseBusy,
          "Unreleased frames exist: %s",
          ToString(pimpl_->used_memory).c_str());
    }
    if ((pimpl_->num != pimpl_->reserved_num) ||
        (pimpl_->size != pimpl_->reserved_size)) {
      DeletePool(module_inst);
      Status status = CreatePool(module_inst);
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
    }
  }
  pimpl_->running_stream.insert(stream);
  return Status::OK();
}

/**
 * @brief Changes state when stream is stopped.
 * @param[in] stream  Stream handle.
 */
void WasmMemoryPool::Stop(senscord_stream_t stream) {
  LOG_D("Stop: stream='%s'", senscord_stream_get_key(stream));
  LockGuard _lock(pimpl_->mutex);
  pimpl_->running_stream.erase(stream);
}

/**
 * @brief Reserves frame memory in memory pool.
 * @param[in] module_inst  Wasm module instance.
 * @param[in] frame  Frame handle.
 * @return Status object.
 */
Status WasmMemoryPool::ReserveFrameMemory(
    wasm_module_inst_t module_inst,
    senscord_frame_t frame) {
  LOG_D("ReserveFrameMemory: stream='%s', frame=%" PRIu64,
        senscord_stream_get_key(senscord_frame_get_parent_stream(frame)),
        frame);
  LockGuard _lock(pimpl_->mutex);

  // size == 0: Creates memory pool of adjusted size.
  if ((pimpl_->num > 0) && (pimpl_->size == 0)) {
    uint32_t total_size = 0;
    uint32_t channel_count = 0;
    senscord_frame_get_channel_count(frame, &channel_count);
    for (uint32_t i = 0; i < channel_count; ++i) {
      senscord_channel_t channel = 0;
      senscord_frame_get_channel(frame, i, &channel);
      senscord_raw_data_t raw_data = {};
      senscord_channel_get_raw_data(channel, &raw_data);
      total_size += raw_data.size;
    }
    pimpl_->reserved_size = total_size;
    Status status = CreatePool(module_inst);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }

  uint64_t seq_num = 0;
  senscord_frame_get_sequence_number(frame, &seq_num);
  WasmMemoryInfo& memory_info = pimpl_->used_memory[seq_num];
  if ((pimpl_->num > 0) && (memory_info.acquired_memory == NULL)) {
    if (pimpl_->free_memory.empty()) {
      pimpl_->used_memory.erase(seq_num);
      return SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseResourceExhausted,
          "There is no free memory.");
    }
    memory_info.acquired_memory = pimpl_->free_memory.front();
    pimpl_->free_memory.pop_front();
    LOG_D("ReserveFrameMemory: free_memory size=%" PRIuS,
          pimpl_->free_memory.size());
    LOG_D("ReleaseFrameMemory: used_memory size=%" PRIuS,
          pimpl_->used_memory.size());
  }
  memory_info.ref_frames.insert(frame);

  return Status::OK();
}

/**
 * @brief Releases frame memory.
 * @param[in] module_inst  Wasm module instance.
 * @param[in] frame  Frame handle.
 */
void WasmMemoryPool::ReleaseFrameMemory(
    wasm_module_inst_t module_inst,
    senscord_frame_t frame) {
  LOG_D("ReleaseFrameMemory: stream='%s', frame=%" PRIu64,
        senscord_stream_get_key(senscord_frame_get_parent_stream(frame)),
        frame);
  uint64_t seq_num = 0;
  senscord_frame_get_sequence_number(frame, &seq_num);
  LockGuard _lock(pimpl_->mutex);
  std::map<uint64_t, WasmMemoryInfo>::iterator itr =
      pimpl_->used_memory.find(seq_num);
  if (itr != pimpl_->used_memory.end()) {
    WasmMemoryInfo& memory_info = itr->second;
    memory_info.ref_frames.erase(frame);
    if (memory_info.ref_frames.empty()) {
      if (memory_info.acquired_memory != NULL) {
        // return memory to memory pool.
        pimpl_->free_memory.push_back(memory_info.acquired_memory);
        LOG_D("ReleaseFrameMemory: free_memory size=%" PRIuS,
              pimpl_->free_memory.size());
      } else {
        // free memory.
        for (std::map<uint32_t, WasmMemoryArea>::const_iterator
            ch_itr = memory_info.channel_area.begin(),
            ch_end = memory_info.channel_area.end();
            ch_itr != ch_end; ++ch_itr) {
          WasmMemory* memory = ch_itr->second.memory;
          if (memory == NULL) {
            continue;
          }
          LOG_D("ReleaseFrameMemory: free wasm address=%" PRIu32,
                memory->GetWasmAddress());
          wasm_runtime_module_free(module_inst, memory->GetWasmAddress());
          delete memory;
        }
      }
      pimpl_->used_memory.erase(itr);
      LOG_D("ReleaseFrameMemory: used_memory size=%" PRIuS,
            pimpl_->used_memory.size());
    }
  }
}

/**
 * @brief Obtains channel memory from memory pool.
 * @param[in] module_inst  Wasm module instance.
 * @param[in] frame  Frame handle.
 * @param[in] channel  Channel handle.
 * @param[out] memory_area  Memory area.
 * @return Status object.
 */
Status WasmMemoryPool::GetChannelMemory(
    wasm_module_inst_t module_inst,
    senscord_frame_t frame,
    senscord_channel_t channel,
    WasmMemoryArea* memory_area) {
  LOG_D("GetChannelMemory: stream='%s', frame=%" PRIu64,
        senscord_stream_get_key(senscord_frame_get_parent_stream(frame)),
        frame);
  uint64_t seq_num = 0;
  senscord_frame_get_sequence_number(frame, &seq_num);
  uint32_t channel_id = 0;
  senscord_channel_get_channel_id(channel, &channel_id);
  senscord_raw_data_t raw_data = {};
  senscord_channel_get_raw_data(channel, &raw_data);
  LockGuard _lock(pimpl_->mutex);
  WasmMemoryInfo& memory_info = pimpl_->used_memory[seq_num];
  WasmMemoryArea& area = memory_info.channel_area[channel_id];
  if (area.memory == NULL) {
    if (memory_info.acquired_memory != NULL) {
      WasmMemory* memory = memory_info.acquired_memory;
      uint32_t next_offset = memory_info.current_offset + raw_data.size;
      if (memory->GetSize() < next_offset) {
        return SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseDataLoss,
          "There is no free memory for the channel.");
      }
      osal::OSMemcpy(reinterpret_cast<void*>(
          memory->GetAddress() + memory_info.current_offset),
          memory->GetSize() - memory_info.current_offset,
          raw_data.address, raw_data.size);
      area.memory = memory;
      area.offset = memory_info.current_offset;
      area.size = raw_data.size;
      memory_info.current_offset = next_offset;
    } else {
      uint32_t wasm_address = wasm_runtime_module_dup_data(
          module_inst, reinterpret_cast<const char*>(raw_data.address),
          raw_data.size);
      if (wasm_address == 0) {
        return SENSCORD_STATUS_FAIL(
            kBlockName, senscord::Status::kCauseResourceExhausted,
            "[pool] wasm_runtime_module_dup_data(%" PRIu32 ") failed",
            pimpl_->reserved_size);
      }
      void* native_address =
          wasm_runtime_addr_app_to_native(module_inst, wasm_address);
      area.memory = new WasmMemory(
          reinterpret_cast<uintptr_t>(native_address), wasm_address,
          raw_data.size, NULL);
      area.offset = 0;
      area.size = raw_data.size;
    }
  }
  *memory_area = area;
  return Status::OK();
}

/**
 * @brief Creates memory pool.
 * @param[in] module_inst  Wasm module instance.
 * @return Status object.
 */
Status WasmMemoryPool::CreatePool(wasm_module_inst_t module_inst) {
  LOG_D("CreatePool: module_inst=%p", module_inst);
  if (pimpl_->reserved_size > 0) {
    for (uint32_t i = 0; i < pimpl_->reserved_num; ++i) {
      void* native_address = NULL;
      uint32_t wasm_address = wasm_runtime_module_malloc(
          module_inst, pimpl_->reserved_size, &native_address);
      if ((native_address == NULL) && (wasm_address == 0)) {
        return SENSCORD_STATUS_FAIL(
            kBlockName, senscord::Status::kCauseResourceExhausted,
            "[pool] wasm_runtime_module_malloc(%" PRIu32 ") failed",
            pimpl_->reserved_size);
      }
      WasmMemory* memory = new WasmMemory(
          reinterpret_cast<uintptr_t>(native_address), wasm_address,
          pimpl_->reserved_size, NULL);
      pimpl_->free_memory.push_back(memory);
    }
  }
  pimpl_->num = pimpl_->reserved_num;
  pimpl_->size = pimpl_->reserved_size;
  LOG_D("CreatePool: num=%" PRIu32 ", size=%" PRIu32,
        pimpl_->num, pimpl_->size);
  return Status::OK();
}

/**
 * @brief Deletes memory pool.
 * @param[in] module_inst  Wasm module instance.
 */
void WasmMemoryPool::DeletePool(wasm_module_inst_t module_inst) {
  LOG_D("DeletePool: module_inst=%p", module_inst);

  // used_memory
  LOG_D("DeletePool: used_memory size=%" PRIuS,
        pimpl_->used_memory.size());
  for (std::map<uint64_t, WasmMemoryInfo>::const_iterator
      itr = pimpl_->used_memory.begin(), end = pimpl_->used_memory.end();
      itr != end; ++itr) {
    const WasmMemoryInfo& memory_info = itr->second;
    if (memory_info.acquired_memory != NULL) {
      if (module_inst != NULL) {
        wasm_runtime_module_free(
            module_inst, memory_info.acquired_memory->GetWasmAddress());
      }
      delete memory_info.acquired_memory;
    } else {
      for (std::map<uint32_t, WasmMemoryArea>::const_iterator
          ch_itr = memory_info.channel_area.begin(),
          ch_end = memory_info.channel_area.end();
          ch_itr != ch_end; ++ch_itr) {
        WasmMemory* memory = ch_itr->second.memory;
        if (module_inst != NULL) {
          wasm_runtime_module_free(module_inst, memory->GetWasmAddress());
        }
        delete memory;
      }
    }
  }
  pimpl_->used_memory.clear();

  // free_memory
  LOG_D("DeletePool: free_memory size=%" PRIuS,
        pimpl_->free_memory.size());
  for (std::list<WasmMemory*>::const_iterator
      itr = pimpl_->free_memory.begin(), end = pimpl_->free_memory.end();
      itr != end; ++itr) {
    WasmMemory* memory = *itr;
    if (module_inst != NULL) {
      wasm_runtime_module_free(module_inst, memory->GetWasmAddress());
    }
    delete memory;
  }
  pimpl_->free_memory.clear();
}

}  // namespace senscord
