# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(ws_connection VERSION ${SensCord_VERSION})

# Name of the build target
set(TARGET_NAME ${PROJECT_NAME})

### Setting Build Options.
# add -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(WIN32)
  message(STATUS "win __cplusplus & c++14 on")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /std:c++14")
endif()

# Change the output directory.
set_ssdk_library_output_dir(${SSDK_CONNECTION_OUT_DIR})

### Sources
# Add senscord lib source codes.
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/ws_connection.cpp
    ${PROJECT_SOURCE_DIR}/ws_connection_manager.cpp
    ${PROJECT_SOURCE_DIR}/shared_memory_manager.cpp
    ${PROJECT_SOURCE_DIR}/../../../thirdparty/websocket/websocket.c
    ${PROJECT_SOURCE_DIR}/messagepack/web_socket_bridge.cpp
    ${PROJECT_SOURCE_DIR}/messagepack/register_access_64_property.cpp
    ${PROJECT_SOURCE_DIR}/messagepack/register_access_32_property.cpp
    ${PROJECT_SOURCE_DIR}/messagepack/register_access_16_property.cpp
    ${PROJECT_SOURCE_DIR}/messagepack/register_access_08_property.cpp
    ${PROJECT_SOURCE_DIR}/messagepack/record_property.cpp
)

if(WIN32)
  set(PROJECT_NAME_FULL "WebSocket Connection")
  set(PROJECT_DESCRIPTION "WebSocket connection library for SensCord")
  configure_file(
      ${TEMPLATE_WINDOWS_DLL_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND PROJECT_SOURCES
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

# Create target.
add_library(${TARGET_NAME} SHARED ${PROJECT_SOURCES})

# Add lib/core include directory.
target_include_directories(
    ${TARGET_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/../../../thirdparty
)

# Link libraries.
target_link_libraries(${TARGET_NAME} PRIVATE senscord ${COMMON_GCOV_LIBRARY})

target_compile_options(${TARGET_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

# C++11
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 11)

# Set target folder name
set_target_folder(${TARGET_NAME} ${TARGET_NAME})

# Create output destination directory
make_output_dir_prebuild(${TARGET_NAME} ${SSDK_CONNECTION_OUT_DIR})

# Generate config.
add_custom_target(
    ${TARGET_NAME}-config ALL
    DEPENDS Generate-config-connection ${TARGET_NAME}
    COMMAND ${CMAKE_COMMAND}
        -DTARGET_FILE=${SSDK_CONFIG_OUT_DIR}/senscord_connections.xml
        -P ${PROJECT_SOURCE_DIR}/replace_config.cmake)

### Install files.
# library
install_connection(${TARGET_NAME})
