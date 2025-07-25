# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# Change the output destination directory of the file.
set_ssdk_runtime_output_dir(${SSDK_BINARY_OUT_DIR})

# Setting the folder name displayed in the IDE
function(set_target_folder TARGET_NAME)
  set(FOLDER_NAME "sample/cpp")
  set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${FOLDER_NAME})
endfunction()

# Add build directory.
add_subdirectory(simple_stream)
add_subdirectory(smoke_test_stream)

if(SENSCORD_RECORDER)
  add_subdirectory(recorder)
endif()

# Build only when using opencv component.
if(SENSCORD_COMPONENT_OPENCV)
  add_subdirectory(stream_monitor)
endif()
