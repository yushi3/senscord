/*
 * SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "./shared_memory_manager.h"

#include <inttypes.h>

#include <map>
#include <string>
#include <sstream>

#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"

#ifdef _WIN32
#include "./shared_memory_object_windows.h"
#else
#include "./shared_memory_object_linux.h"
#endif  // _WIN32

namespace senscord {
namespace ws {

/**
 * @brief Get manager instance.
 * @return Manager instance.
 */
SharedMemoryManager* SharedMemoryManager::GetInstance() {
  static SharedMemoryManager instance;
  return &instance;
}

/**
 * @brief Constructor.
 */
SharedMemoryManager::SharedMemoryManager() :
    mutex_(), name_index_(), params_(), memory_size_list_() {
  osal::OSCreateMutex(&mutex_);
}

/**
 * @brief Destructor.
 */
SharedMemoryManager::~SharedMemoryManager() {
  osal::OSLockMutex(mutex_);
  for (std::map<uint64_t, SharedMemoryParameter>::const_iterator
      itr = params_.begin(), end = params_.end(); itr != end; ++itr) {
    osal::OSLockMutex(itr->second.mutex);
    delete itr->second.memory_object;
    osal::OSUnlockMutex(itr->second.mutex);
    osal::OSDestroyMutex(itr->second.mutex);
  }
  params_.clear();
  osal::OSUnlockMutex(mutex_);

  osal::OSDestroyMutex(mutex_);
  mutex_ = NULL;
}

/**
 * @brief Opens a memory object.
 * @param[in]  stream_id    Stream ID for identification.
 * @param[in]  size         Size of memory object.
 * @param[out] memory_name  Name of the opened memory object.
 * @return Status object.
 */
Status SharedMemoryManager::Open(
    uint64_t stream_id, int32_t size, std::string* memory_name) {
  if (size <= 0) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "size <= 0 (%" PRId32 ")", size);
  }
  if (memory_name == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "memory_name == NULL");
  }
  Status status;

  osal::OSLockMutex(mutex_);
  std::map<uint64_t, SharedMemoryParameter>::iterator itr =
      params_.find(stream_id);
  if (itr != params_.end()) {
    ++itr->second.ref_count;
    *memory_name = itr->second.memory_name;
  } else {
    // Create shared memory.
    std::ostringstream buffer;
    buffer << "wsconnection." << name_index_;
    ++name_index_;
    std::string object_name = buffer.str();

    SharedMemoryObject* object = CreateSharedMemoryObject();
    status = object->Open(object_name, size);
    if (status.ok()) {
      SharedMemoryParameter param = {};
      param.memory_object = object;
      param.memory_name = object_name;
      param.total_size = object->GetTotalSize();
      param.block_size = object->GetBlockSize();
      param.ref_count = 1;
      osal::OSCreateMutex(&param.mutex);
      params_[stream_id] = param;
      *memory_name = param.memory_name;
    } else {
      delete object;
    }
  }
  osal::OSUnlockMutex(mutex_);

  return status;
}

/**
 * @brief Closes a memory object.
 * @param[in]  stream_id  Stream ID for identification.
 * @return Status object.
 */
Status SharedMemoryManager::Close(uint64_t stream_id) {
  Status status;

  osal::OSLockMutex(mutex_);
  std::map<uint64_t, SharedMemoryParameter>::iterator itr =
      params_.find(stream_id);
  if (itr != params_.end()) {
    --itr->second.ref_count;
    if (itr->second.ref_count <= 0) {
      osal::OSLockMutex(itr->second.mutex);
      itr->second.memory_object->Close();
      delete itr->second.memory_object;
      osal::OSUnlockMutex(itr->second.mutex);
      osal::OSDestroyMutex(itr->second.mutex);
      params_.erase(itr);
    }
  } else {
    status = SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "Unmanaged stream id: %" PRIu64);
  }
  osal::OSUnlockMutex(mutex_);

  return status;
}

/**
 * @brief Set data on shared memory.
 * @param[in]  stream_id    Stream ID for identification.
 * @param[in]  input_list   List of input data.
 * @param[in]  input_count  Count of input data.
 * @param[out] output       Output data.
 * @return Status object.
 */
Status SharedMemoryManager::SetData(
    uint64_t stream_id, const InputData* input_list, size_t input_count,
    OutputData* output) {
  if (input_list == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "input_list == NULL");
  }
  if (output == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "output == NULL");
  }
  size_t input_size = 0;
  for (size_t i = 0; i < input_count; ++i) {
    if (input_list[i].buffer == NULL) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidArgument,
          "buffer_list[" PRIuS "] == NULL", i);
    }
    input_size += static_cast<int32_t>(input_list[i].size);
  }
  if (input_size == 0) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "input size == 0");
  }

  Status status;
  SharedMemoryParameter param = {};
  int32_t map_offset = 0;

  osal::OSLockMutex(mutex_);
  std::map<uint64_t, SharedMemoryParameter>::iterator itr =
      params_.find(stream_id);
  if (itr == params_.end()) {
    status = SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "Unmanaged stream id: %" PRIu64);
  }
  if (status.ok()) {
    // Calculate the next offset.
    int32_t block_size = itr->second.block_size;
    int32_t map_size = static_cast<int32_t>(input_size);
    map_size = ((map_size + block_size - 1) / block_size) * block_size;
    if (map_size > itr->second.total_size) {
      status = SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidArgument,
          "map size=%" PRId32 ", total_size=%" PRId32,
          map_size, itr->second.total_size);
    }
    if (status.ok()) {
      map_offset = itr->second.next_offset;
      if (map_offset + map_size > itr->second.total_size) {
        map_offset = 0;
      }
      itr->second.next_offset = map_offset + map_size;

      param = itr->second;
      osal::OSLockMutex(param.mutex);  // lock
    }
  }
  osal::OSUnlockMutex(mutex_);

  if (status.ok()) {
    // map & copy data.
    void* address = NULL;
    status = param.memory_object->Map(
        map_offset, static_cast<int32_t>(input_size), &address);
    if (status.ok()) {
      uint8_t* pointer = reinterpret_cast<uint8_t*>(address);
      size_t current_offset = 0;
      for (size_t i = 0; i < input_count; ++i) {
        osal::OSMemcpy(pointer + current_offset, input_size - current_offset,
                       input_list[i].buffer, input_list[i].size);
        current_offset += input_list[i].size;
      }
      param.memory_object->Unmap(address);

      output->offset = map_offset;
      output->size = input_size;
    }
    osal::OSUnlockMutex(param.mutex);  // unlock
  }

  return status;
}

/**
 * @brief Returns true if shared memory.
 * @return true if shared memory, false otherwise.
 */
bool SharedMemoryManager::IsSharedMemory(uint64_t stream_id) const {
  bool result = false;
  osal::OSLockMutex(mutex_);
  if (params_.find(stream_id) != params_.end()) {
    result = true;
  }
  osal::OSUnlockMutex(mutex_);
  return result;
}

/**
 * @brief Gets memory parameters.
 * @param[in]  stream_id  Stream ID for identification.
 * @param[out] param      Memory parameters.
 * @return Status object.
 */
Status SharedMemoryManager::GetMemoryParameter(
    uint64_t stream_id, SharedMemoryParameter* param) const {
  if (param == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "param == NULL");
  }
  Status status;
  osal::OSLockMutex(mutex_);
  std::map<uint64_t, SharedMemoryParameter>::const_iterator itr =
      params_.find(stream_id);
  if (itr != params_.end()) {
    *param = itr->second;
  } else {
    status = SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "Unmanaged stream id: %" PRIu64);
  }
  osal::OSUnlockMutex(mutex_);
  return status;
}

/**
 * @brief Keeps the memory size for each stream.
 * @param[in]  stream_key  Stream key.
 * @param[in]  size        Memory size.
 */
void SharedMemoryManager::SetSharedMemorySize(
    const std::string& stream_key, uint32_t size) {
  osal::OSLockMutex(mutex_);
  memory_size_list_[stream_key] = size;
  osal::OSUnlockMutex(mutex_);
}

/**
 * @brief Gets the memory size for each stream.
 * @param[in]  stream_key  Stream key.
 * @return Memory size. Returns zero if not registered.
 */
uint32_t SharedMemoryManager::GetSharedMemorySize(
    const std::string& stream_key) const {
  uint32_t size = 0;
  osal::OSLockMutex(mutex_);
  std::map<std::string, uint32_t>::const_iterator itr =
      memory_size_list_.find(stream_key);
  if (itr != memory_size_list_.end()) {
    size = itr->second;
  }
  osal::OSUnlockMutex(mutex_);
  return size;
}

}  // namespace ws
}  // namespace senscord

