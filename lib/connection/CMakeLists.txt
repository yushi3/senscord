# SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################
set(CATEGORY_NAME "connection")

### Install files.
# header files
file(GLOB PROJECT_PUBLIC_HEADERS "${SSDK_INCLUDE_DIR}/${SSDK_CORE_NAME}/${CATEGORY_NAME}/*.h")
install(FILES       ${PROJECT_PUBLIC_HEADERS}
        DESTINATION ${SSDK_INCLUDE_SENSCORD_INSTALL_DIR}/${CATEGORY_NAME})

# Generate config.
add_custom_target(Generate-config-connection ALL
    COMMAND ${CMAKE_COMMAND} -E copy
        ${SSDK_TOP_DIR}/lib/core/config/senscord_connections.xml
        ${SSDK_CONFIG_OUT_DIR}/senscord_connections.xml)

### Add subdirectories
add_subdirectory(tcp)
if(UNIX)
  add_subdirectory(uds)
endif()
if(SENSCORD_SERVER_WEBSOCKET)
  add_subdirectory(ws)
endif()
