/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_OSAL_NUTTX_OSAL_LINUXERROR_H_
#define LIB_OSAL_NUTTX_OSAL_LINUXERROR_H_

#include "./osal_error.h"

namespace senscord {
namespace osal {

/**
 * @brief Get error cause from errno.
 * @param[in] errno_val  Error number. (defined in <errno.h>)
 * @return OSAL error cause.
 *         If the errno_val is an undefined value, it returns kErrorUnknown.
 */
OSErrorCause GetErrorCauseFromErrno(int32_t err_no);

}  // namespace osal
}  // namespace senscord

#endif  // LIB_OSAL_NUTTX_OSAL_LINUXERROR_H_
