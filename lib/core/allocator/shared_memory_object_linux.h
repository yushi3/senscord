/*
 * SPDX-FileCopyrightText: 2020-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_ALLOCATOR_SHARED_MEMORY_OBJECT_LINUX_H_
#define LIB_CORE_ALLOCATOR_SHARED_MEMORY_OBJECT_LINUX_H_

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>

#include <string>
#include <map>
#include <utility>

#include "senscord/noncopyable.h"
#include "senscord/osal_inttypes.h"
#include "allocator/shared_memory_object.h"
#include "logger/logger.h"

// create macro
#define CreateSharedMemoryObject() \
  new senscord::SharedMemoryObjectLinux()

namespace senscord {

/**
 * @brief RAII style file lock class.
 */
class FileLock : private util::Noncopyable {
 public:
  /**
   * @brief Lock
   */
  FileLock(int32_t fd, int32_t pos, int32_t len) :
      fd_(-1), pos_(pos), len_(len) {
    struct flock fl = {};
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = pos;
    fl.l_len = len;
    if (fcntl(fd, F_SETLKW, &fl) == 0) {
      fd_ = fd;
    }
  }

  /**
   * @brief Unlock
   */
  ~FileLock() {
    if (fd_ != -1) {
      struct flock fl = {};
      fl.l_type = F_UNLCK;
      fl.l_whence = SEEK_SET;
      fl.l_start = pos_;
      fl.l_len = len_;
      fcntl(fd_, F_SETLKW, &fl);
      fd_ = -1;
    }
  }

 private:
  int32_t fd_;
  const int32_t pos_;
  const int32_t len_;
};

/**
 * @brief Shared memory object.
 */
class SharedMemoryObjectLinux : public SharedMemoryObject {
 public:
  /**
   * @brief Constructor.
   */
  SharedMemoryObjectLinux() : fd_(-1), total_size_() {}

  /**
   * @brief Destructor.
   */
  virtual ~SharedMemoryObjectLinux() {}

  /**
   * @brief Get the size of the unit block of memory allocation.
   * @return Size of block in bytes.
   */
  virtual int32_t GetBlockSize() const {
    uint64_t size = sysconf(_SC_PAGESIZE);
    return static_cast<int32_t>(size);
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
    name_ = "/" + name;

    // Open or create
    fd_ = shm_open(
        name_.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_ == -1) {
      fd_ = shm_open(name_.c_str(), O_RDWR, 0);
      if (fd_ == -1) {
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation, "shm_open failed: %s",
            strerror(errno));
      }
    }

    // Read lock until Close()
    {
      struct flock fl = {};
      fl.l_type = F_RDLCK;
      fl.l_whence = SEEK_SET;
      fl.l_start = 0;
      fl.l_len = 1;
      if (fcntl(fd_, F_SETLKW, &fl) != 0) {
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation, "fcntl(read lock) failed: %s",
            strerror(errno));
      }
    }

    {
      FileLock lock(fd_, 1, 0);

      struct stat st = {};
      if (fstat(fd_, &st) != 0) {
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation, "fstat failed: %s",
            strerror(errno));
      }

      if (st.st_size == 0) {
        if (ftruncate(fd_, total_size) != 0) {
          return SENSCORD_STATUS_FAIL(kStatusBlockCore,
              Status::kCauseInvalidOperation, "ftruncate failed: %s",
              strerror(errno));
        }
        fstat(fd_, &st);
        if (st.st_size != total_size) {
          return SENSCORD_STATUS_FAIL(kStatusBlockCore,
              Status::kCauseInvalidOperation,
              "ftruncate failed: input=%" PRId32 ", output=%" PRIdS,
              total_size, st.st_size);
        }
      } else if (st.st_size != total_size) {
        SENSCORD_LOG_WARNING(
            "[Shared memory] Size mismatch: input=%" PRId32 ", output=%" PRIdS,
            total_size, st.st_size);
      }
      total_size_ = static_cast<int32_t>(st.st_size);
    }

    return Status::OK();
  }

  /**
   * @brief Closes the memory object.
   * @return Status object.
   */
  virtual Status Close() {
    if (!map_list_.empty()) {
      std::map<void*, int32_t> list = map_list_;
      for (std::map<void*, int32_t>::const_iterator itr = list.begin(),
          end = list.end(); itr != end; ++itr) {
        Unmap(itr->first);
      }
    }
    if (fd_ != -1) {
      // Try to change from read lock to write lock.
      struct flock fl = {};
      fl.l_type = F_WRLCK;
      fl.l_whence = SEEK_SET;
      fl.l_start = 0;
      fl.l_len = 1;
      if (fcntl(fd_, F_SETLK, &fl) == 0) {
        if (shm_unlink(name_.c_str()) != 0) {
          SENSCORD_LOG_WARNING("[Shared memory] shm_unlink failed: %s",
                               strerror(errno));
        }
      }
      close(fd_);
      fd_ = -1;
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
    *address = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    fd_, offset);
    if (*address ==  MAP_FAILED) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidOperation,
          "[Shared memory] mmap failed: %s", strerror(errno));
    }
    map_list_.insert(std::make_pair(*address, size));
    return Status::OK();
  }

  /**
   * @brief Unmap memory.
   * @param[in] (address) Mapped virtual address.
   * @return Status object.
   */
  virtual Status Unmap(void* address) {
    std::map<void*, int32_t>::iterator pos = map_list_.find(address);
    if (pos == map_list_.end()) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidArgument,
          "[Shared memory] Unmanaged address: %p", address);
    }
    if (munmap(address, pos->second) != 0) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseInvalidOperation,
          "[Shared memory] munmap failed: %s", strerror(errno));
    }
    map_list_.erase(pos);
    return Status::OK();
  }

 private:
  int32_t fd_;
  int32_t total_size_;
  std::string name_;
  std::map<void*, int32_t> map_list_;
};

}  // namespace senscord

#endif  // LIB_CORE_ALLOCATOR_SHARED_MEMORY_OBJECT_LINUX_H_
