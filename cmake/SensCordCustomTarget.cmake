# SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################
################################################################################
#  Custom targets
################################################################################
# Clean output directory.
add_custom_target(clean_output
                  COMMAND ${CMAKE_COMMAND}
                          -E remove_directory ${SSDK_BUILD_OUTPUT_DIR})

# Target to enabled all options.
add_custom_target(option_all_enabled
                  COMMAND ${CMAKE_COMMAND}
                          -DCMAKELISTS_DIR="${SSDK_TOP_DIR}"
                          -DOPTION_SWITCH=ON
                          -P ${SSDK_MODULE_DIR}/SwitchAllOptions.cmake)

# Target to diabled all options.
add_custom_target(option_all_disabled
                  COMMAND ${CMAKE_COMMAND}
                          -DCMAKELISTS_DIR="${SSDK_TOP_DIR}"
                          -DOPTION_SWITCH=OFF
                          -P ${SSDK_MODULE_DIR}/SwitchAllOptions.cmake)

if(UNIX)
  add_custom_target(clean_all
                    COMMAND ${CMAKE_BUILD_TOOL} clean
                    COMMAND ${CMAKE_BUILD_TOOL} clean_output)
endif()

# Running cpplint
add_custom_target(cpplint-all)

