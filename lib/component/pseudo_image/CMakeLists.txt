# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

set(PROJECT_NAME component_pseudo_image)
set(OUTPUT_INCLUDE_DIR pseudo_image)

project(${PROJECT_NAME} VERSION ${SensCord_VERSION})

# target name
set(TARGET_NAME ${PROJECT_NAME})

set(PROJECT_SOURCES
    src/pseudo_image_source.cpp
    src/pseudo_image_component.cpp
)

if(WIN32)
  set(PROJECT_NAME_FULL "Pseudo Image Component")
  set(PROJECT_DESCRIPTION "Pseudo image component library for SensCord")
  configure_file(
      ${TEMPLATE_WINDOWS_DLL_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND PROJECT_SOURCES
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

add_library(${TARGET_NAME} SHARED ${PROJECT_SOURCES})

# Include directory.
target_include_directories(${TARGET_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include)

if (WIN32)
  # Set target folder name
  set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${FOLDER_NAME})
endif()

target_link_libraries(${TARGET_NAME} senscord)

target_compile_options(${TARGET_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

# Create output destination directory
add_custom_command(TARGET  ${TARGET_NAME} PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E make_directory
                           ${SSDK_COMPONENT_OUT_DIR})

install_component(${TARGET_NAME}
                  ${PROJECT_SOURCE_DIR}/config/component_pseudo_image.xml)

### Copy Config file.
add_configure_file(${TARGET_NAME}
                   ${PROJECT_SOURCE_DIR}/config/component_pseudo_image.xml
                   ${SSDK_COMPONENT_OUT_DIR})

### Public header function.
add_public_header(${TARGET_NAME}
                  ${PROJECT_SOURCE_DIR}/include
                  ${OUTPUT_INCLUDE_DIR})
