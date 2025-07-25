# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# Extract file name from path
get_filename_component(file_name "${SOURCE_FILE}" NAME)
# Create output destination file path
set(destination_path ${DEST_DIR}/${file_name})
# If the file does not exist
if (NOT EXISTS ${destination_path})
  # Register process to execute
  execute_process(COMMAND  ${CMAKE_COMMAND} -E copy
                  ${SOURCE_FILE} ${DEST_DIR})
endif()
