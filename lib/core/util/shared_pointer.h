/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_UTIL_SHARED_POINTER_H_
#define LIB_CORE_UTIL_SHARED_POINTER_H_

#include <stdint.h>
#include <algorithm>
#include "senscord/osal.h"

namespace senscord {

/**
 * @brief A simple smart pointer.
 * It will be deleted automatically when the reference count reaches zero.
 *
 * Example:
 *   SharedPointer<X> ptr(new X);
 */
template<typename T>
class SharedPointer {
 public:
  /**
   * @brief Constructor.
   */
  explicit SharedPointer(T* pointer)
      : pointer_(pointer), count_(new SharedCount) {
  }

  /**
   * @brief Copy constructor.
   */
  SharedPointer(const SharedPointer<T>& rhs)
      : pointer_(), count_() {
    rhs.lock_.Lock();
    rhs.count_->AddCount();
    pointer_ = rhs.pointer_;
    count_ = rhs.count_;
    rhs.lock_.Unlock();
  }

  /**
   * @brief Assignment operator.
   */
  SharedPointer<T>& operator =(const SharedPointer<T>& rhs) {
    if (this == &rhs) {
      return *this;
    }
    Lock(rhs);
    rhs.count_->AddCount();
    if (count_->Release()) {
      delete pointer_;
      delete count_;
    }
    pointer_ = rhs.pointer_;
    count_ = rhs.count_;
    Unlock(rhs);
    return *this;
  }

  /**
   * @brief Destructor.
   */
  ~SharedPointer() {
    lock_.Lock();
    if (count_->Release()) {
      delete pointer_;
      pointer_ = NULL;
      delete count_;
      count_ = NULL;
    }
    lock_.Unlock();
  }

  /**
   * @brief Equal operator. (object == SharedPointer)
   */
  bool operator ==(const SharedPointer<T>& rhs) const {
    return (pointer_ == rhs.pointer_);
  }

  /**
   * @brief Not equal operator. (object != SharedPointer)
   */
  bool operator !=(const SharedPointer<T>& rhs) const {
    return (pointer_ != rhs.pointer_);
  }

  /**
   * @brief Equal operator. (object == pointer)
   */
  bool operator ==(const T* rhs) const {
    return (pointer_ == rhs);
  }

  /**
   * @brief Not equal operator. (object != pointer)
   */
  bool operator !=(const T* rhs) const {
    return (pointer_ != rhs);
  }

  /**
   * @brief Indirection operator. (*object)
   */
  T& operator *() {
    return *pointer_;
  }
  const T& operator *() const {
    return *pointer_;
  }

  /**
   * @brief Member access operator. (object->)
   */
  T* operator ->() {
    return pointer_;
  }
  const T* operator ->() const {
    return pointer_;
  }

 private:
  /**
   * @brief Multiple lock.
   * In order to prevent deadlock, the lock order is
   * determined by the pointer value.
   */
  void Lock(const SharedPointer<T>& rhs) const {
    if (&lock_ == &rhs.lock_) {
      lock_.Lock();
    } else {
      osal::OSExclusiveLock* first = std::min(&lock_, &rhs.lock_);
      osal::OSExclusiveLock* second = std::max(&lock_, &rhs.lock_);
      first->Lock();
      second->Lock();
    }
  }

  /**
   * @brief Multiple unlock.
   * Unlock in reverse order of Lock().
   */
  void Unlock(const SharedPointer<T>& rhs) const {
    if (&lock_ == &rhs.lock_) {
      lock_.Unlock();
    } else {
      osal::OSExclusiveLock* first = std::min(&lock_, &rhs.lock_);
      osal::OSExclusiveLock* second = std::max(&lock_, &rhs.lock_);
      second->Unlock();
      first->Unlock();
    }
  }

 private:
  /**
   * @brief Shared reference counter.
   */
  class SharedCount {
   public:
    /**
     * @brief Constructor.
     */
    SharedCount() : count_(1), lock_() {
    }

    /**
     * @brief Destructor.
     */
    ~SharedCount() {
    }

    /**
     * @brief Increment the reference count.
     */
    void AddCount() {
      lock_.Lock();
      ++count_;
      lock_.Unlock();
    }

    /**
     * @brief Decrement the reference count.
     * @return Returns true if the reference count is zero.
     */
    bool Release() {
      lock_.Lock();
      int32_t new_count = --count_;
      lock_.Unlock();
      return (new_count == 0);
    }

   private:
    SharedCount(const SharedCount&);  // = delete;
    SharedCount& operator =(const SharedCount&);  // = delete;

   private:
    int32_t count_;  ///< Reference count.
    osal::OSExclusiveLock lock_;  ///< Exclusive lock object.
  };

 private:
  T* pointer_;          ///< Contained pointer.
  SharedCount* count_;  ///< Reference counter.
  mutable osal::OSExclusiveLock lock_;  ///< Exclusive lock object for copying.
};

}  // namespace senscord

#endif  // LIB_CORE_UTIL_SHARED_POINTER_H_
