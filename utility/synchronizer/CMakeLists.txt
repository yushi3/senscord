# SPDX-FileCopyrightText: 2020-2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(SynchronizerUtility)

# set project directory name for IDE.
set(SYNCHRONIZER_FOLDER_NAME "${UTILITY_FOLDER_NAME}/synchronizer")

# include & headers
set(SYNCHRONIZER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/synchronizer/include/)

# binaly output location
set_ssdk_runtime_output_dir("${SSDK_UTIL_BINARY_OUT_DIR}")
set_ssdk_library_output_dir("${SSDK_UTIL_LIBRARY_OUT_DIR}")
set_ssdk_archive_output_dir("${SSDK_UTIL_LIBRARY_OUT_DIR}")

# C++11 support is required to compile/use Synchronizer
if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

################################################################################
#  Add sub directories
################################################################################

# library
add_subdirectory(synchronizer)

# sample
if(SENSCORD_SYNCHRONIZER_SAMPLE)
  add_subdirectory(sample)
endif()

