/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_OSAL_INTTYPES_H_
#define SENSCORD_OSAL_INTTYPES_H_

/*
 * printf format specifiers for size_t.
 * Example:
 *   size_t size = xxxx;
 *   osal::OSPrintf("size = %" PRIuS "\n", size);
 */

#ifdef _WIN64
#define __PRIS_PREFIX "z"
#else
#ifdef _LP64
#define __PRIS_PREFIX "z"
#else
#define __PRIS_PREFIX
#endif
#endif  /* _WIN64 */

#define PRIdS __PRIS_PREFIX "d"
#define PRIxS __PRIS_PREFIX "x"
#define PRIuS __PRIS_PREFIX "u"
#define PRIXS __PRIS_PREFIX "X"
#define PRIoS __PRIS_PREFIX "o"

#endif  /* SENSCORD_OSAL_INTTYPES_H_ */
