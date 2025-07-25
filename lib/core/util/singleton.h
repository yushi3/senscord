/*
 * SPDX-FileCopyrightText: 2022-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_UTIL_SINGLETON_H_
#define LIB_CORE_UTIL_SINGLETON_H_

#include "util/autolock.h"
#include "util/mutex.h"
#include "senscord/noncopyable.h"

namespace senscord {
namespace util {

typedef void (*FinalizerFunc)();

/**
 * @brief A class that manages singletons created with templates.
 */
class SingletonManager {
 public:
  /**
   * @brief Initialize the manager.
   */
  static void Init();

  /**
   * @brief Exit the manager.
   */
  static void Exit();

  /**
   * @brief Adds a finalizer function.
   * @param[in] (func) finalizer function
   * @param[in] (at_exit) If true, `func` is called at process exit.
   *     If false, `func` is called when all Core instances are released.
   */
  static void AddFinalizer(FinalizerFunc func, bool at_exit);

  /**
   * @brief Returns a mutex for singleton.
   */
  static Mutex* GetMutex();

 private:
  SingletonManager();
  ~SingletonManager();
};

/**
 * @brief Singleton template.
 */
template<typename T, bool AtExit = false>
class Singleton : private util::Noncopyable {
 public:
  /**
   * @brief Gets a singleton instance.
   */
  static T* GetInstance() {
    AutoLock lock(SingletonManager::GetMutex());
    if (instance_ == NULL) {
      instance_ = new T;
      SingletonManager::AddFinalizer(Destroy, AtExit);
    }
    return instance_;
  }

  /**
   * @brief Releases a singleton instance.
   */
  static void Destroy() {
    delete instance_;
    instance_ = NULL;
  }

 private:
  static T* instance_;
};

template<typename T, bool AtExit>
T* Singleton<T, AtExit>::instance_ = NULL;

}  // namespace util
}  // namespace senscord

#endif  // LIB_CORE_UTIL_SINGLETON_H_
