/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>  /* std::max */
#include "senscord/osal.h"
#include "common/osal_error.h"
#include "linux/osal_linuxerror.h"

namespace senscord {
namespace osal {

/**
 * @brief Extract file name from file path.
 * @param[in] path  File path.
 * @return Pointer of extracted file name, or NULL.
 */
const char* OSBasename(const char* path) {
  if (path == NULL) {
    return NULL;
  }
  const char* ptr = strrchr(path, '/');
  if (ptr == NULL) {
    ptr = strrchr(path, '\\');
  }
  if (ptr != NULL) {
    return ptr + 1;
  }
  return path;
}

}  // namespace osal
}  // namespace senscord
