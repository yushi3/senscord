/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_PYTHON_SRC_PYTHON_GIL_H_
#define LIB_PYTHON_SRC_PYTHON_GIL_H_

#include "python_limited.h"
#include "senscord/noncopyable.h"

namespace senscord {

/**
 * @brief Python's global interpreter lock.
 */
class PythonGlobalInterpreterLock : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   *
   * Acquire the global interpreter lock.
   */
  PythonGlobalInterpreterLock() : gil_state_() {
    gil_state_ = PyGILState_Ensure();
  }

  /**
   * @brief Destructor.
   *
   * Release the global interpreter lock.
   */
  ~PythonGlobalInterpreterLock() {
    PyGILState_Release(gil_state_);
  }

 private:
  PyGILState_STATE gil_state_;
};

}  // namespace senscord

#endif  // LIB_PYTHON_SRC_PYTHON_GIL_H_
