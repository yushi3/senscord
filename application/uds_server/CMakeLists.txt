# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(UdsServer VERSION ${SensCord_VERSION})

# set target name
set(TARGET_NAME ${PROJECT_NAME})

# Add sources
set(SERVER_SOURCES
        ${PROJECT_SOURCE_DIR}/uds_server.cpp
)
add_executable(${TARGET_NAME} ${SERVER_SOURCES})

# Headers with servers
target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR})

# Linking senscord project
target_link_libraries(${TARGET_NAME} senscord server uds_connection_static)

target_compile_options(${TARGET_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

# Set target folder
set_target_folder(${TARGET_NAME})

# Install files.
install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION ${SSDK_BIN_INSTALL_DIR})

# Create output destination of executable file.
make_output_dir_prebuild(${TARGET_NAME} ${SSDK_BINARY_OUT_DIR})
