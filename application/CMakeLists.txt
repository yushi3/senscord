# SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# Change the output destination directory of the file.
set_ssdk_runtime_output_dir(${SSDK_BINARY_OUT_DIR})

# Setting the folder name displayed in the IDE
function(set_target_folder TARGET_NAME)
  set(FOLDER_NAME "application")
  set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${FOLDER_NAME})
endfunction()

if(SENSCORD_SERVER)
  add_subdirectory(multi_server)
endif()
if(SENSCORD_SERVER_ROS)
  add_subdirectory(tcp_server)
  if(UNIX)
    add_subdirectory(uds_server)
  endif()
endif()
