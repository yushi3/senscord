/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_C_API_CONVERTER_DYNAMIC_FACTORY_H_
#define LIB_CORE_C_API_CONVERTER_DYNAMIC_FACTORY_H_

#include "loader/class_dynamic_factory.h"

namespace senscord {

/**
 * @brief Converter dynamic factory.
 */
class ConverterDynamicFactory : public ClassDynamicFactory {
 public:
  /**
   * @brief Constructor.
   */
  ConverterDynamicFactory();

  /**
   * @brief Destructor.
   */
  ~ConverterDynamicFactory();

 protected:
  /**
   * @brief Call a function that creates instance.
   * @param[in]  handle    Pointer to function to create instance.
   * @param[out] instance  Instance to delete.
   * @return Status object.
   */
  Status CallCreateInstance(void* handle, void** instance);

  /**
   * @brief Call a function that delete instance.
   * @param[in] handle    Pointer to function to delete instance.
   * @param[in] instance  Instance to delete.
   * @return Status object.
   */
  Status CallDestroyInstance(void* handle, void* instance);
};

}  // namespace senscord

#endif  //  LIB_CORE_C_API_CONVERTER_DYNAMIC_FACTORY_H_
