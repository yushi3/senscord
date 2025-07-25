# SPDX-FileCopyrightText: 2020-2025 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

set(LIBXML2_ARCHIVE libxml2_android-security-10.0.0_r75.tar.gz)
set(LIBXML2_INPUT_DIR ${SSDK_TOP_DIR}/thirdparty/libxml2/android)
set(LIBXML2_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libxml2)
set(LIBXML2_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/libxml2)

function(add_external_libxml2 TARGET)
  if(NOT EXISTS ${LIBXML2_BUILD_DIR})
    file(MAKE_DIRECTORY ${LIBXML2_BUILD_DIR})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${LIBXML2_INPUT_DIR}/${LIBXML2_ARCHIVE}
        WORKING_DIRECTORY ${LIBXML2_BUILD_DIR}
    )
  endif()
  configure_file(
      ${LIBXML2_INPUT_DIR}/CMakeLists.txt.in
      ${LIBXML2_OUTPUT_DIR}/CMakeLists.txt
      @ONLY)
  add_subdirectory(${LIBXML2_OUTPUT_DIR})
endfunction()
