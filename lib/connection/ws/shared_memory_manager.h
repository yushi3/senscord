/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CONNECTION_WS_SHARED_MEMORY_MANAGER_H_
#define LIB_CONNECTION_WS_SHARED_MEMORY_MANAGER_H_

#include <stdint.h>
#include <map>
#include <string>

#include "senscord/osal.h"
#include "senscord/noncopyable.h"
#include "senscord/status.h"

#include "./shared_memory_object.h"

namespace senscord {
namespace ws {

/**
 * @brief Input data for SetData().
 */
struct InputData {
  const void* buffer;
  size_t size;
};

/**
 * @brief Output data for SetData().
 */
struct OutputData {
  size_t offset;
  size_t size;
};

/**
 * @brief Shared memory parameters for each stream.
 */
struct SharedMemoryParameter {
  std::string memory_name;
  SharedMemoryObject* memory_object;
  osal::OSMutex* mutex;
  int32_t total_size;
  int32_t block_size;
  int32_t next_offset;
  int32_t ref_count;
};

/**
 * @brief Class that manages the shared memory.
 */
class SharedMemoryManager : private util::Noncopyable {
 public:
  /**
   * @brief Get manager instance.
   * @return Manager instance.
   */
  static SharedMemoryManager* GetInstance();

  /**
   * @brief Opens a memory object.
   * @param[in]  stream_id    Stream ID for identification.
   * @param[in]  size         Size of memory object.
   * @param[out] memory_name  Name of the opened memory object.
   * @return Status object.
   */
  Status Open(uint64_t stream_id, int32_t size, std::string* memory_name);

  /**
   * @brief Closes a memory object.
   * @param[in]  stream_id  Stream ID for identification.
   * @return Status object.
   */
  Status Close(uint64_t stream_id);

  /**
   * @brief Set data on shared memory.
   * @param[in]  stream_id    Stream ID for identification.
   * @param[in]  input_list   List of input data.
   * @param[in]  input_count  Count of input data.
   * @param[out] output       Output data.
   * @return Status object.
   */
  Status SetData(
      uint64_t stream_id, const InputData* input_list, size_t input_count,
      OutputData* output);

  /**
   * @brief Returns true if shared memory.
   * @return true if shared memory, false otherwise.
   */
  bool IsSharedMemory(uint64_t stream_id) const;

  /**
   * @brief Gets memory parameters.
   * @param[in]  stream_id  Stream ID for identification.
   * @param[out] param      Memory parameters.
   * @return Status object.
   */
  Status GetMemoryParameter(
      uint64_t stream_id, SharedMemoryParameter* param) const;

  /**
   * @brief Keeps the memory size for each stream.
   * @param[in]  stream_key  Stream key.
   * @param[in]  size        Memory size.
   */
  void SetSharedMemorySize(const std::string& stream_key, uint32_t size);

  /**
   * @brief Gets the memory size for each stream.
   * @param[in]  stream_key  Stream key.
   * @return Memory size. Returns zero if not registered.
   */
  uint32_t GetSharedMemorySize(const std::string& stream_key) const;

 private:
  /**
   * @brief Constructor.
   */
  SharedMemoryManager();

  /**
   * @brief Destructor.
   */
  ~SharedMemoryManager();

 private:
  // Mutex for parameters.
  osal::OSMutex* mutex_;

  // Index value for memory object name.
  uint32_t name_index_;
  // Parameter list (Key=stream id, Value=parameter).
  std::map<uint64_t, SharedMemoryParameter> params_;

  // Memory size list (Key=stream key, Value=memory size).
  std::map<std::string, uint32_t> memory_size_list_;
};

}  // namespace ws
}  // namespace senscord

#endif  // LIB_CONNECTION_WS_SHARED_MEMORY_MANAGER_H_

