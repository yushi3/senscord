/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_LOADER_CLASS_DYNAMIC_FACTORY_H_
#define LIB_CORE_LOADER_CLASS_DYNAMIC_FACTORY_H_

#include <stdint.h>

#include <list>
#include <string>
#include <vector>

#include "senscord/osal.h"
#include "senscord/status.h"

namespace senscord {

typedef std::vector<void*> InstanceListVector;

/**
 * @brief Class dynamic factory.
 */
class ClassDynamicFactory {
 public:
  /**
   * @brief Destructor.
   */
  virtual ~ClassDynamicFactory();

  /**
   * @brief Set handle of shared library to factory.
   * @param[in] (handle) Shared library handle.
   * @param[in] (func_create) Pointer to Create function of instance.
   * @param[in] (func_destroy) Pointer to delete instance function.
   */
  void SetHandle(osal::OSDlHandle* handle,
                 void* func_create,
                 void* func_destroy);

  /**
   * @brief Get the handle set in the factory.
   * @param[out] (handle) Shared library handle.
   * @return Status object.
   */
  Status GetHandle(osal::OSDlHandle** handle);

  /**
   * @brief Create an instance.
   * @param[out] (instance) A storage pointer of the created instance.
   * @return Status object.
   */
  Status CreateInstance(void** instance);

  /**
   * @brief Delete an instance.
   * @param[in] (instance) Instance to delete.
   * @return Status object.
   */
  Status DestroyInstance(void* instance);

  /**
   * @brief Returns the size of the list of instances held by Factory.
   * @return Number of instances.
   */
  int32_t GetInstanceNum() {
    return static_cast<int32_t>(instance_list_.size());
  }

  /**
   * @brief Set the name of the Instance
   * @param[in] (name) Name of instance.
   */
  void SetInstanceName(const std::string& name) {
    instance_name_ = name;
  }

 protected:
  /**
   * @brief Constructor.
   */
  ClassDynamicFactory();

  /**
   * @brief Call a function that creates instance.
   * @param (handle) Pointer to function to create instance.
   * @param (instance) Instance to delete.
   * @return Status object.
   */
  virtual Status CallCreateInstance(void* handle, void** instance) = 0;

  /**
   * @brief Call a function that delete instance.
   * @param (handle) Pointer to function to delete instance.
   * @param (instance) Instance to delete.
   * @return Status object.
   */
  virtual Status CallDestroyInstance(void* handle, void* instance) = 0;

  // Name of instance.
  std::string instance_name_;

 private:
  // Handler for instance creation function.
  void* create_instance_handle_;
  // Handler for instance delete function.
  void* destroy_instance_handle_;
  // Shared library handle.
  osal::OSDlHandle* handle_;
  // List of created instances.
  std::vector<void*> instance_list_;
};
}    // namespace senscord
#endif  // LIB_CORE_LOADER_CLASS_DYNAMIC_FACTORY_H_
