/*
 * SPDX-FileCopyrightText: 2020-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_ALLOCATOR_SHARED_MEMORY_ALLOCATOR_H_
#define LIB_CORE_ALLOCATOR_SHARED_MEMORY_ALLOCATOR_H_

#include <vector>
#include <map>
#include <set>
#include <string>
#include "senscord/develop/memory_allocator_core.h"
#include "senscord/memory.h"
#include "allocator/shared_memory.h"
#include "allocator/shared_memory_object.h"
#include "allocator/shared_allocation_method.h"
#include "util/mutex.h"

namespace senscord {

/**
 * @brief Shared memory allocator.
 */
class SharedMemoryAllocator : public MemoryAllocatorCore {
 public:
  /**
   * @brief Constructor.
   */
  SharedMemoryAllocator();

  /**
   * @brief Destructor.
   */
  ~SharedMemoryAllocator();

  /**
   * @brief Initialization.
   * @param[in] (config) Allocator config.
   * @return Status object.
   */
  virtual Status Init(const AllocatorConfig& config);

  /**
   * @brief Exiting.
   * @return Status object.
   */
  virtual Status Exit();

  /**
   * @brief Allocate memory block.
   * @param[in] (size) Size to allocate.
   * @param[out] (memory) Allocated Memory.
   * @return Status object.
   */
  virtual Status Allocate(size_t size, Memory** memory);

  /**
   * @brief Free memory block.
   * @param[in] (memory) Memory to free.
   * @return Status object.
   */
  virtual Status Free(Memory* memory);

#ifdef SENSCORD_SERVER
  /**
   * @brief Serialize the raw data memory area.
   * @param[in] (rawdata_memory) Memory information for raw data.
   * @param[out] (serialized) Serialized memory information.
   * @return Status object.
   */
  virtual Status ServerSerialize(
      const RawDataMemory& rawdata_memory,
      std::vector<uint8_t>* serialized) const;

  /**
   * @brief Initialize the mapping area.
   * @return Status object.
   */
  virtual Status ClientInitMapping();

  /**
   * @brief Deinitialize the mapping area.
   * @return Status object.
   */
  virtual Status ClientExitMapping();

  /**
   * @brief Mapping memory with serialized memory information.
   * @param[in] (serialized) Serialized memory information.
   * @param[out] (rawdata_memory) Memory information for raw data.
   * @return Status object.
   */
  virtual Status ClientMapping(
      const std::vector<uint8_t>& serialized,
      RawDataMemory* rawdata_memory);

  /**
   * @brief Release the mapped area.
   * @param[in] (rawdata_memory) Memory information for raw data.
   * @return Status object.
   */
  virtual Status ClientUnmapping(const RawDataMemory& rawdata_memory);
#endif  // SENSCORD_SERVER

  /**
   * @brief Whether the memory is shared.
   * @return Always returns true.
   */
  virtual bool IsMemoryShared() const;

  /**
   * @brief Is cacheable allocator.
   * @return Always returns false.
   */
  virtual bool IsCacheable() const;

 private:
  /**
   * @brief Parse the arguments.
   * @return Status object.
   */
  Status ParseArguments();

  /**
   * @brief Get the argument value.
   * @param[in] (argument_name) The argument name.
   * @param[out] (value) The argument value.
   * @return Status object.
   */
  Status GetArgument(const std::string& argument_name, std::string* value);
  Status GetArgument(const std::string& argument_name, int64_t* value);

  /**
   * @brief Check for duplicate name.
   * @param[in] (name) The memory object name.
   * @return False if the name is duplicated.
   */
  bool CheckDuplicateName(const std::string& name);

  /**
   * @brief Free all memory.
   */
  void FreeAll();

 private:
  struct MappingInfo {
    OffsetParam offset;
    bool allocation;
  };

 private:
  int32_t total_size_;
  int32_t block_size_;
  SharedMemoryObject* object_;
  AllocationMethod* method_;
  std::string memory_name_;
  std::map<std::string, std::string> arguments_;
  std::map<SharedMemory*, MappingInfo> memory_list_;
  mutable util::Mutex memory_list_mutex_;

  static std::set<std::string> memory_names_;
};

}  // namespace senscord

#endif  // LIB_CORE_ALLOCATOR_SHARED_MEMORY_ALLOCATOR_H_
