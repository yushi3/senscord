# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

set(PROJECT_NAME allocator_sample)
project(${PROJECT_NAME} VERSION ${SensCord_VERSION})

# target name
set(TARGET_NAME ${PROJECT_NAME})

set(TARGET_SOURCES
    src/memory_allocator_sample.cpp
)

if(WIN32)
  set(PROJECT_NAME_FULL "Sample Allocator")
  set(PROJECT_DESCRIPTION "Sample memory allocator library for SensCord")
  configure_file(
      ${TEMPLATE_WINDOWS_DLL_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND TARGET_SOURCES
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

add_library(${TARGET_NAME} SHARED ${TARGET_SOURCES})

if (WIN32)
  # Set target folder name
  set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${FOLDER_NAME})
endif()

target_link_libraries(${TARGET_NAME} senscord)

target_compile_options(${TARGET_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

# Create output destination directory
add_custom_command(TARGET  ${TARGET_NAME} PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E make_directory
                           ${SSDK_ALLOCATOR_OUT_DIR})

install_allocator(${TARGET_NAME})
