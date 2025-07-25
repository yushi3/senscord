/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_C_API_C_CORE_H_
#define LIB_CORE_C_API_C_CORE_H_

#include <vector>
#include <string>
#include "senscord/senscord.h"
#include "senscord/c_api/senscord_c_types.h"
#include "util/mutex.h"

namespace senscord {
namespace c_api {

/**
 * @brief Data of core handle.
 */
struct CoreHandle {
  /** Core object pointer */
  Core* core;
  /** Mutex for this handle */
  util::Mutex mutex;
  /** Cache of supported stream list */
  std::vector<StreamTypeInfo> supported_stream_list_cache;
  /** Cache of senscord version */
  senscord_version_t* senscord_version_cache;
};

}  // namespace c_api
}  // namespace senscord

#endif  // LIB_CORE_C_API_C_CORE_H_
