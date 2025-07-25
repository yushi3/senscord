# SPDX-FileCopyrightText: 2021-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

set(PROJECT_NAME component_opencv_image)
project(${PROJECT_NAME} VERSION ${SensCord_VERSION})

set(PROJECT_SOURCES
    src/opencv_image_source.cpp
    src/opencv_image_component.cpp
)

if(WIN32)
  set(PROJECT_NAME_FULL "OpenCV Image Component")
  set(PROJECT_DESCRIPTION "OpenCV image component library for SensCord")
  configure_file(
      ${TEMPLATE_WINDOWS_DLL_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND PROJECT_SOURCES
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

find_package(OpenCV REQUIRED)
#message(STATUS "OpenCV_INCLUDE_DIRS=${OpenCV_INCLUDE_DIRS}")
#message(STATUS "OpenCV_LIBS=${OpenCV_LIBS}")

# Windows:
#   Download OpenCV release package: `opencv-*.*.*-vc14_vc15.exe`
#   set OpenCV_DIR
#
# Ubuntu:
#   sudo apt install libopencv-dev
#     or
#   set OpenCV_DIR

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    senscord
    ${OpenCV_LIBS}
)

target_compile_options(${PROJECT_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

if(WIN32)
  # Set target folder name
  set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
endif()

# Create output destination directory
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SSDK_COMPONENT_OUT_DIR})

# Copy Config file.
add_configure_file(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/config/component_opencv_image.xml
    ${SSDK_COMPONENT_OUT_DIR})

install_component(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/config/component_opencv_image.xml)
