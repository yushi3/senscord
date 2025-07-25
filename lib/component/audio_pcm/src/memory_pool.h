/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef COMPONENT_AUDIO_SRC_MEMORY_POOL_H_
#define COMPONENT_AUDIO_SRC_MEMORY_POOL_H_

#include <stdint.h>

#include <vector>
#include <list>

#include "senscord/memory_allocator.h"
#include "senscord/osal.h"

/**
 * @brief Memory pool
 */
class MemoryPool {
 public:
  /**
   * @brief Constructor.
   * @param[in] (allocator) Memory allocator.
   */
  explicit MemoryPool(senscord::MemoryAllocator* allocator);

  /**
   * @brief Destructor.
   */
  ~MemoryPool();

  /**
   * @brief Initialize the memory pool.
   * @param[in] (buffer_num) Number of buffers. (If 0, allocate each time)
   * @param[in] (buffer_size) Size of buffer.
   * @return Status object.
   */
  senscord::Status Init(uint32_t buffer_num, uint32_t buffer_size);

  /**
   * @brief Terminate the memory pool.
   */
  void Exit();

  /**
   * @brief Gets memory.
   * @return Memory object or NULL.
   */
  senscord::Memory* GetMemory();

  /**
   * @brief Releases memory.
   */
  void ReleaseMemory(senscord::Memory* memory);

  /**
   * @brief Returns number of buffers.
   */
  uint32_t GetBufferNum() const;

  /**
   * @brief Returns size of buffer.
   */
  uint32_t GetBufferSize() const;

 private:
  /**
   * @brief Returns true if initialized.
   */
  bool IsInitialized() const;

 private:
  senscord::MemoryAllocator* allocator_;
  std::vector<senscord::Memory*> memory_list_;
  std::list<senscord::Memory*> memory_queue_;
  senscord::osal::OSMutex* mutex_;
  uint32_t buffer_size_;
};

#endif  // COMPONENT_AUDIO_SRC_MEMORY_POOL_H_
