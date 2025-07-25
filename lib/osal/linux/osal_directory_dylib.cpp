/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>

#include <string>

#include "senscord/osal.h"
#include "common/osal_error.h"

namespace senscord {
namespace osal {

/**
 * @brief Get the file name of the dynamic library.
 * @param[in] base  Base file name.
 * @param[out] name  Dynamic library file name.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetDynamicLibraryFileName(
    const std::string& base, std::string* name) {
  static const OSFunctionId kFuncId = kIdOSGetDynamicLibraryFileName;
  if (name == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  *name = "lib" + base + ".so";
  return 0;
}

}  //  namespace osal
}  //  namespace senscord
