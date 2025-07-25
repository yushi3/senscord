/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <inttypes.h>

#include "senscord/osal.h"
#include "common/osal_logger.h"

namespace senscord {
namespace osal {

/**
 * @brief Constructor.
 */
OSExclusiveLock::OSExclusiveLock() : lock_object_(NULL) {
  lock_object_ = new pthread_mutex_t;
  pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(lock_object_);
  int result = pthread_mutex_init(mutex, NULL);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex init failed(0x%" PRIx32 ")", result);
  }
}

/**
 * @brief Destrouctor.
 */
OSExclusiveLock::~OSExclusiveLock() {
  pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(lock_object_);
  int result = pthread_mutex_destroy(mutex);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex delete failed(0x%" PRIx32 ")", result);
  }
  delete mutex;
}

/**
 * @brief Exclusive lock.
 */
void OSExclusiveLock::Lock() {
  pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(lock_object_);
  int result = pthread_mutex_lock(mutex);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex lock failed(0x%" PRIx32 ")", result);
  }
}

/**
 * @brief Exclusive unlock.
 */
void OSExclusiveLock::Unlock() {
  pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(lock_object_);
  int result = pthread_mutex_unlock(mutex);
  if (result != 0) {
    SENSCORD_OSAL_LOG_WARNING("mutex unlock failed(0x%" PRIx32 ")", result);
  }
}

}  //  namespace osal
}  //  namespace senscord
