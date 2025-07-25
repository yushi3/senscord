# SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(wasm-simple-stream VERSION ${SensCord_VERSION})

include(WasmToolchain)

set(TARGET_NAME senscord_simple_stream.wasm)

# copy source file
configure_file(
    ${SSDK_TOP_DIR}/sample/c/simple_stream/SensCordSimpleStream.c
    ${CMAKE_CURRENT_BINARY_DIR}/SensCordSimpleStream.c)

add_executable(${TARGET_NAME} ${CMAKE_CURRENT_BINARY_DIR}/SensCordSimpleStream.c)

target_include_directories(${TARGET_NAME} PRIVATE
    ${SSDK_BUILD_OUTPUT_INCLUDE_DIR}
    ${SSDK_INCLUDE_DIR})

# output to `output/wasm/senscord_simple_stream.wasm`
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SSDK_BUILD_OUTPUT_DIR}/wasm
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET_NAME}> ${SSDK_BUILD_OUTPUT_DIR}/wasm
)

install(TARGETS ${TARGET_NAME}
    EXPORT ${SSDK_EXPORT_NAME}
    RUNTIME DESTINATION wasm
)
