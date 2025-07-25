# SPDX-FileCopyrightText: 2023-2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

add_subdirectory(simple_stream)
add_subdirectory(smoke_test)

if(WAMR_BUILD_LIB_WASI_THREADS EQUAL 1 OR
   WAMR_BUILD_LIB_PTHREAD EQUAL 1)
  add_subdirectory(sample_pthread)
endif()
