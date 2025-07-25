# SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

if(APPLE)
  set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)
  set(CMAKE_C_LINK_FLAGS "")
  set(CMAKE_CXX_LINK_FLAGS "")
endif()

# reset coverage flags
set(CMAKE_C_FLAGS_COVERAGE "")
set(CMAKE_CXX_FLAGS_COVERAGE "")

set(CMAKE_SYSTEM_PROCESSOR  wasm32)
set(CMAKE_SYSROOT           ${WASI_SDK_DIR}/share/wasi-sysroot)

set(CMAKE_C_FLAGS           "-pthread")
set(CMAKE_C_COMPILER        "${WASI_SDK_DIR}/bin/clang")
set(CMAKE_C_COMPILER_TARGET "wasm32-wasi-threads")

set(CMAKE_CXX_FLAGS           "-pthread")
set(CMAKE_CXX_COMPILER        "${WASI_SDK_DIR}/bin/clang++")
set(CMAKE_CXX_COMPILER_TARGET "wasm32-wasi-threads")

set(CMAKE_EXE_LINKER_FLAGS
    "-target wasm32-wasi-threads                  \
     -Wl,--shared-memory                          \
     -Wl,--import-memory,--export-memory          \
     -Wl,--max-memory=1048576                     \
     -Wl,--no-entry,--strip-all                   \
     -Wl,--export=__main_argc_argv                \
     -Wl,--export=__heap_base,--export=__data_end \
     -Wl,--export=__wasm_call_ctors               \
     -Wl,--export=malloc,--export=free            \
     -Wl,--allow-undefined"
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wformat -Wformat-security")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wformat -Wformat-security")
