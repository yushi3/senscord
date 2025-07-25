/*
 * SPDX-FileCopyrightText: 2020-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_ALLOCATOR_SHARED_MEMORY_OBJECT_WINDOWS_H_
#define LIB_CORE_ALLOCATOR_SHARED_MEMORY_OBJECT_WINDOWS_H_

#include <Windows.h>
#include <Memoryapi.h>

#include <string>

#include "senscord/noncopyable.h"
#include "senscord/osal_inttypes.h"
#include "allocator/shared_memory_object.h"
#include "logger/logger.h"

// create macro
#define CreateSharedMemoryObject() \
  new senscord::SharedMemoryObjectWindows()

namespace senscord {

/**
 * @brief Shared memory object.
 */
class SharedMemoryObjectWindows : public SharedMemoryObject {
 public:
  /**
   * @brief Constructor.
   */
  SharedMemoryObjectWindows() : handle_(), total_size_() {}

  /**
   * @brief Destructor.
   */
  virtual ~SharedMemoryObjectWindows() {}

  /**
   * @brief Get the size of the unit block of memory allocation.
   * @return Size of block in bytes.
   */
  virtual int32_t GetBlockSize() const {
    SYSTEM_INFO info = {};
    GetSystemInfo(&info);
    return static_cast<int32_t>(info.dwAllocationGranularity);
  }

  /**
   * @brief Get the total size of shared memory.
   * @return Total size.
   */
  virtual int32_t GetTotalSize() const {
    return total_size_;
  }

  /**
   * @brief Opens or creates a memory object.
   * @param[in] (name) Name of memory object.
   * @param[in] (total_size) Total size of memory.
   * @return Status object.
   */
  virtual Status Open(const std::string& name, int32_t total_size) {
    handle_ = CreateFileMapping(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
        0, static_cast<DWORD>(total_size), name.c_str());
    if (handle_ == NULL) {
      handle_ = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
      if (handle_ == NULL) {
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation, "OpenFileMapping failed: %u",
            GetLastError());
      }
    }
    // check memory size.
    void* ptr = NULL;
    Status status = Map(0, 0, &ptr);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    MEMORY_BASIC_INFORMATION info = {};
    size_t ret = VirtualQuery(ptr, &info, sizeof(info));
    Unmap(ptr);
    if (ret == 0) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidOperation, "VirtualQuery failed: %u",
          GetLastError());
    }
    if (info.RegionSize > INT32_MAX) {
      info.RegionSize = static_cast<DWORD>(INT32_MAX) + 1 - GetBlockSize();
    }
    total_size_ = static_cast<int32_t>(info.RegionSize);
    if (total_size != total_size_) {
      SENSCORD_LOG_WARNING(
          "[Shared memory] Size mismatch: input=%" PRId32 ", output=%" PRId32,
          total_size, total_size_);
    }
    return Status::OK();
  }

  /**
   * @brief Closes the memory object.
   * @return Status object.
   */
  virtual Status Close() {
    if (handle_ != NULL) {
      CloseHandle(handle_);
      handle_ = NULL;
    }
    return Status::OK();
  }

  /**
   * @brief Map to memory.
   * @param[in] (offset) Starting offset for the mapping.
   * @param[in] (size) Size to map.
   * @param[out] (address) Mapped virtual address.
   * @return Status object.
   */
  virtual Status Map(int32_t offset, int32_t size, void** address) {
    *address = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0,
        static_cast<DWORD>(offset), static_cast<DWORD>(size));
    if (*address == NULL) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidOperation,
          "[Shared memory] MapViewOfFile failed: %u", GetLastError());
    }
    return Status::OK();
  }

  /**
   * @brief Unmap memory.
   * @param[in] (address) Mapped virtual address.
   * @return Status object.
   */
  virtual Status Unmap(void* address) {
    if (!UnmapViewOfFile(address)) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidOperation,
          "[Shared memory] UnmapViewOfFile failed: %u", GetLastError());
    }
    return Status::OK();
  }

 private:
  HANDLE handle_;
  int32_t total_size_;
};

}  // namespace senscord

#endif  // LIB_CORE_ALLOCATOR_SHARED_MEMORY_OBJECT_WINDOWS_H_
