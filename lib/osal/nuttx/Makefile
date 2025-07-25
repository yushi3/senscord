# SPDX-FileCopyrightText: 2021-2025 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

-include $(TOPDIR)/Make.defs
-include $(SDKDIR)/Make.defs

BIN = libsenscord_osal$(LIBEXT)

CXXFLAGS += -std=c++11 -D__NuttX__ -nostdinc++ -isystem $(TOPDIR)/include/libcxx

CXXFLAGS += -I./include
CXXFLAGS += -I.

# osal
CXXSRCS += osal_cond_broadcastcond.cxx
CXXSRCS += osal_cond_createcond.cxx
CXXSRCS += osal_cond_destroycond.cxx
CXXSRCS += osal_cond_relativetimedwaitcond.cxx
CXXSRCS += osal_cond_signalcond.cxx
CXXSRCS += osal_cond_timedwaitcond.cxx
CXXSRCS += osal_cond_waitcond.cxx
CXXSRCS += osal_error.cxx
CXXSRCS += osal_exclusive_lock.cxx
CXXSRCS += osal_linuxerror.cxx
CXXSRCS += osal_mem_free.cxx
CXXSRCS += osal_mem_malloc.cxx
CXXSRCS += osal_mem_memchr.cxx
CXXSRCS += osal_mem_memcmp.cxx
CXXSRCS += osal_mem_memcpy.cxx
CXXSRCS += osal_mem_memmove.cxx
CXXSRCS += osal_mem_memset.cxx
CXXSRCS += osal_mutex_createmutex.cxx
CXXSRCS += osal_mutex_destroymutex.cxx
CXXSRCS += osal_mutex_lockmutex.cxx
CXXSRCS += osal_mutex_trylockmutex.cxx
CXXSRCS += osal_mutex_unlockmutex.cxx
CXXSRCS += osal_sleep.cxx
CXXSRCS += osal_stdio_printf.cxx
CXXSRCS += osal_stdio_vprintf.cxx
CXXSRCS += osal_stdio_vsnprintf.cxx
CXXSRCS += osal_string_strtoll.cxx
CXXSRCS += osal_string_strtoull.cxx
CXXSRCS += osal_thread.cxx
CXXSRCS += osal_thread_createthread.cxx
CXXSRCS += osal_thread_detachthread.cxx
CXXSRCS += osal_thread_jointhread.cxx
CXXSRCS += osal_thread_getcurrentthread.cxx
CXXSRCS += osal_time_gettime.cxx

include $(APPDIR)/Application.mk

