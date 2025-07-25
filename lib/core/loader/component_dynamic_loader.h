/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_LOADER_COMPONENT_DYNAMIC_LOADER_H_
#define LIB_CORE_LOADER_COMPONENT_DYNAMIC_LOADER_H_

#include <string>
#include <vector>
#include <map>

#include "senscord/osal.h"

#include "loader/class_dynamic_loader.h"
#include "senscord/develop/component.h"
#include "core/internal_types.h"

namespace senscord {

/**
 * @brief Component dynamic loader.
 */
class ComponentDynamicLoader : public ClassDynamicLoader {
 public:
  /**
   * @brief Constructor.
   */
  ComponentDynamicLoader();

  /**
   * @brief Destructor.
   */
  ~ComponentDynamicLoader();

  /**
   * @brief Generate an instance based on the component name of the argument.
   * @param[in] (name) Name of component.
   * @param[out] (component) Where to store the created component.
   * @return Status object.
   */
  Status Create(const std::string& name, Component** component);

  /**
   * @brief Delete the component passed in the argument.
   * @param[in] (name) Name of component.
   * @param[in] (component) Component to delete.
   * @return Status object.
   */
  Status Destroy(const std::string& name, Component* component);

 protected:
  /**
   * @brief A function that loads a library based on the argument name.
   * @param[in] (name) Key name of library.
   * @return Status object.
   */
  Status Load(const std::string& name);
};

}   // namespace senscord

#endif  //  LIB_CORE_LOADER_COMPONENT_DYNAMIC_LOADER_H_
