# SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(SynchronizerSamplePolling VERSION ${SensCord_VERSION})

# set executable file name
set(TARGET_NAME ${PROJECT_NAME})

set(TARGET_SOURCES
    ${PROJECT_SOURCE_DIR}/sample_main.cpp
)

if(WIN32)
  set(PROJECT_NAME_FULL "SensCord Synchronizer polling")
  set(PROJECT_DESCRIPTION "Sample synchronizer polling application")
  configure_file(
      ${TEMPLATE_WINDOWS_APP_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND TARGET_SOURCES
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

add_executable(${TARGET_NAME} ${TARGET_SOURCES})

target_include_directories(${TARGET_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${SYNCHRONIZER_INCLUDE_DIR}
  ${senscord_INCLUDE_DIR}
  ../
)

target_link_libraries(${TARGET_NAME} PRIVATE senscord synchronizer
                                            ${COMMON_GCOV_LIBRARY})

# this sample requires C++11 support
# target_compile_features(${TARGET_NAME} PUBLIC cxx_std_11)

# add synchronizer lib to IDE project tree.
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${SYNCHRONIZER_FOLDER_NAME})

# install config
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION ${UTILITY_BIN_INSTALL_DIR})

# Other. Copy the playing bat file.
if(WIN32)
  # Setting the output destination directory
  string(REPLACE "/" "\\" srouce_path "${PROJECT_SOURCE_DIR}")
  string(REPLACE "/" "\\" target_path "${SSDK_BUILD_OUTPUT_DIR}")

  # Copy the debug script for Windows
  add_custom_command(
    TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${srouce_path}\\${TARGET_NAME}.bat
            ${target_path}\\${TARGET_NAME}.bat)
endif()
