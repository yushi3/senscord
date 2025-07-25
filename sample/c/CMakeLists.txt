# SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# Change the output destination directory of the file.
set_ssdk_runtime_output_dir(${SSDK_BINARY_OUT_DIR})

# Setting the folder name displayed in the IDE
set(FOLDER_NAME "sample/c")

# Add build directory.
add_subdirectory(simple_stream)
add_subdirectory(smoke_test_stream)

if(SENSCORD_RECORDER)
  add_subdirectory(recorder)
endif()
