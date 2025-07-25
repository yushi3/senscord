# SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# make all: Copy to the output directory.
add_custom_target(senscord_py ALL)

set(OUTPUT_PYTHON_PACKAGE_DIR "${SSDK_PYTHON_OUT_DIR}/senscord")

# Make 'senscord' and 'senscord.external' package directory.
add_custom_command(
  TARGET senscord_py PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_PYTHON_PACKAGE_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_PYTHON_PACKAGE_DIR}/external)

# File copy function.
function(copy_python_module INPUT_FILE OUTPUT_DIR)
  add_custom_command(
      TARGET senscord_py POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${INPUT_FILE} ${OUTPUT_DIR})
endfunction()

# Copy senscord/*.py
file(GLOB PYTHON_MODULES "${PROJECT_SOURCE_DIR}/python/senscord/*.py")
foreach(PYTHON_MODULE ${PYTHON_MODULES})
  copy_python_module(${PYTHON_MODULE} ${OUTPUT_PYTHON_PACKAGE_DIR})
endforeach()

# Copy senscord/external/*.py
file(GLOB PYTHON_MODULES "${PROJECT_SOURCE_DIR}/python/senscord/external/*.py")
foreach(PYTHON_MODULE ${PYTHON_MODULES})
 copy_python_module(${PYTHON_MODULE} "${OUTPUT_PYTHON_PACKAGE_DIR}/external/")
endforeach()

# Copy setup.py
copy_python_module("${PROJECT_SOURCE_DIR}/python/setup.py" ${SSDK_PYTHON_OUT_DIR})

# Set target folder name
set_target_folder(senscord_py python)

# Install (copy from 'output/python' to '<install_prefix>/python')
install(DIRECTORY ${SSDK_PYTHON_OUT_DIR} DESTINATION .)
