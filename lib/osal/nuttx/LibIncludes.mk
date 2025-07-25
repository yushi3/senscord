# SPDX-FileCopyrightText: 2021-2025 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

ifeq ($(CONFIG_EXTERNALS_SENSCORD_OSAL),y)
CFLAGS   += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "$(SDKDIR)/../externals/senscord_osal/include"}
CXXFLAGS += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "$(SDKDIR)/../externals/senscord_osal/include"}
endif
