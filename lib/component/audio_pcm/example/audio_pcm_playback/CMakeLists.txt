# SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

# Change the output destination directory of the file.
set_ssdk_runtime_output_dir(${SSDK_BINARY_OUT_DIR})

# Project
project(SensCordAudioPcmPlayback CXX)

set(PROJECT_SOURCES
    main.cpp
    audio_pcm_playback.cpp
)

if(UNIX)
  find_package(ALSA)
  if(NOT ALSA_FOUND)
    message(FATAL_ERROR "Please install libasound-dev")
  endif()

  list(APPEND PROJECT_SOURCES
    audio_pcm_playback_alsa.cpp
  )
endif()

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} senscord)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})

if(UNIX)
  target_link_libraries(${PROJECT_NAME} ${ALSA_LIBRARIES})
  target_include_directories(${PROJECT_NAME} PRIVATE ${ALSA_INCLUDE_DIRS})
endif()

# Install files.
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${SSDK_BIN_INSTALL_DIR})

make_output_dir_prebuild(${PROJECT_NAME} ${SSDK_BINARY_OUT_DIR})
