/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "loader/component_dynamic_loader.h"

#include <inttypes.h>
#include <sstream>

#include "util/senscord_utils.h"
#include "loader/component_dynamic_factory.h"
#include "logger/logger.h"
#include "senscord/osal.h"
#include "senscord/environment.h"

namespace senscord {

// Name of the function to be obtained from the library.
static const char kCreateInstance[] = "CreateComponent";
static const char kDestroyInstance[] = "DestroyComponent";

/**
 * @brief Constructor.
 */
ComponentDynamicLoader::ComponentDynamicLoader() {}

/**
 * @brief Destructor.
 */
ComponentDynamicLoader::~ComponentDynamicLoader() {}

/**
 * @brief Generate an instance based on the component name of the argument.
 * @param[in] (name) Name of component.
 * @param[out] (component) Where to store the created component.
 * @return Status object.
 */
Status ComponentDynamicLoader::Create(const std::string& name,
                                      Component** component) {
  Status ret =
      ClassDynamicLoader::Create(name, reinterpret_cast<void**>(component));
  return SENSCORD_STATUS_TRACE(ret);
}

/**
 * @brief Delete the component passed in the argument.
 * @param[in] (name) Name of component.
 * @param[in] (component) Component to delete.
 * @return Status object.
 */
Status ComponentDynamicLoader::Destroy(const std::string& name,
                                       Component* component) {
  Status ret = ClassDynamicLoader::Destroy(name, component);
  return SENSCORD_STATUS_TRACE(ret);
}

/**
 * @brief A function that loads a library based on the argument name.
 * @param[in] (name) Key name of library.
 * @return Status object.
 */
Status ComponentDynamicLoader::Load(const std::string& name) {
  std::string file_path;
  Status ret = GetLibraryPath(name, &file_path);
  if (!ret.ok()) {
    return SENSCORD_STATUS_TRACE(ret);
  }

  // Register component factory as loader.
  ComponentDynamicFactory* factory = new ComponentDynamicFactory();

  ret = LoadAndRegisterLibrary(file_path, kCreateInstance,
      kDestroyInstance, factory);
  if (!ret.ok()) {
    delete factory;
    return SENSCORD_STATUS_TRACE(ret);
  }

  SetFactory(name, factory);

  return Status::OK();
}

}   // namespace senscord
