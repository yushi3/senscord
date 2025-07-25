/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_OSAL_INTTYPES_H_
#define SENSCORD_OSAL_INTTYPES_H_

#include "senscord/config.h"

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

#ifdef __wasm__
#undef __PRIS_PREFIX
#if UINTPTR_MAX == UINT64_MAX
#define __PRI64_PREFIX  "l"
#define __PRIPTR_PREFIX "l"
#define __PRIS_PREFIX   "l"
#else
#define __PRI64_PREFIX  "ll"
#define __PRIPTR_PREFIX ""
#define __PRIS_PREFIX   "l"
#endif

#ifndef PRId8
#define PRId8 "d"
#define PRIo8 "o"
#define PRIu8 "u"
#define PRIx8 "x"
#define PRIX8 "X"
#endif  /* PRId8 */

#ifndef PRId16
#define PRId16 "d"
#define PRIo16 "o"
#define PRIu16 "u"
#define PRIx16 "x"
#define PRIX16 "X"
#endif  /* PRId16 */

#ifndef PRId32
#define PRId32 "d"
#define PRIo32 "o"
#define PRIu32 "u"
#define PRIx32 "x"
#define PRIX32 "X"
#endif  /* PRId32 */

#ifndef PRId64
#define PRId64 __PRI64_PREFIX "d"
#define PRIo64 __PRI64_PREFIX "o"
#define PRIu64 __PRI64_PREFIX "u"
#define PRIx64 __PRI64_PREFIX "x"
#define PRIX64 __PRI64_PREFIX "X"
#endif  /* PRId64 */

#ifndef PRIdPTR
#define PRIdPTR __PRIPTR_PREFIX "d"
#define PRIoPTR __PRIPTR_PREFIX "o"
#define PRIuPTR __PRIPTR_PREFIX "u"
#define PRIxPTR __PRIPTR_PREFIX "x"
#define PRIXPTR __PRIPTR_PREFIX "X"
#endif  /* PRIdPTR */
#endif  /* __wasm__ */

#define PRIdS __PRIS_PREFIX "d"
#define PRIxS __PRIS_PREFIX "x"
#define PRIuS __PRIS_PREFIX "u"
#define PRIXS __PRIS_PREFIX "X"
#define PRIoS __PRIS_PREFIX "o"

#endif  /* SENSCORD_OSAL_INTTYPES_H_ */
