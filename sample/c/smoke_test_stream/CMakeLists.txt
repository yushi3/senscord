# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(SensCordSmokeTestStreamPlayerC VERSION ${SensCord_VERSION}
        LANGUAGES C)

# set target name
set(TARGET_NAME ${PROJECT_NAME})

set(TARGET_SOURCES
    ${PROJECT_SOURCE_DIR}/SensCordSmokeTestStream.c
)

if(WIN32)
  set(PROJECT_NAME_FULL "SensCord Smoke Test Player")
  set(PROJECT_DESCRIPTION "Sample smoke test player application for SensCord")
  configure_file(
      ${TEMPLATE_WINDOWS_APP_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND TARGET_SOURCES
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

add_executable(${TARGET_NAME} ${TARGET_SOURCES})

# Pass the path to the directory containing the Component header.
target_include_directories(${TARGET_NAME} PRIVATE ${SSDK_COMPONENT_HEADER_DIR})

if (WIN32)
  string(REPLACE "/" "\\" srouce_path "${PROJECT_SOURCE_DIR}")
  string(REPLACE "/" "\\" target_path "${SSDK_BUILD_OUTPUT_DIR}")

  # Copy the debug script for Windows
  add_custom_command(TARGET  ${TARGET_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy
                             ${srouce_path}\\${TARGET_NAME}.bat
                             ${target_path}\\${TARGET_NAME}.bat)

  # Set target folder name
  set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${FOLDER_NAME})
endif()

# Linking senscord project
target_link_libraries(${TARGET_NAME} senscord)

# Add build dependency
add_dependencies(${TARGET_NAME} component_pseudo_image)

# Install files.
install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION ${SSDK_BIN_INSTALL_DIR})

# Create output destination of executable file.
make_output_dir_prebuild(${TARGET_NAME} ${SSDK_BINARY_OUT_DIR})
