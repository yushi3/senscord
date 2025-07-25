# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(SensCordServer VERSION ${SensCord_VERSION})

# set target name
set(TARGET_NAME ${PROJECT_NAME})

# Add sources
set(SERVER_SOURCES
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/multi_server.cpp
    ${PROJECT_SOURCE_DIR}/src/core_clientless.cpp
    ${PROJECT_SOURCE_DIR}/src/clientless_core_behavior.cpp
    ${PROJECT_SOURCE_DIR}/src/client_listener.cpp
    ${PROJECT_SOURCE_DIR}/src/client_adapter_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/client_adapter.cpp
    ${PROJECT_SOURCE_DIR}/src/stream_adapter.cpp
    ${PROJECT_SOURCE_DIR}/src/publisher_adapter.cpp
    ${PROJECT_SOURCE_DIR}/src/config_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/secondary_client_listener.cpp
    ${PROJECT_SOURCE_DIR}/src/secondary_client_adapter.cpp
)

if(WIN32)
  set(PROJECT_NAME_FULL "SensCord Server")
  set(PROJECT_DESCRIPTION "Server application for SensCord")
  configure_file(
      ${TEMPLATE_WINDOWS_APP_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND SERVER_SOURCES
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

add_executable(${TARGET_NAME} ${SERVER_SOURCES})

# Headers with servers
target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
# TODO: private header access
target_include_directories(${TARGET_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/../../lib/core
)

# Linking senscord project
target_link_libraries(${TARGET_NAME} senscord)

target_compile_options(${TARGET_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

# Set target folder
set_target_folder(${TARGET_NAME})

# Copy Config file.
add_configure_file(${TARGET_NAME}
                   ${PROJECT_SOURCE_DIR}/config/senscord_server.xml
                   ${SSDK_CONFIG_OUT_DIR})

# Install files.
install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION ${SSDK_BIN_INSTALL_DIR})

if(SENSCORD_INSTALL_CONFIG)
  install(FILES       ${PROJECT_SOURCE_DIR}/config/senscord_server.xml
          DESTINATION ${SSDK_CONIFIG_INSTALL_DIR})
endif()

# Create output destination of executable file.
make_output_dir_prebuild(${TARGET_NAME} ${SSDK_BINARY_OUT_DIR})

### Other.
# Copy the playing bat file.
if (WIN32)
  string(REPLACE "/" "\\" srouce_path "${PROJECT_SOURCE_DIR}")
  string(REPLACE "/" "\\" target_path "${SSDK_BUILD_OUTPUT_DIR}")

  # Copy the debug script for Windows
  add_custom_command(TARGET  ${TARGET_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy
                             ${srouce_path}\\SensCordServerStart.bat
                             ${target_path}\\SensCordServerStart.bat)
  if(SENSCORD_INSTALL_CONFIG)
    install(FILES       SensCordServerStart.bat
            DESTINATION ./)
  endif()
endif()
