# SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

set(PROJECT_NAME senscord_core_converter)
project(${PROJECT_NAME} VERSION ${SensCord_VERSION})

# Setting Build Options.
## add -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Change the output directory.
set_ssdk_library_output_dir(${SSDK_CONVERTER_OUT_DIR})

# Sources.
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/core_converter.cpp
)

if(WIN32)
  set(PROJECT_NAME_FULL "SensCord Core Converter")
  set(PROJECT_DESCRIPTION "SensCord Core converter library")
  configure_file(
      ${TEMPLATE_WINDOWS_DLL_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND PROJECT_SOURCES
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

# Create target.
add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

# Include directories.
target_include_directories(
    ${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} senscord)

target_compile_options(${PROJECT_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

# Generate config.
add_custom_target(
    ${PROJECT_NAME}-config ALL
    DEPENDS senscord ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND}
        -DTARGET_FILE=${SSDK_CONFIG_OUT_DIR}/senscord.xml
        -P ${PROJECT_SOURCE_DIR}/replace_config.cmake)

# Output directory
make_output_dir_prebuild(${PROJECT_NAME} ${SSDK_CONVERTER_OUT_DIR})

if(EXISTS ${PROJECT_SOURCE_DIR}/include)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${PROJECT_SOURCE_DIR}/include/
          ${SSDK_BUILD_OUTPUT_DIR}/include)
endif()

# Install.
## library.
if(WIN32)
  ## for Windows
  install(TARGETS ${PROJECT_NAME}
          RUNTIME DESTINATION bin/converter)
else()
  ## for Linux
  install(TARGETS ${PROJECT_NAME}
          LIBRARY DESTINATION lib/senscord/converter)
endif()

## Public header.
if(EXISTS ${PROJECT_SOURCE_DIR}/include)
  install(DIRECTORY
      ${PROJECT_SOURCE_DIR}/include/
      DESTINATION include)
endif()
