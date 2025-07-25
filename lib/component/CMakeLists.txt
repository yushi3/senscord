# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# Change the output destination directory of the library.
set_ssdk_library_output_dir(${SSDK_COMPONENT_OUT_DIR})

# Setting the folder name displayed in the IDE
set(FOLDER_NAME "component")

if(SENSCORD_SERVER)
  # Connection client component.
  add_subdirectory(client)
endif()

# Component of player
if(SENSCORD_PLAYER_RAW)
  add_subdirectory(player)
endif()

if(SENSCORD_COMPONENT_PSEUDO)
  add_subdirectory(pseudo_image)
endif()

if(SENSCORD_COMPONENT_V4L2)
  add_subdirectory(v4l2_image)
endif()

# Component of skv_player
if(SENSCORD_PLAYER_SKV)
  add_subdirectory(skv_player)
endif()

if(SENSCORD_COMPONENT_OPENCV)
  add_subdirectory(opencv_image)
endif()

if(SENSCORD_COMPONENT_AUDIO_PCM)
  add_subdirectory(audio_pcm)
endif()
