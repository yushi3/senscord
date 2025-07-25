# SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# Change the output directory.
set_ssdk_library_output_dir(${SSDK_RECORDER_OUT_DIR})

# Setting the folder name displayed in the IDE
set(FOLDER_NAME "recorder")

# Subdirectories.
if(SENSCORD_RECORDER_BIN)
  add_subdirectory(bin)
endif()

if(SENSCORD_RECORDER_PNM)
  add_subdirectory(pnm)
endif()
