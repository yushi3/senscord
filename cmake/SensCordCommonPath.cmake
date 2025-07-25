# SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################
################################################################################
#  Common variables
################################################################################

# Set the name of core
set(SSDK_CORE_NAME senscord)

# Set top directory variable
set(SSDK_TOP_DIR ${PROJECT_SOURCE_DIR})

# Module path
set(SSDK_MODULE_DIR ${SSDK_TOP_DIR}/cmake)

# set include directory path
set(SSDK_INCLUDE_DIR ${SSDK_TOP_DIR}/include)

# Setting the output destination directory
set(SSDK_BUILD_OUTPUT_DIR ${CMAKE_BINARY_DIR}/output)

# Output include directory.
set(SSDK_BUILD_OUTPUT_INCLUDE_DIR ${SSDK_BUILD_OUTPUT_DIR}/include)

# Destination directory.
set(SSDK_BINARY_OUT_DIR ${SSDK_BUILD_OUTPUT_DIR}/bin)

# Config destination directory.
set(SSDK_CONFIG_OUT_DIR ${SSDK_BUILD_OUTPUT_DIR}/config)

if (WIN32)
  # Core destination directory.
  set(SSDK_LIBRARY_OUT_DIR ${SSDK_BUILD_OUTPUT_DIR}/bin)
else()
  # Core destination directory.
  set(SSDK_LIBRARY_OUT_DIR ${SSDK_BUILD_OUTPUT_DIR}/lib)
endif()

# Component destination directory.
set(SSDK_COMPONENT_OUT_DIR ${SSDK_LIBRARY_OUT_DIR}/component)

# Allocator estination directory.
set(SSDK_ALLOCATOR_OUT_DIR ${SSDK_LIBRARY_OUT_DIR}/allocator)

# Recorder destination directory.
set(SSDK_RECORDER_OUT_DIR ${SSDK_LIBRARY_OUT_DIR}/recorder)

# Connection destination directory.
set(SSDK_CONNECTION_OUT_DIR ${SSDK_LIBRARY_OUT_DIR}/connection)

# Converter destination directory.
set(SSDK_CONVERTER_OUT_DIR ${SSDK_LIBRARY_OUT_DIR}/converter)

# Component header reference.
set(SSDK_COMPONENT_HEADER_DIR ${SSDK_BUILD_OUTPUT_DIR}/include)

# Header destination directory.
set(SSDK_HEADER_OUT_DIR ${SSDK_COMPONENT_HEADER_DIR}/${SSDK_CORE_NAME})

# Python module output directory.
set(SSDK_PYTHON_OUT_DIR ${SSDK_BUILD_OUTPUT_DIR}/python)

# Utility destination directory
set(SSDK_UTIL_BINARY_OUT_DIR ${SSDK_BINARY_OUT_DIR}/utility)
set(SSDK_UTIL_LIBRARY_OUT_DIR ${SSDK_LIBRARY_OUT_DIR}/utility)
set(SSDK_UTIL_TEST_OUT_DIR ${SSDK_TEST_OUT_DIR}/utility)

