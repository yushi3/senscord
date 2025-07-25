# SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(wasm-sample-pthread VERSION ${SensCord_VERSION})

include(WasmToolchain)

set(TARGET_NAME senscord_sample_pthread.wasm)

# ============================================================================

add_executable(${TARGET_NAME} sample_pthread.c)

target_include_directories(${TARGET_NAME} PRIVATE
    ${SSDK_BUILD_OUTPUT_INCLUDE_DIR}
    ${SSDK_INCLUDE_DIR})

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SSDK_BUILD_OUTPUT_DIR}/wasm
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET_NAME}> ${SSDK_BUILD_OUTPUT_DIR}/wasm
)

install(TARGETS ${TARGET_NAME}
    EXPORT ${SSDK_EXPORT_NAME}
    RUNTIME DESTINATION wasm
)

# ============================================================================

set(TARGET_NAME senscord_sample_pthread2.wasm)

add_executable(${TARGET_NAME} sample_pthread2.c)

target_include_directories(${TARGET_NAME} PRIVATE
    ${SSDK_BUILD_OUTPUT_INCLUDE_DIR}
    ${SSDK_INCLUDE_DIR})

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SSDK_BUILD_OUTPUT_DIR}/wasm
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET_NAME}> ${SSDK_BUILD_OUTPUT_DIR}/wasm
)

install(TARGETS ${TARGET_NAME}
    EXPORT ${SSDK_EXPORT_NAME}
    RUNTIME DESTINATION wasm
)
