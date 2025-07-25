# SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

################################################################################
# Find Skv library.
################################################################################

# initialized variables.
set(SKV_INCLUDE_DIRS)

set(SKV_LIBRARIES)
set(SKV_LIBRARY_DIRS)
set(SKV_LIB_NAME)

set(H5LZ4_LIB_NAME)
set(H5LZ4_LIBRARIES)

set(SKV_TOP_DIR ${SSDK_TOP_DIR}/thirdparty/skvlibrary)

# Check Skv top directory.
if (NOT EXISTS ${SKV_TOP_DIR})
    message(WARNING "Skv top directory not found.")
endif()

if(WIN32)
  # Set Directory
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(SKV_LIB_DIR Windows_x86_64)
  else()
    set(SKV_LIB_DIR Windows_x86)
  endif()
  # Set File Name
  set(SKV_IMPLIB_NAME skv.lib)
  set(SKV_LIB_NAME skv.dll)
  set(H5LZ4_LIB_NAME h5lz4.dll)
else()
  # Set Directory
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(SKV_LIB_DIR Linux_x86_64)
  else()
    set(SKV_LIB_DIR Linux_x86)
  endif()
  # Set File Name
  set(SKV_IMPLIB_NAME libskv.so)
  set(SKV_LIB_NAME libskv.so)
  set(H5LZ4_LIB_NAME libh5lz4.so)
endif()

set(SKV_INCLUDE_DIRS ${SKV_TOP_DIR}/include)
set(SKV_LIBRARY_DIRS ${SKV_TOP_DIR}/${SKV_LIB_DIR})
set(SKV_LIBRARIES ${SKV_LIBRARY_DIRS}/${SKV_LIB_NAME})
set(SKV_IMPORT_LIBRARIES ${SKV_LIBRARY_DIRS}/${SKV_IMPLIB_NAME})
set(H5LZ4_LIBRARIES ${SKV_LIBRARY_DIRS}/${H5LZ4_LIB_NAME})
