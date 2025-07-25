# SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# Change the output destination directory of the library.
set_ssdk_library_output_dir(${SSDK_ALLOCATOR_OUT_DIR})

# Setting the folder name displayed in the IDE
set(FOLDER_NAME "allocator")

if(SENSCORD_ALLOCATOR_SAMPLE)
  # Add sample allocator
  add_subdirectory(allocator_sample)
endif()

