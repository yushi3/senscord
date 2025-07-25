# SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# make all: Copy to the output directory.
add_custom_target(PythonSample ALL)

# Make 'output/python' directory.
add_custom_command(
    TARGET PythonSample PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SSDK_PYTHON_OUT_DIR})

# Copy sample/python/*.py script
file(GLOB SCRIPT_FILES ${PROJECT_SOURCE_DIR}/sample/python/*.py)
add_custom_command(
    TARGET PythonSample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${SCRIPT_FILES} ${SSDK_PYTHON_OUT_DIR})

# Set target folder name
set_target_folder(PythonSample sample/python)
