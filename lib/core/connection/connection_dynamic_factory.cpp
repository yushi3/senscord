/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "connection/connection_dynamic_factory.h"

namespace senscord {

typedef void* (*CreateConnection) ();
typedef void (*DeleteConnection) (void*);

/**
 * @brief Constructor.
 */
ConnectionDynamicFactory::ConnectionDynamicFactory() {
}

/**
 * @brief Destructor.
 */
ConnectionDynamicFactory::~ConnectionDynamicFactory() {
}

 /**
  * @brief Call a function that creates instance.
  * @param[in]  (handle) Pointer to function to create instance.
  * @param[out] (instance) Instance to delete.
  * @return Status object.
 */
Status ConnectionDynamicFactory::CallCreateInstance(
    void* handle, void** instance) {
  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  if (handle == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  CreateConnection create_handle = reinterpret_cast<CreateConnection>(handle);

  // call function.
  *instance = create_handle();
  if (*instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "create connection failed : instance_name=%s", instance_name_.c_str());
  }

  return Status::OK();
}

/**
 * @brief Call a function that delete instance.
 * @param[in] (handle) Pointer to function to delete instance.
 * @param[in] (instance) Instance to delete.
 * @return Status object.
*/
Status ConnectionDynamicFactory::CallDestroyInstance(
    void* handle, void* instance) {
  if (instance == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  if (handle == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  DeleteConnection delete_handle = reinterpret_cast<DeleteConnection>(handle);

  // call function.
  delete_handle(instance);

  return Status::OK();
}

}  //  namespace senscord
