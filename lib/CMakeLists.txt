# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

if (WIN32)
  # Add __declspec(dllimport) for all global symbols
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS on)
endif()

### Add Build target
# Build senscord library.
add_subdirectory(core)

# Build osal library
add_subdirectory(osal)

if(SENSCORD_SERVER_ROS)
  # Build server library.
  add_subdirectory(server)
endif()

if(SENSCORD_SERVER)
  # Build connection libraries.
  add_subdirectory(connection)
endif()

# Build senscord library.
add_subdirectory(component)
add_subdirectory(allocator)

if(SENSCORD_RECORDER_LOADER)
  add_subdirectory(recorder)
endif()

add_subdirectory(converter)

if(SENSCORD_API_PYTHON)
  add_subdirectory(python)
endif()
if(SENSCORD_API_WASM)
  add_subdirectory(wamr)
endif()
