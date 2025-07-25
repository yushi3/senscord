/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "c_api/converter_dynamic_factory.h"

namespace senscord {

typedef void* (*CreateConverter)();
typedef void (*DeleteConverter)(void*);

/**
 * @brief Constructor.
 */
ConverterDynamicFactory::ConverterDynamicFactory() {
}

/**
 * @brief Destructor.
 */
ConverterDynamicFactory::~ConverterDynamicFactory() {
}

/**
 * @brief Call a function that creates instance.
 * @param[in]  handle    Pointer to function to create instance.
 * @param[out] instance  Instance to delete.
 * @return Status object.
 */
Status ConverterDynamicFactory::CallCreateInstance(
    void* handle, void** instance) {
  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  if (handle == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  CreateConverter create_handle = reinterpret_cast<CreateConverter>(handle);

  // call function.
  *instance = create_handle();
  if (*instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "create converter failed : instance_name=%s", instance_name_.c_str());
  }

  return Status::OK();
}

/**
 * @brief Call a function that delete instance.
 * @param[in] handle    Pointer to function to delete instance.
 * @param[in] instance  Instance to delete.
 * @return Status object.
 */
Status ConverterDynamicFactory::CallDestroyInstance(
    void* handle, void* instance) {
  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  if (handle == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  DeleteConverter delete_handle = reinterpret_cast<DeleteConverter>(handle);

  // call function.
  delete_handle(instance);

  return Status::OK();
}

}  // namespace senscord
