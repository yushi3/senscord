/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_LOADER_CLASS_DYNAMIC_LOADER_H_
#define LIB_CORE_LOADER_CLASS_DYNAMIC_LOADER_H_

#include <map>
#include <string>

#include "senscord/osal.h"
#include "senscord/status.h"
#include "loader/class_dynamic_factory.h"
#include "util/mutex.h"

namespace senscord {

typedef std::map<std::string, ClassDynamicFactory*>::iterator FactoryIterator;

/**
 * @brief Class dynamic loader.
 */
class ClassDynamicLoader {
 public:
  /**
   * @brief Destructor.
   */
  virtual ~ClassDynamicLoader();

 protected:
  /**
   * @brief Constructor.
   */
  ClassDynamicLoader();

  /**
   * @brief Generate an instance based on the library name of the argument.
   * @param[in] (name) Key name of library.
   * @param[out] (instance) Where to store the created instance.
   * @return Status object.
   */
  Status Create(const std::string& name, void** instance);

  /**
   * @brief Delete the instance passed in the argument.
   * @param[in] (name) Key name of library.
   * @param[in] (instance) Instance to delete.
   * @return Status object.
   */
  Status Destroy(const std::string& name, void* instance);

  /**
   * @brief Register factory in association with component name.
   * @param[in] (name) Key name of library.
   * @param[in] (factory) Factory to register.
   * @return Status object.
   */
  Status SetFactory(const std::string& name, ClassDynamicFactory* factory);

  /**
   * @brief Get the factory with the component name as the key.
   * @param[in] (name) Key name of library.
   * @param[out] (factory) Factory pointer.
   * @return Status object.
   */
  Status GetFactory(const std::string& name, ClassDynamicFactory** factory);

  /**
   * @brief Get the path of the directory containing the specified library.
   * @param[in] (name) Filename of library.
   * @param[out] (lib_path) Path storage location.
   * @return Status object.
   */
  Status GetLibraryPath(const std::string& name, std::string* lib_path);

  /**
   * @brief Load library and register factory.
   * @param[in] (file_path) Path to the directory where the library is located.
   * @param[in] (create_function) Name of the function that creates the instance.
   * @param[in] (destroy_function) Name of function to destroy instance.
   * @param[in] (factory) Pointer of factory to register.
   * @return Status object.
   */
  Status LoadAndRegisterLibrary(
    const std::string& file_path,
    const std::string& create_function,
    const std::string& destroy_function,
    ClassDynamicFactory* factory);

  /**
   * @brief A function that loads a library based on the argument name.
   * @param[in] (name) Key name of library.
   * @return Status object.
   */
  virtual Status Load(const std::string& name) = 0;

 private:
  /**
   * @brief Unload the loaded library.
   * @param[in] (name) Key name of library.
   * @return Status object.
   */
  Status Unload(const std::string& name);

  // Map of class factory.
  std::map<std::string, ClassDynamicFactory*> factory_map_;

  // factory map lock object
  util::Mutex* mutex_;
};
}    // namespace senscord
#endif  // LIB_CORE_LOADER_CLASS_DYNAMIC_LOADER_H_
