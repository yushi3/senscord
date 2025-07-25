# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(uds_connection VERSION ${SensCord_VERSION})

# Name of the build target
set(TARGET_NAME ${PROJECT_NAME})

# Setting Build Options.
## add -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Change the output directory.
set_ssdk_library_output_dir(${SSDK_CONNECTION_OUT_DIR})

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Sources
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/uds_connection.cpp
)

# Create target.
add_library(${TARGET_NAME} SHARED ${PROJECT_SOURCES})

# Link libraries.
target_link_libraries(${TARGET_NAME} PRIVATE senscord ${COMMON_GCOV_LIBRARY})

target_compile_options(${TARGET_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

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

# Install files.
install_connection(${TARGET_NAME})

if(SENSCORD_SERVER_ROS)
  # Static library
  set_ssdk_archive_output_dir(${SSDK_LIBRARY_OUT_DIR})
  add_library(${TARGET_NAME}_static STATIC ${PROJECT_SOURCES})
  target_link_libraries(${TARGET_NAME}_static PRIVATE senscord ${COMMON_GCOV_LIBRARY})
  make_output_dir_prebuild(${TARGET_NAME}_static ${SSDK_LIBRARY_OUT_DIR})
  install_static_libs(${TARGET_NAME}_static)

  ## Public header.
  if(EXISTS ${PROJECT_SOURCE_DIR}/include)
    add_custom_command(TARGET ${TARGET_NAME}_static POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/include/
            ${SSDK_BUILD_OUTPUT_INCLUDE_DIR})

    install(DIRECTORY
        ${PROJECT_SOURCE_DIR}/include/
        DESTINATION include)
  endif()
endif()
