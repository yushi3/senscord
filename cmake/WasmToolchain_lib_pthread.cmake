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
set(CMAKE_SYSROOT           ${WAMR_ROOT_DIR}/wamr-sdk/app/libc-builtin-sysroot)

set(CMAKE_C_FLAGS           "-nostdlib -pthread -Qunused-arguments")
set(CMAKE_C_COMPILER_TARGET "wasm32")
set(CMAKE_C_COMPILER        "${WASI_SDK_DIR}/bin/clang")

set(CMAKE_CXX_FLAGS           "-nostdlib -pthread -Qunused-arguments")
set(CMAKE_CXX_COMPILER_TARGET "wasm32")
set(CMAKE_CXX_COMPILER        "${WASI_SDK_DIR}/bin/clang++")

# -Wl,--export=__heap_base,--export=__data_end:
#     export these globals so the runtime can resolve the total aux stack size
#     and the start offset of the stack top
# -Wl,--export=__wasm_call_ctors:
#     export the init function to initialize the passive data segments
set(CMAKE_EXE_LINKER_FLAGS
    "-Wl,--shared-memory                          \
     -Wl,--no-entry,--strip-all                   \
     -Wl,--export=__main_argc_argv                \
     -Wl,--export=__heap_base,--export=__data_end \
     -Wl,--export=__wasm_call_ctors               \
     -Wl,--allow-undefined"
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wformat -Wformat-security")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wformat -Wformat-security")
