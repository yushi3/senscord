# SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# Add build directory.
if(SENSCORD_API_C)
  add_subdirectory(c)
endif()

add_subdirectory(cpp)

if(SENSCORD_API_PYTHON)
  add_subdirectory(python)
endif()
