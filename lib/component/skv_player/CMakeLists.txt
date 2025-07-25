# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

set(PROJECT_NAME component_skv_player)
set(OUTPUT_INCLUDE_DIR skv_player)

project(${PROJECT_NAME} VERSION ${SensCord_VERSION})

# target name
set(TARGET_NAME ${PROJECT_NAME})

# compile option
if(WIN32)
  # C++14 build
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /std:c++14")
else()
  # C++11 build
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

### Sources
# Add source codes.
set(PROJECT_SOURCE
    # Common functions
    src/skv_player_source.cpp
    src/skv_player_component.cpp
    src/skv_play_library.cpp
    src/skv_player_util.cpp
    # Playback function for SensCord recordings
    src/skv_player_senscord_file_manager.cpp
    src/senscord_depth_channel_accessor.cpp
    src/senscord_confidence_channel_accessor.cpp
    src/senscord_pointcloud_channel_accessor.cpp
    src/senscord_rawdata_channel_accessor.cpp
)

if(WIN32)
  set(PROJECT_NAME_FULL "SKV Player Component")
  set(PROJECT_DESCRIPTION "SKV Player component library for SensCord")
  configure_file(
      ${TEMPLATE_WINDOWS_DLL_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND PROJECT_SOURCE
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

# Create library target.
add_library(${TARGET_NAME} SHARED ${PROJECT_SOURCE})

target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR})

if (WIN32)
  # Set target folder name
  set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${FOLDER_NAME})
endif()

target_link_libraries(${TARGET_NAME} senscord skv)

target_compile_options(${TARGET_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

# Create output destination directory
add_custom_command(TARGET  ${TARGET_NAME} PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E make_directory
                           ${SSDK_COMPONENT_OUT_DIR})

install_component(${TARGET_NAME}
                  ${PROJECT_SOURCE_DIR}/config/component_skv_player.xml)

### Copy Config file.
add_configure_file(${TARGET_NAME}
                   ${PROJECT_SOURCE_DIR}/config/component_skv_player.xml
                   ${SSDK_COMPONENT_OUT_DIR})

