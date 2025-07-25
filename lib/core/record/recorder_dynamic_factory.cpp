/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/recorder_dynamic_factory.h"

namespace senscord {

typedef void* (*CreateRecorder) ();
typedef void (*DeleteRecorder) (void*);

 /**
  * @brief Call a function that creates instance.
  * @param[in] (instance) Instance to delete.
  * @param[out] (handle) Pointer to function to create instance.
  * @return Status object.
 */
Status RecorderDynamicFactory::CallCreateInstance(
    void* handle, void** instance) {
  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  if (handle == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // call function.
  CreateRecorder create_handle = reinterpret_cast<CreateRecorder>(handle);
  *instance = create_handle();
  if (*instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "create component failed : instance_name=%s", instance_name_.c_str());
  }

  return Status::OK();
}

/**
 * @brief Call a function that delete instance.
 * @param[in] (instance) Instance to delete.
 * @param[in] (handle) Pointer to function to delete instance.
 * @return Status object.
*/
Status RecorderDynamicFactory::CallDestroyInstance(
    void* handle, void* instance) {
  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  if (handle == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // call function.
  DeleteRecorder delete_handle = reinterpret_cast<DeleteRecorder>(handle);
  delete_handle(instance);
  return Status::OK();
}

/**
 * @brief Constructor.
 */
RecorderDynamicFactory::RecorderDynamicFactory() {}

/**
 * @brief Destructor.
 */
RecorderDynamicFactory::~RecorderDynamicFactory() {}

}  //  namespace senscord
