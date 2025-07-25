/*
 * SPDX-FileCopyrightText: 2022-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util/singleton.h"

#include <stdlib.h>

#include <vector>

namespace {

/**
 * @brief A container that stores finalizer functions.
 */
class Finalizer {
 public:
  Finalizer() : finalizers_() {}

  ~Finalizer() {
    for (std::vector<senscord::util::FinalizerFunc>::const_reverse_iterator
        itr = finalizers_.rbegin(), end = finalizers_.rend();
        itr != end; ++itr) {
      (*itr)();
    }
    finalizers_.clear();
  }

  void Add(senscord::util::FinalizerFunc func) {
    finalizers_.push_back(func);
  }

 private:
  std::vector<senscord::util::FinalizerFunc> finalizers_;
};

// forward declaration
void AddReference();
void RemoveReference();

// static variables
senscord::util::Mutex* g_mutex = new senscord::util::Mutex;
int32_t g_reference_count = 0;
Finalizer* g_finalizer = NULL;

/**
 * @brief Create Finalizer instance.
 */
Finalizer* CreateFinalizer() {
  AddReference();
  atexit(RemoveReference);
  return new Finalizer;
}

Finalizer* g_finalizer_at_exit = CreateFinalizer();

/**
 * @brief Increase the reference count.
 */
void AddReference() {
  senscord::util::AutoLock lock(g_mutex);
  if ((g_finalizer == NULL) && (g_reference_count == 1)) {
    g_finalizer = new Finalizer;
  }
  ++g_reference_count;
}

/**
 * @brief Decrease the reference count.
 *
 * Executes the finalizer by reference count value.
 */
void RemoveReference() {
  senscord::util::AutoLock lock(g_mutex);
  if (g_reference_count > 0) {
    --g_reference_count;
  }
  // When all core instances are released.
  if (g_reference_count == 1) {
    delete g_finalizer;
    g_finalizer = NULL;
  }
  // When the process is terminated.
  if (g_reference_count == 0) {
    delete g_finalizer_at_exit;
    g_finalizer_at_exit = NULL;
  }
}

}  // namespace

namespace senscord {
namespace util {

/**
 * @brief Initialize the manager.
 */
void SingletonManager::Init() {
  AddReference();
}

/**
 * @brief Exit the manager.
 */
void SingletonManager::Exit() {
  RemoveReference();
}

/**
 * @brief Adds a finalizer function.
 * @param[in] (func) finalizer function
 * @param[in] (at_exit) If true, `func` is called at process exit.
 *     If false, `func` is called when all Core instances are released.
 */
void SingletonManager::AddFinalizer(FinalizerFunc func, bool at_exit) {
  if (at_exit) {
    g_finalizer_at_exit->Add(func);
  } else {
    g_finalizer->Add(func);
  }
}

/**
 * @brief Returns a mutex for singleton.
 */
Mutex* SingletonManager::GetMutex() {
  return g_mutex;
}

}  // namespace util
}  // namespace senscord
