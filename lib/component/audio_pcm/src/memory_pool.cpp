/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/memory_pool.h"

#include <inttypes.h>

#include "senscord/logger.h"

namespace {

const char kBlockName[] = "memory";

}  // namespace

/**
 * @brief Constructor.
 * @param[in] (allocator) Memory allocator.
 */
MemoryPool::MemoryPool(senscord::MemoryAllocator* allocator)
    : allocator_(allocator), memory_list_(), memory_queue_(), mutex_(),
      buffer_size_() {
  senscord::osal::OSCreateMutex(&mutex_);
}

/**
 * @brief Destructor.
 */
MemoryPool::~MemoryPool() {
  Exit();
  senscord::osal::OSDestroyMutex(mutex_);
}

/**
 * @brief Initialize the memory pool.
 * @param[in] (buffer_num) Number of buffers. (If 0, allocate each time)
 * @param[in] (buffer_size) Size of buffer.
 * @return Status object.
 */
senscord::Status MemoryPool::Init(uint32_t buffer_num, uint32_t buffer_size) {
  senscord::Status status;
  SENSCORD_LOG_INFO_TAGGED(
      kBlockName, "Init: num=%" PRIu32 ", size=%" PRIu32,
      buffer_num, buffer_size);

  senscord::osal::OSLockMutex(mutex_);
  if (IsInitialized()) {
    status = SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidOperation,
        "Already initialized");
  }
  if (status.ok() && (buffer_num > 0)) {
    for (uint32_t i = 0; i < buffer_num; ++i) {
      senscord::Memory* memory = NULL;
      status = allocator_->Allocate(buffer_size, &memory);
      if (!status.ok()) {
        SENSCORD_STATUS_TRACE(status);
        break;
      }
      memory_list_.push_back(memory);
      memory_queue_.push_back(memory);
      SENSCORD_LOG_DEBUG_TAGGED(kBlockName, "Allocate: %p", memory);
    }
  }
  if (status.ok()) {
    buffer_size_ = buffer_size;
  }
  senscord::osal::OSUnlockMutex(mutex_);

  if (!status.ok()) {
    Exit();
  }

  return status;
}

/**
 * @brief Terminate the memory pool.
 */
void MemoryPool::Exit() {
  senscord::osal::OSLockMutex(mutex_);
  while (!memory_list_.empty()) {
    senscord::Memory* memory = memory_list_.back();
    memory_list_.pop_back();
    SENSCORD_LOG_DEBUG_TAGGED(kBlockName, "Free: %p", memory);
    allocator_->Free(memory);
  }
  memory_queue_.clear();
  buffer_size_ = 0;
  senscord::osal::OSUnlockMutex(mutex_);
}

/**
 * @brief Gets memory.
 * @return Memory object or NULL.
 */
senscord::Memory* MemoryPool::GetMemory() {
  senscord::Memory* memory = NULL;
  senscord::osal::OSLockMutex(mutex_);
  if (IsInitialized()) {
    if (!memory_list_.empty()) {
      if (!memory_queue_.empty()) {
        memory = memory_queue_.front();
        memory_queue_.pop_front();
      }
    } else {
      allocator_->Allocate(buffer_size_, &memory);
      SENSCORD_LOG_DEBUG_TAGGED(kBlockName, "Allocate: %p", memory);
    }
  }
  senscord::osal::OSUnlockMutex(mutex_);
  return memory;
}

/**
 * @brief Releases memory.
 */
void MemoryPool::ReleaseMemory(senscord::Memory* memory) {
  if (memory == NULL) {
    return;
  }
  senscord::osal::OSLockMutex(mutex_);
  if (IsInitialized()) {
    if (!memory_list_.empty()) {
      memory_queue_.push_back(memory);
    } else {
      SENSCORD_LOG_DEBUG_TAGGED(kBlockName, "Free: %p", memory);
      allocator_->Free(memory);
    }
  }
  senscord::osal::OSUnlockMutex(mutex_);
}

/**
 * @brief Returns number of buffers.
 */
uint32_t MemoryPool::GetBufferNum() const {
  return static_cast<uint32_t>(memory_list_.size());
}

/**
 * @brief Returns size of buffer.
 */
uint32_t MemoryPool::GetBufferSize() const {
  return buffer_size_;
}

/**
 * @brief Returns true if initialized.
 */
bool MemoryPool::IsInitialized() const {
  return (buffer_size_ != 0);
}
