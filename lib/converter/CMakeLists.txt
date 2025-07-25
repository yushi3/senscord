# SPDX-FileCopyrightText: 2020-2021 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# Change the output destination directory of the library.
set_ssdk_library_output_dir(${SSDK_CONVERTER_OUT_DIR})

# Setting the folder name displayed in the IDE
set(FOLDER_NAME "converter")

if(SENSCORD_API_C)
  add_subdirectory(core_converter)
endif()

