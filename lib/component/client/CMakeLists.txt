# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

set(PROJECT_NAME component_client)
project(${PROJECT_NAME} VERSION ${SensCord_VERSION})

# target name
set(TARGET_NAME ${PROJECT_NAME})

set(SOURCE_OBJECTS
    ${PROJECT_SOURCE_DIR}/src/component_client.cpp
    ${PROJECT_SOURCE_DIR}/src/client_messenger_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/client_property_accessor.cpp
    ${PROJECT_SOURCE_DIR}/src/allocate_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/allocate_adapter.cpp
    ${PROJECT_SOURCE_DIR}/src/port_allocator.cpp
    ${PROJECT_SOURCE_DIR}/src/port_frame_manager.cpp
)

if(WIN32)
  set(PROJECT_NAME_FULL "Client Component")
  set(PROJECT_DESCRIPTION "Client component library for SensCord")
  configure_file(
      ${TEMPLATE_WINDOWS_DLL_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND SOURCE_OBJECTS
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

# Create library target.
add_library(${TARGET_NAME} SHARED ${SOURCE_OBJECTS})

# config xml file
set(COMPONENT_XML_FILE ${PROJECT_SOURCE_DIR}/config/${TARGET_NAME}.xml)

# Set target folder name
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${FOLDER_NAME})

# link files
target_link_libraries(${TARGET_NAME} senscord)

target_compile_options(${TARGET_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

# Create output destination directory
add_custom_command(TARGET  ${TARGET_NAME} PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E make_directory
                           ${SSDK_COMPONENT_OUT_DIR})

install_component(${TARGET_NAME} ${COMPONENT_XML_FILE})

### Copy Config file.
add_configure_file(${TARGET_NAME} ${COMPONENT_XML_FILE} ${SSDK_COMPONENT_OUT_DIR})
