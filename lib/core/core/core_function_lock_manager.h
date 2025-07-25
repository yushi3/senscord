/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CORE_CORE_FUNCTION_LOCK_MANAGER_H_
#define LIB_CORE_CORE_CORE_FUNCTION_LOCK_MANAGER_H_

#include <set>
#include <string>
#include <map>

#include "senscord/status.h"
#include "senscord/osal.h"
#include "core/stream_manager.h"
#include "util/mutex.h"

namespace senscord {

/**
 * @brief Type of function.
 */
enum CoreFunctionType {
  kFunctionTypeInit,
  kFunctionTypeExit,
  kFunctionTypeReadOnly,
  kFunctionTypeStream,
};

/**
 * @brief Exclusive lock management class for Core.
 */
class CoreFunctionLockManager {
 public:
  /**
   * @brief Constructor.
   */
  CoreFunctionLockManager();

  /**
   * @brief Destructor.
   */
  ~CoreFunctionLockManager();

  /**
   * @brief Get the initialization state of the Core class.
   */
  bool IsCoreInitialized() const;

  /**
   * @brief Set the initialization state of the Core class.
   */
  void SetCoreInitialized(bool initialized);

  /**
   * @brief Insert the closing stream information.
   * @param[in] stream target stream isntance.
   */
  void InsertClosingStream(Stream* stream);

  /**
   * @brief Remove the closing stream information.
   * @param[in] stream target stream isntance.
   */
  void RemoveClosingStream(Stream* stream);

  /**
   * @brief Set the closed stream status.
   * @param[in] stream  target stream isntance.
   * @param[in] status  close stream status.
   */
  void SetCloseStreamStatus(Stream* stream, const Status& status);

  /**
   * @brief Get the closed stream status.
   * @param[in]  stream     target stream isntance.
   * @param[out] is_closed  true is already running close stream.
   * @return closed status (when is_closed is true only).
   */
  Status GetCloseStreamStatus(Stream* stream, bool* is_closed);

  /**
   * @brief Lock for Init function.
   */
  Status LockForInit();

  /**
   * @brief Lock for Exit function.
   */
  Status LockForExit();

  /**
   * @brief Lock for ReadOnly functions.
   */
  Status LockForReadOnly();

  /**
   * @brief Lock for Stream functions.
   */
  Status LockForStream(
      const std::string& stream_key, const ConfigManager& config_manager);

  /**
   * @brief Release the lock of the function.
   * @param[in] stream related stream instance (if not, it is null).
   */
  void Unlock(Stream* stream);

 private:
  /**
   * @brief Insert data into the function list.
   * @param[in] type  Type of function.
   * @param[in] key   Key to identify.
   */
  void Insert(CoreFunctionType type, const std::string& key);

  /**
   * @brief Remove data from the function list.
   */
  void Remove();

  /**
   * @brief Waking up the other accesses that are waiting.
   */
  void WakeupWaitFunction();

  util::Mutex* mutex_;
  osal::OSCond* cond_;

  // Initialization state of the Core class.
  bool core_initialized_;

  /**
   * @brief Information of the function.
   */
  struct FunctionInfo {
    CoreFunctionType type;
    std::string key;
  };

  typedef std::map<osal::OSThread*, FunctionInfo> FunctionList;

  // List of running functions.
  FunctionList running_functions_;

  /**
   * @brief Information of the close stream status.
   */
  struct CloseStreamStatusInfo {
    std::set<osal::OSThread*> closing_threads;
    Status status;
    bool closed;
  };

  typedef std::map<Stream*, CloseStreamStatusInfo> CloseStatusList;

  // List of close stream statuses.
  CloseStatusList close_stream_info_;
};

/**
 * @brief RAII-style function lock.
 */
class CoreFunctionLock {
 public:
  /**
   * @brief Acquire the lock of the function.
   * @param[in] manager  Lock manager.
   * @param[in] type     Type of function.
   *
   * Available types are as follows:
   *  - kFunctionTypeInit
   *  - kFunctionTypeExit
   *  - kFunctionTypeReadOnly
   */
  CoreFunctionLock(CoreFunctionLockManager* manager, CoreFunctionType type);

  explicit CoreFunctionLock(CoreFunctionLockManager* manager);

  /**
   * @brief Acquire the lock of the function with stream key.
   * @param[in] manager     Lock manager.
   * @param[in] stream_key  Stream key.
   * @param[in] config_manager  Config manager.
   *
   * The type is fixed to kFunctionTypeStream.
   */
  CoreFunctionLock(CoreFunctionLockManager* manager,
      const std::string& stream_key, const ConfigManager& config_manager);

  /**
   * @brief Acquire the lock of the function with stream instance.
   * @param[in] manager         Lock manager.
   * @param[in] stream_manager  Stream manager.
   * @param[in] stream          Stream instance.
   * @param[in] config_manager  Config manager.
   *
   * The type is fixed to kFunctionTypeStream.
   */
  CoreFunctionLock(CoreFunctionLockManager* manager,
      StreamManager* stream_manager,
      Stream* stream, const ConfigManager& config_manager);

  /**
   * @brief Release the lock of the function.
   */
  ~CoreFunctionLock();

  /**
   * @brief Get the lock status.
   */
  Status status() const;

 private:
  CoreFunctionLockManager* manager_;
  Stream* stream_;
  Status status_;
};

}  // namespace senscord

#endif  // LIB_CORE_CORE_CORE_FUNCTION_LOCK_MANAGER_H_
