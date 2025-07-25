/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "loader/component_dynamic_factory.h"

#include "senscord/develop/component.h"

namespace senscord {

typedef void* (*CreateComponent) ();
typedef void (*DeleteComponent) (void*);

/**
 * @brief Constructor.
 */
ComponentDynamicFactory::ComponentDynamicFactory() {
}

/**
 * @brief Destructor.
 */
ComponentDynamicFactory::~ComponentDynamicFactory() {
}

 /**
  * @brief Call a function that creates instance.
  * @param[in] (instance) Instance to delete.
  * @param[out] (handle) Pointer to function to create instance.
  * @return Status object.
 */
Status ComponentDynamicFactory::CallCreateInstance(void* handle,
                                                   void** instance) {
  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  if (handle == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  CreateComponent create_handle
    = reinterpret_cast<CreateComponent>(handle);

  // call function.
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
Status ComponentDynamicFactory::CallDestroyInstance(void* handle,
                                                    void* instance) {
  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  if (handle == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  DeleteComponent delete_handle
    = reinterpret_cast<DeleteComponent>(handle);

  // call function.
  delete_handle(instance);

  return Status::OK();
}

}  //  namespace senscord
