/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_ALLOCATOR_ALLOCATOR_SAMPLE_SRC_MEMORY_ALLOCATOR_SAMPLE_H_
#define LIB_ALLOCATOR_ALLOCATOR_SAMPLE_SRC_MEMORY_ALLOCATOR_SAMPLE_H_

#include <stdint.h>
#include <vector>
#include "senscord/develop/memory_allocator_core.h"
#include "senscord/memory.h"

namespace senscord {

/**
 * @brief Memory allocator sample.
 */
class MemoryAllocatorSample : public MemoryAllocatorCore {
 public:
  /**
   * @brief Initialization.
   * @param[in] (config) Allocator config.
   * @return Status object.
   */
  virtual Status Init(const AllocatorConfig& config);

  /**
   * @brief Allocate memory block.
   * @param[in]  (size) Size to allocate.
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
   * @return True means sharing between other process, false means local.
   */
  virtual bool IsMemoryShared() const;

  /**
   * @brief Constructor.
   */
  MemoryAllocatorSample() {}

  /**
   * @brief Destructor.
   */
  ~MemoryAllocatorSample() {}
};

}  // namespace senscord
#endif  // LIB_ALLOCATOR_ALLOCATOR_SAMPLE_SRC_MEMORY_ALLOCATOR_SAMPLE_H_
