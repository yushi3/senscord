# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(SensCordStreamMonitor VERSION ${SensCord_VERSION})

# set target name
set(TARGET_NAME ${PROJECT_NAME})

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Check find package result
if(OpenCV_FOUND)
    message(STATUS "Found OpenCV libraries -> ${OpenCV_LIBS}")
else()
    message(FATAL_ERROR
    "OpenCV Library is not found." \n
    "Please set the installation path for OpenCV in OpenCV_DIR.")
endif()

set(TARGET_SOURCES
    ${PROJECT_SOURCE_DIR}/SensCordStreamMonitor.cpp
)

if(WIN32)
  set(PROJECT_NAME_FULL "SensCord Stream Monitor")
  set(PROJECT_DESCRIPTION "Sample stream monitor application for SensCord")
  configure_file(
      ${TEMPLATE_WINDOWS_APP_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND TARGET_SOURCES
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

add_executable(${TARGET_NAME} ${TARGET_SOURCES})

### Setup include pathes.
# Pass the path to the directory containing the Component header.
target_include_directories(${TARGET_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})

### Setup link libraries.
# Linking senscord, OpenCV
target_link_libraries(${TARGET_NAME} senscord ${OpenCV_LIBS})

target_compile_options(${TARGET_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

# Set target folder
set_target_folder(${TARGET_NAME})

# Install files.
install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION ${SSDK_BIN_INSTALL_DIR})

# Create output destination of executable file.
make_output_dir_prebuild(${TARGET_NAME} ${SSDK_BINARY_OUT_DIR})

### Other.
# Copy the playing bat file.
if (WIN32)
string(REPLACE "/" "\\" srouce_path "${PROJECT_SOURCE_DIR}")
string(REPLACE "/" "\\" target_path "${SSDK_BUILD_OUTPUT_DIR}")

# Copy the debug script for Windows
add_custom_command(TARGET  ${TARGET_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                           ${srouce_path}\\${TARGET_NAME}.bat
                           ${target_path}\\${TARGET_NAME}.bat)
endif()
