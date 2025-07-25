# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(bin_recorder VERSION ${SensCord_VERSION})

# target name
set(TARGET_NAME ${PROJECT_NAME})

set(PROJECT_SOURCES
    src/bin_recorder.cpp
    src/bin_channel_recorder.cpp
    src/registration.cpp
)

if(WIN32)
  set(PROJECT_NAME_FULL "BIN Recorder")
  set(PROJECT_DESCRIPTION "BIN recorder library for SensCord")
  configure_file(
      ${TEMPLATE_WINDOWS_DLL_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND PROJECT_SOURCES
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

# Create library target.
add_library(${TARGET_NAME} SHARED ${PROJECT_SOURCES})

# Include directory.
target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR})

# Set target folder name
if (WIN32)
  set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${FOLDER_NAME})
endif()

target_link_libraries(${TARGET_NAME} senscord)

target_compile_options(${TARGET_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

# Create output destination directory
add_custom_command(TARGET  ${TARGET_NAME} PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E make_directory
                           ${SSDK_RECORDER_OUT_DIR})

install_recorder(${TARGET_NAME})
