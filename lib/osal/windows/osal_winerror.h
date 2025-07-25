/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_OSAL_WINDOWS_OSAL_WINERROR_H_
#define LIB_OSAL_WINDOWS_OSAL_WINERROR_H_

#include "common/osal_error.h"

namespace senscord {
namespace osal {

/**
 * @brief Get error cause from errno.
 * @param[in] errno_val  Error number. (defined in <errno.h>)
 * @return OSAL error cause.
 *         If the errno_val is an undefined value, it returns kErrorUnknown.
 */
OSErrorCause GetErrorCauseFromErrno(int32_t errno_val);

/**
 * @brief Get error cause from Winsock error.
 * @param[in] wsa_err  Error code. (defined in <Winsock2.h>)
 * @return OSAL error cause.
 *         If the wsa_err is an undefined value, it returns kErrorUnknown.
 */
OSErrorCause GetErrorCauseFromWinsock(int32_t wsa_err);

}  // namespace osal
}  // namespace senscord

#endif  // LIB_OSAL_WINDOWS_OSAL_WINERROR_H_
