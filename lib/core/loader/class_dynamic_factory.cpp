/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "loader/class_dynamic_factory.h"

namespace senscord {

/**
 * @brief Constructor.
 */
ClassDynamicFactory::ClassDynamicFactory() {
  handle_ = NULL;
  create_instance_handle_ = NULL;
  destroy_instance_handle_ = NULL;
}

/**
 * @brief Destructor.
 */
ClassDynamicFactory::~ClassDynamicFactory() {
}

/**
 * @brief Set handle of shared library to factory.
 * @param[in] (handle) Shared library handle.
 * @param[in] (func_create) Pointer to Create function of instance.
 * @param[in] (func_destroy) Pointer to delete instance function.
 */
void ClassDynamicFactory::SetHandle(osal::OSDlHandle* handle,
                                    void* func_create,
                                    void* func_destroy) {
  handle_ = handle;
  create_instance_handle_ = func_create;
  destroy_instance_handle_ = func_destroy;
}

/**
 * @brief Get the handle set in the factory.
 * @param[out] (handle) Shared library handle.
 * @return Status object.
 */
Status ClassDynamicFactory::GetHandle(osal::OSDlHandle** handle) {
  if (handle == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  *handle = handle_;

  return Status::OK();
}

/**
 * @brief Create an instance.
 * @param[out] (instance) A storage pointer of the created instance.
 * @return Status object.
 */
Status ClassDynamicFactory::CreateInstance(void** instance) {
  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  Status ret = CallCreateInstance(create_instance_handle_, instance);
  SENSCORD_STATUS_TRACE(ret);
  if (!ret.ok()) {
    return ret;
  }

  instance_list_.push_back(*instance);

  return Status::OK();
}

/**
 * @brief Delete an instance.
 * @param[in] (instance) Instance to delete.
 * @return Status object.
 */
Status ClassDynamicFactory::DestroyInstance(void* instance) {
  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  InstanceListVector::iterator it = instance_list_.begin();

  while (it != instance_list_.end()) {
    if (*it == instance) {
      instance_list_.erase(it);
      Status ret = CallDestroyInstance(destroy_instance_handle_, instance);
      SENSCORD_STATUS_TRACE(ret);
      if (!ret.ok()) {
        return ret;
      }
      break;
    } else {
      ++it;
    }
  }

  return Status::OK();
}

}    // namespace senscord
