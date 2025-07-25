/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "c_api/converter_dynamic_loader.h"

#include <string>
#include "c_api/converter_dynamic_factory.h"

namespace senscord {

// Name of the function to be obtained from the library.
static const char kCreateInstance[] = "CreateConverter";
static const char kDestroyInstance[] = "DeleteConverter";

/**
 * @brief Constructor.
 */
ConverterDynamicLoader::ConverterDynamicLoader() {}

/**
 * @brief Destructor.
 */
ConverterDynamicLoader::~ConverterDynamicLoader() {}

/**
 * @brief Generate an instance based on the converter name of the argument.
 * @param[in]  name       Name of converter library.
 * @param[out] converter  Where to store the created converter.
 * @return Status object.
 */
Status ConverterDynamicLoader::Create(
    const std::string& name, ConverterLibrary** converter) {
  Status ret = ClassDynamicLoader::Create(
      name, reinterpret_cast<void**>(converter));
  return SENSCORD_STATUS_TRACE(ret);
}

/**
 * @brief Delete the converter passed in the argument.
 * @param[in] name        Name of converter library.
 * @param[in] connection  Converter to delete.
 * @return Status object.
 */
Status ConverterDynamicLoader::Destroy(
    const std::string& name, ConverterLibrary* converter) {
  Status ret = ClassDynamicLoader::Destroy(name, converter);
  return SENSCORD_STATUS_TRACE(ret);
}

/**
 * @brief A function that loads a library based on the argument name.
 * @param[in] (name) Key name of library.
 * @return Status object.
 */
Status ConverterDynamicLoader::Load(const std::string& name) {
  std::string file_path;
  Status ret = GetLibraryPath(name, &file_path);
  if (!ret.ok()) {
    return ret;
  }

  // Register factory as loader.
  ConverterDynamicFactory* factory = new ConverterDynamicFactory();
  ret = LoadAndRegisterLibrary(
      file_path, kCreateInstance, kDestroyInstance, factory);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    delete factory;
    return ret;
  }

  SetFactory(name, factory);
  return Status::OK();
}

}  // namespace senscord
