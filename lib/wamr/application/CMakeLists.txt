# SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# iwasmctrl
add_executable(senscord_iwasmctrl
    controller/main.c
    controller/senscord_iwasmctrl.cpp
)
target_link_libraries(senscord_iwasmctrl libiwasm senscord -lpthread -lm)
target_include_directories(senscord_iwasmctrl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(TARGET senscord_iwasmctrl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SSDK_BINARY_OUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:senscord_iwasmctrl> ${SSDK_BINARY_OUT_DIR})
install(TARGETS senscord_iwasmctrl
    EXPORT ${SSDK_EXPORT_NAME}
    RUNTIME DESTINATION ${SSDK_BIN_INSTALL_DIR})

# iwasmserver
add_executable(senscord_iwasmserver
    server/main.c
    server/senscord_iwasmserver.cpp
)
target_link_libraries(senscord_iwasmserver libiwasm senscord senscord_wamr -lpthread -lm)
target_include_directories(senscord_iwasmserver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(TARGET senscord_iwasmserver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SSDK_BINARY_OUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:senscord_iwasmserver> ${SSDK_BINARY_OUT_DIR})
install(TARGETS senscord_iwasmserver
    EXPORT ${SSDK_EXPORT_NAME}
    RUNTIME DESTINATION ${SSDK_BIN_INSTALL_DIR})
