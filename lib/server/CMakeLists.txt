# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(server VERSION ${SensCord_VERSION})

# Name of the build target
set(TARGET_NAME ${PROJECT_NAME})

# Setting Build Options.
## add -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set output directory.
set_ssdk_archive_output_dir(${SSDK_LIBRARY_OUT_DIR})

# Sources
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/server.cpp
    ${PROJECT_SOURCE_DIR}/src/core_clientless.cpp
    ${PROJECT_SOURCE_DIR}/src/clientless_core_behavior.cpp
    ${PROJECT_SOURCE_DIR}/src/client_listener.cpp
    ${PROJECT_SOURCE_DIR}/src/client_adapter_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/client_adapter.cpp
    ${PROJECT_SOURCE_DIR}/src/stream_adapter.cpp
    ${PROJECT_SOURCE_DIR}/src/config_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/secondary_client_listener.cpp
    ${PROJECT_SOURCE_DIR}/src/secondary_client_adapter.cpp
)

# Create target.
add_library(${TARGET_NAME} STATIC ${PROJECT_SOURCES})

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
# TODO: private header access
target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/../core)

# Link libraries.
target_link_libraries(${TARGET_NAME} PRIVATE ${SSDK_CORE_NAME} ${COMMON_GCOV_LIBRARY})

target_compile_options(${TARGET_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

# Set target folder name
set_target_folder(${TARGET_NAME} ${TARGET_NAME})

# Create output destination directory
make_output_dir_prebuild(${TARGET_NAME} ${SSDK_LIBRARY_OUT_DIR})

# Copy Config file.
add_configure_file(${TARGET_NAME}
                   ${PROJECT_SOURCE_DIR}/config/server.xml
                   ${SSDK_CONFIG_OUT_DIR})

# Install files.
## library
install_static_libs(${TARGET_NAME})

## config
if(SENSCORD_INSTALL_CONFIG)
  install(FILES       ${PROJECT_SOURCE_DIR}/config/server.xml
          DESTINATION ${SSDK_CONIFIG_INSTALL_DIR})
endif()

## Public header.
if(EXISTS ${PROJECT_SOURCE_DIR}/include)
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${PROJECT_SOURCE_DIR}/include/
          ${SSDK_BUILD_OUTPUT_INCLUDE_DIR})

  install(DIRECTORY
      ${PROJECT_SOURCE_DIR}/include/
      DESTINATION include)
endif()
