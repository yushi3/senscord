# SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

# Project
set(PROJECT_NAME component_audio_pcm)
project(${PROJECT_NAME} VERSION ${SensCord_VERSION})

set(PROJECT_SOURCES
    src/audio_pcm_component.cpp
    src/audio_pcm_source.cpp
    src/memory_pool.cpp
)

if(UNIX)
  find_package(ALSA)
  if(NOT ALSA_FOUND)
    message(FATAL_ERROR "Please install libasound-dev")
  endif()

  list(APPEND PROJECT_SOURCES
      src/audio_capture_alsa.cpp
  )
endif()

if(WIN32)
  set(PROJECT_NAME_FULL "Audio PCM Component")
  set(PROJECT_DESCRIPTION "Audio PCM component library for SensCord")
  configure_file(
      ${TEMPLATE_WINDOWS_DLL_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND PROJECT_SOURCES
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} senscord senscord_osal)

# Include directory
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
)

if(UNIX)
  target_link_libraries(${PROJECT_NAME} ${ALSA_LIBRARIES})
  target_include_directories(${PROJECT_NAME} PRIVATE ${ALSA_INCLUDE_DIRS})
endif()

if(SENSCORD_SAMPLE)
  add_subdirectory(example)
endif()

# Install
if(WIN32)
  # for Windows
  install(TARGETS ${PROJECT_NAME}
          RUNTIME DESTINATION bin/component)
  install(FILES config/component_audio_pcm.xml
          DESTINATION bin/component)
else()
  # for Linux
  install(TARGETS ${PROJECT_NAME}
          LIBRARY DESTINATION lib/senscord/component)
  install(FILES config/component_audio_pcm.xml
          DESTINATION lib/senscord/component)
endif()

# Copy Config file.
add_configure_file(
    ${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/config/component_audio_pcm.xml
    ${SSDK_COMPONENT_OUT_DIR})
