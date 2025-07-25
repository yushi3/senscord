/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_LOADER_COMPONENT_DYNAMIC_FACTORY_H_
#define LIB_CORE_LOADER_COMPONENT_DYNAMIC_FACTORY_H_

#include "loader/class_dynamic_factory.h"

namespace senscord {

/**
 * @brief Component dynamic factory.
 */
class ComponentDynamicFactory : public ClassDynamicFactory {
 public:
  /**
   * @brief Constructor.
   */
  ComponentDynamicFactory();

  /**
   * @brief Destructor.
   */
  ~ComponentDynamicFactory();

 protected:
  /**
   * @brief Call a function that creates instance.
   * @param[in] (handle) Pointer to function to create instance.
   * @param[out] (instance) Instance to delete.
   * @return Status object.
   */
  Status CallCreateInstance(void* handle, void** instance);

  /**
   * @brief Call a function that delete instance.
   * @param[in] (handle) Pointer to function to delete instance.
   * @param[in] (instance) Instance to delete.
   * @return Status object.
   */
  Status CallDestroyInstance(void* handle, void* instance);
};

}   // namespace senscord

#endif  //  LIB_CORE_LOADER_COMPONENT_DYNAMIC_FACTORY_H_
