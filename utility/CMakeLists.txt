# SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

if(WIN32)
  # Add __declspec(dllimport) for all global symbols
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS on)
endif()

# set project directory for IDE
set(UTILITY_FOLDER_NAME "utility")

# Add Build target Build synchronizer library.
if(SENSCORD_SYNCHRONIZER)
  add_subdirectory(synchronizer)
endif()
