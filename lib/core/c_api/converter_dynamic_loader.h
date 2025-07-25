/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_C_API_CONVERTER_DYNAMIC_LOADER_H_
#define LIB_CORE_C_API_CONVERTER_DYNAMIC_LOADER_H_

#include <string>

#include "loader/class_dynamic_loader.h"
#include "senscord/develop/converter.h"

namespace senscord {

/**
 * @brief Converter dynamic loader.
 */
class ConverterDynamicLoader : public ClassDynamicLoader {
 public:
  /**
   * @brief Constructor.
   */
  ConverterDynamicLoader();

  /**
   * @brief Destructor.
   */
  ~ConverterDynamicLoader();

  /**
   * @brief Generate an instance based on the converter name of the argument.
   * @param[in]  name       Name of converter library.
   * @param[out] converter  Where to store the created converter.
   * @return Status object.
   */
  Status Create(const std::string& name, ConverterLibrary** converter);

  /**
   * @brief Delete the converter passed in the argument.
   * @param[in] name        Name of converter library.
   * @param[in] connection  Converter to delete.
   * @return Status object.
   */
  Status Destroy(const std::string& name, ConverterLibrary* converter);

 protected:
  /**
   * @brief A function that loads a library based on the argument name.
   * @param[in] name  Key name of library.
   * @return Status object.
   */
  Status Load(const std::string& name);
};

}  // namespace senscord

#endif  //  LIB_CORE_C_API_CONVERTER_DYNAMIC_LOADER_H_
