# SPDX-FileCopyrightText: 2023-2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

if(WAMR_BUILD_LIB_WASI_THREADS EQUAL 1)
  include(WasmToolchain_wasi_threads)
else()
  include(WasmToolchain_lib_pthread)
endif()
