# SPDX-FileCopyrightText: 2021-2025 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

ifeq ($(CONFIG_EXTERNALS_SENSCORD_OSAL),y)
  EXTRA_LIBPATHS += -L "$(EXTLIBDIR)$(DELIM)senscord_osal"
  EXTRA_LIBS     += -lsenscord_osal
endif
