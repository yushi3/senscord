/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "allocator/memory_manager.h"

#include <stdlib.h>

#include <utility>

#include "senscord/configuration.h"
#include "allocator/memory_allocator_heap.h"
#include "util/mutex.h"
#include "util/autolock.h"
#include "util/senscord_utils.h"
#include "util/singleton.h"
#include "core/internal_types.h"
#include "configuration/core_config.h"
#include "senscord/osal.h"
#include "logger/logger.h"

#ifdef SENSCORD_ALLOCATOR_SHARED_MEMORY
#include "allocator/shared_memory_allocator.h"
#endif  // SENSCORD_ALLOCATOR_SHARED_MEMORY

namespace {

/**
 * @brief Create heap allocator.
 * @param[in] (config) allocator config.
 * @param[out] (allocator) created allocator.
 * @return Status object.
 */
senscord::Status CreateHeapMemoryAllocator(
    const senscord::AllocatorConfig& config,
    senscord::MemoryAllocatorCore** allocator) {
  senscord::MemoryAllocatorHeap* tmp = new senscord::MemoryAllocatorHeap();
  senscord::Status status = tmp->Init(config);
  if (!status.ok()) {
    delete tmp;
    return SENSCORD_STATUS_TRACE(status);
  }
  *allocator = tmp;
  return status;
}

#ifdef SENSCORD_ALLOCATOR_SHARED_MEMORY
/**
 * @brief Create shared memory allocator.
 * @param[in] (config) allocator config.
 * @param[out] (allocator) created allocator.
 * @return Status object.
 */
senscord::Status CreateSharedMemoryAllocator(
    const senscord::AllocatorConfig& config,
    senscord::MemoryAllocatorCore** allocator) {
  senscord::SharedMemoryAllocator* tmp = new senscord::SharedMemoryAllocator();
  senscord::Status status = tmp->Init(config);
  if (!status.ok()) {
    delete tmp;
    return SENSCORD_STATUS_TRACE(status);
  }
  *allocator = tmp;
  return status;
}
#endif  // SENSCORD_ALLOCATOR_SHARED_MEMORY

}  // namespace

namespace senscord {

/**
 * @brief Get MemoryManager instance.
 * @return MemoryManager instance.
 */
MemoryManager* MemoryManager::GetInstance() {
  // for private constructor / destructor
  struct InnerMemoryManager : public MemoryManager {
  };
  return util::Singleton<InnerMemoryManager>::GetInstance();
}

/**
 * @brief Constructor.
 */
MemoryManager::MemoryManager() {
}

/**
 * @brief Destructor.
 */
MemoryManager::~MemoryManager() {
  {
    util::AutoLock lock(&mutex_);
    DeleteMemoryAllocators();
  }
}

/**
 * @brief Initialize.
 * @param[in] (config_list) allocator config list.
 * @return Status object.
 */
Status MemoryManager::Init(const std::vector<AllocatorConfig>& config_list) {
  util::AutoLock lock(&mutex_);

  Status status;
  if (allocator_map_.empty()) {
    status = AddDefaultMemoryAllocator(config_list);
    SENSCORD_STATUS_TRACE(status);
  }

  if (status.ok()) {
    status = AddMemoryAllocators(config_list);
    SENSCORD_STATUS_TRACE(status);
  }

  return status;
}

/**
 * @brief Add allocator to manage map.
 * @param[in] (config_list) allocator config list.
 * @return Status object.
 */
Status MemoryManager::AddMemoryAllocators(
    const std::vector<AllocatorConfig>& config_list) {
  for (std::vector<AllocatorConfig>::const_iterator
      itr = config_list.begin(), end = config_list.end(); itr != end; ++itr) {
    const AllocatorConfig& config = (*itr);
    if (config.key == senscord::kDefaultAllocatorKey) {
      continue;  // skip if default allocator.
    }
    Status status = AddMemoryAllocator(config);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }
  return Status::OK();
}

/**
 * @brief Add allocator.
 * @param[in] (config_list) allocator config.
 * @return Status object.
 */
Status MemoryManager::AddMemoryAllocator(const AllocatorConfig& config) {
  Status status;
  std::pair<AllocatorMap::iterator, bool> ret =
      allocator_map_.insert(std::make_pair(config.key, AllocatorInstance()));
  if (ret.second) {
    MemoryAllocatorCore* allocator = NULL;
    if (config.type == kAllocatorTypeHeap) {
      status = CreateHeapMemoryAllocator(config, &allocator);
      SENSCORD_STATUS_TRACE(status);
    }
#ifdef SENSCORD_ALLOCATOR_SHARED_MEMORY
    if (allocator == NULL && config.type == kAllocatorTypeSharedMemory) {
      status = CreateSharedMemoryAllocator(config, &allocator);
      SENSCORD_STATUS_TRACE(status);
    }
#endif  // SENSCORD_ALLOCATOR_SHARED_MEMORY
    if (allocator == NULL && status.ok()) {
      status = CreateUserMemoryAllocator(config, &allocator);
      SENSCORD_STATUS_TRACE(status);
    }
    if (status.ok()) {
      AllocatorInstance& instance = ret.first->second;
      instance.allocator = allocator;
      instance.type = config.type;
#ifdef SENSCORD_LOG_ENABLED
      // print config
      SENSCORD_LOG_DEBUG("Allocator: key=%s, type=%s, cacheable=%s",
                         config.key.c_str(), config.type.c_str(),
                         config.cacheable ? "on" : "off");
      for (std::map<std::string, std::string>::const_iterator
          itr = config.arguments.begin(), end = config.arguments.end();
          itr != end; ++itr) {
        SENSCORD_LOG_DEBUG("    - argument : name=%s, value=%s",
                           itr->first.c_str(), itr->second.c_str());
      }
#endif  // SENSCORD_LOG_ENABLED
    }
  }
  if (!status.ok()) {
    SENSCORD_LOG_ERROR("add allocator failed (key:%s, type:%s): status=%s",
        config.key.c_str(), config.type.c_str(), status.ToString().c_str());
    allocator_map_.erase(ret.first);
  }
  return status;
}

/**
 * @brief Get MemoryAllocator instance.
 * @param[in]  (key) Search by allocator key with allocator.
 * @param[out] (allocator) Return MemoryAllocator instance
 * @return Status object.
 */
Status MemoryManager::GetAllocator(const std::string& key,
                                   MemoryAllocator** allocator) {
  util::AutoLock lock(&mutex_);
  AllocatorMap::iterator itr = allocator_map_.find(key);
  if (itr == allocator_map_.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "not found allocator: key=%s", key.c_str());
  }
  *allocator = itr->second.allocator;
  return Status::OK();
}

/**
 * @brief Add default allocator.
 * @param[in] (config_list) allocator config list.
 * @return Status object.
 */
Status MemoryManager::AddDefaultMemoryAllocator(
    const std::vector<AllocatorConfig>& config_list) {
  AllocatorConfig config;
  const AllocatorConfig* tmp_config =
      GetAllocatorConfig(&config_list, senscord::kDefaultAllocatorKey);
  if (tmp_config != NULL) {
    config = *tmp_config;
  } else {
    config.type = kAllocatorTypeHeap;
    config.cacheable = false;
  }
  config.key = kAllocatorDefaultKey;
  Status status = AddMemoryAllocator(config);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Delete all allocators.
 */
void MemoryManager::DeleteMemoryAllocators() {
  while (!allocator_map_.empty()) {
    AllocatorMap::iterator itr = allocator_map_.begin();
    DeleteMemoryAllocator(itr->second.type, itr->second.allocator);
    allocator_map_.erase(itr);
  }
}

/**
 * @brief Delete allocator.
 * @param[in] (type) allocator type.
 * @param[in] (allocator) allocator to delete.
 */
void MemoryManager::DeleteMemoryAllocator(
    const std::string& type,
    MemoryAllocatorCore* allocator) {
  Status status = allocator->Exit();
  if (!status.ok()) {
    SENSCORD_LOG_ERROR("allocator deinit failed : ret=%s",
                       status.ToString().c_str());
  }
  if (type == kAllocatorTypeHeap) {
    delete allocator;
    allocator = NULL;
  }
#ifdef SENSCORD_ALLOCATOR_SHARED_MEMORY
  if (allocator != NULL && type == kAllocatorTypeSharedMemory) {
    delete allocator;
    allocator = NULL;
  }
#endif  // SENSCORD_ALLOCATOR_SHARED_MEMORY
  if (allocator != NULL) {
    DeleteUserMemoryAllocator(type, allocator);
  }
}

/**
 * @brief Create user defined allocator.
 * @param[in] (config) allocator config.
 * @param[out] (allocator) created allocator.
 * @return Status object.
 */
Status MemoryManager::CreateUserMemoryAllocator(
    const AllocatorConfig& config,
    MemoryAllocatorCore** allocator) {
  Status status;
  MemoryAllocatorCore* tmp = NULL;
  status = loader_.Create(config.type, &tmp);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  status = tmp->Init(config);
  if (!status.ok()) {
    DeleteUserMemoryAllocator(config.type, tmp);
    return SENSCORD_STATUS_TRACE(status);
  }
  *allocator = tmp;
  return status;
}

/**
 * @brief Delete user defined allocator.
 * @param[in] (type) allocator type.
 * @param[in] (allocator) allocator to delete.
 * @return Status object.
 */
void MemoryManager::DeleteUserMemoryAllocator(
    const std::string& type,
    MemoryAllocatorCore* allocator) {
  Status status = loader_.Destroy(type, allocator);
  if (!status.ok()) {
    SENSCORD_LOG_ERROR("failed to destroy allocator : ret=%s",
                       status.ToString().c_str());
  }
}

}   // namespace senscord
