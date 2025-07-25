# SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(senscord VERSION ${SensCord_VERSION})

# Name of the build target
set(TARGET_NAME ${PROJECT_NAME})

### Setting the output destination.
# Change the output destination directory of the library.
set_ssdk_library_output_dir(${SSDK_LIBRARY_OUT_DIR})

###
# Setting Build Options.

# Compile options.
set(COMMON_COMPILE_OPTION)

# Prevent throwing exceptions in setting compile options.
if(WIN32)
  # Suppress visual studio warning about exception.
  set(COMMON_COMPILE_OPTION "/wd4530 /wd4577")
  # Delete exception handling flag.
  string(REGEX REPLACE
    "EH[s|a]?c?r?-?" "" CMAKE_CXX_FLAGS_REPLACE "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_REPLACE}")
endif()

# Set common compile option.
add_definitions(${COMMON_COMPILE_OPTION})

### Include directories
# Add lib/core include directory.
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/../../thirdparty
)

# Sources
configure_file(
    ${PROJECT_SOURCE_DIR}/core/version.cpp.in
    ${PROJECT_BINARY_DIR}/core/version.cpp
    @ONLY)

## Core
list(APPEND LIB_CORE_SOURCES
    ${PROJECT_BINARY_DIR}/core/version.cpp
    ${PROJECT_SOURCE_DIR}/core/senscord.cpp
    ${PROJECT_SOURCE_DIR}/core/core_behavior.cpp
    ${PROJECT_SOURCE_DIR}/core/stream_manager.cpp
    ${PROJECT_SOURCE_DIR}/core/config_manager.cpp
    ${PROJECT_SOURCE_DIR}/core/core_function_lock_manager.cpp
    ${PROJECT_SOURCE_DIR}/core/status.cpp
)

## Configuration
list(APPEND LIB_CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/configuration/configuration_core.cpp
    ${PROJECT_SOURCE_DIR}/configuration/core_config.cpp
    ${PROJECT_SOURCE_DIR}/configuration/allocator_config_reader.cpp
)

## Stream
list(APPEND LIB_CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/stream/stream_core.cpp
    ${PROJECT_SOURCE_DIR}/stream/frame_observer_order.cpp
    ${PROJECT_SOURCE_DIR}/stream/event_observer_order.cpp
    ${PROJECT_SOURCE_DIR}/stream/property_history_book.cpp
    ${PROJECT_SOURCE_DIR}/stream/event_argument.cpp
    ${PROJECT_SOURCE_DIR}/stream/stream_function_lock_manager.cpp
)

## Frame
list(APPEND LIB_CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/frame/frame_core.cpp
    ${PROJECT_SOURCE_DIR}/frame/channel_core.cpp
    ${PROJECT_SOURCE_DIR}/frame/frame_manager_factory.cpp
    ${PROJECT_SOURCE_DIR}/frame/frame_manager_core.cpp
    ${PROJECT_SOURCE_DIR}/frame/frame_manager_discard.cpp
    ${PROJECT_SOURCE_DIR}/frame/frame_manager_overwrite.cpp
    ${PROJECT_SOURCE_DIR}/frame/frame_manager_realtime.cpp
    ${PROJECT_SOURCE_DIR}/frame/frame_manager_unlimited.cpp
)

## Loader
list(APPEND LIB_CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/loader/class_dynamic_factory.cpp
    ${PROJECT_SOURCE_DIR}/loader/class_dynamic_loader.cpp
)

## Component
list(APPEND LIB_CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/component/component_manager.cpp
    ${PROJECT_SOURCE_DIR}/component/component_adapter.cpp
    ${PROJECT_SOURCE_DIR}/component/component_port_core.cpp
    ${PROJECT_SOURCE_DIR}/component/core_component.cpp
    ${PROJECT_SOURCE_DIR}/component/standard_component.cpp
    ${PROJECT_SOURCE_DIR}/component/stream_source_adapter.cpp
    ${PROJECT_SOURCE_DIR}/component/stream_source_property_accessor.cpp
    ${PROJECT_SOURCE_DIR}/component/stream_source_function_lock_manager.cpp
    ${PROJECT_SOURCE_DIR}/component/property_lock_manager.cpp
    ${PROJECT_SOURCE_DIR}/loader/component_dynamic_factory.cpp
    ${PROJECT_SOURCE_DIR}/loader/component_dynamic_loader.cpp
)

## Extension
list(APPEND LIB_CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/extension/extension_manager.cpp
    ${PROJECT_SOURCE_DIR}/extension/extension_library.cpp
    ${PROJECT_SOURCE_DIR}/extension/stream_extension.cpp
)

## Util
list(APPEND LIB_CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/util/observer.cpp
    ${PROJECT_SOURCE_DIR}/util/senscord_utils.cpp
    ${PROJECT_SOURCE_DIR}/util/environment_utils.cpp
    ${PROJECT_SOURCE_DIR}/util/singleton.cpp
    ${PROJECT_SOURCE_DIR}/util/property_utils.cpp
    ${PROJECT_SOURCE_DIR}/util/resource_list.cpp
    ${PROJECT_SOURCE_DIR}/util/xml_parser.cpp
    ${PROJECT_SOURCE_DIR}/util/rawdata_utils.cpp
)

## Serialize
if(SENSCORD_SERIALIZE)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/serialize/serialize.cpp
      ${PROJECT_SOURCE_DIR}/serialize/serialize_buffer.cpp
      ${PROJECT_SOURCE_DIR}/serialize/socket_message_buffer.cpp
  )
endif()

## Allocator
list(APPEND LIB_CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/allocator/memory_manager.cpp
    ${PROJECT_SOURCE_DIR}/allocator/memory_allocator_heap.cpp
    ${PROJECT_SOURCE_DIR}/allocator/memory_allocator_dynamic_factory.cpp
    ${PROJECT_SOURCE_DIR}/allocator/memory_allocator_dynamic_loader.cpp
)

## Allocator(Shared memory)
if(SENSCORD_ALLOCATOR_SHARED_MEMORY)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/allocator/shared_memory_allocator.cpp
  )
endif()

## Logger
list(APPEND LIB_CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/logger/logger.cpp
)

## Version
if(SENSCORD_STREAM_VERSION)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/core/version_manager.cpp
      ${PROJECT_SOURCE_DIR}/component/component_config_manager.cpp
  )
  if(SENSCORD_SERVER)
    list(APPEND LIB_CORE_SOURCES
        ${PROJECT_SOURCE_DIR}/core/version_fetcher.cpp
    )
  endif()
endif()

## Connection
if(SENSCORD_SERVER)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/connection/connection_manager.cpp
      ${PROJECT_SOURCE_DIR}/connection/connection_config_manager.cpp
      ${PROJECT_SOURCE_DIR}/connection/connection_utils.cpp
      ${PROJECT_SOURCE_DIR}/connection/connection_dynamic_factory.cpp
      ${PROJECT_SOURCE_DIR}/connection/connection_dynamic_loader.cpp
      ${PROJECT_SOURCE_DIR}/connection/client_instance_utils.cpp
  )
endif()

## ServerConfig
if(SENSCORD_SERVER_SETTING)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/core/server_config_manager.cpp
      ${PROJECT_SOURCE_DIR}/core/server_config_fetcher.cpp
  )
endif()

## Searcher(SSDP)
if(SENSCORD_SERVER_SEARCH_SSDP)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/searcher/ssdp_module.cpp
      ${PROJECT_SOURCE_DIR}/../../thirdparty/lssdp/lssdp.cpp
  )
endif()

## Searcher(UCOM)
if(SENSCORD_SERVER_SEARCH_UCOM)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/searcher/ucom_module.cpp
  )
endif()

# Record / Player common
if(SENSCORD_RECORDER OR SENSCORD_PLAYER)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/record/record_utility.cpp
  )
endif()

## Record
if(SENSCORD_RECORDER)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/record/frame_recorder.cpp
      ${PROJECT_SOURCE_DIR}/record/info_writer.cpp
      ${PROJECT_SOURCE_DIR}/record/channel_recorder_adapter.cpp
      ${PROJECT_SOURCE_DIR}/record/recorder_manager.cpp
  )
endif()

## Record(loader)
if(SENSCORD_RECORDER_LOADER)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/record/recorder_config_manager.cpp
      ${PROJECT_SOURCE_DIR}/record/recorder_dynamic_factory.cpp
      ${PROJECT_SOURCE_DIR}/record/recorder_dynamic_loader.cpp
  )
endif()

## Record(raw)
if(SENSCORD_RECORDER_RAW)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/record/raw_recorder.cpp
      ${PROJECT_SOURCE_DIR}/record/composite_raw_recorder.cpp
      ${PROJECT_SOURCE_DIR}/record/raw_file_writer.cpp
      ${PROJECT_SOURCE_DIR}/record/composite_raw_file_writer.cpp
      ${PROJECT_SOURCE_DIR}/record/channel_properties_file_writer.cpp
      ${PROJECT_SOURCE_DIR}/record/raw_index_file_writer.cpp
  )
endif()

## Record(Skv)
if(SENSCORD_RECORDER_SKV)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/record/skv_recorder/skv_recorder.cpp
      ${PROJECT_SOURCE_DIR}/record/skv_recorder/skv_record_library.cpp
      ${PROJECT_SOURCE_DIR}/record/skv_recorder/skv_record_library_manager.cpp
      ${PROJECT_SOURCE_DIR}/record/skv_recorder/skv_channel_recorder.cpp
      ${PROJECT_SOURCE_DIR}/record/skv_recorder/skv_depth_channel_recorder.cpp
      ${PROJECT_SOURCE_DIR}/record/skv_recorder/skv_confidence_channel_recorder.cpp
      ${PROJECT_SOURCE_DIR}/record/skv_recorder/skv_pointcloud_channel_recorder.cpp
      ${PROJECT_SOURCE_DIR}/record/skv_recorder/skv_rawdata_channel_recorder.cpp
  )
endif()

## Messenger
list(APPEND LIB_CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/messenger/messenger_manager.cpp
    ${PROJECT_SOURCE_DIR}/messenger/messenger_topic.cpp
    ${PROJECT_SOURCE_DIR}/messenger/messenger_component.cpp
    ${PROJECT_SOURCE_DIR}/messenger/publisher_core.cpp
    ${PROJECT_SOURCE_DIR}/messenger/inner_frame_sender.cpp
)
if(SENSCORD_SERVER)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/messenger/client_messenger.cpp
      ${PROJECT_SOURCE_DIR}/messenger/server_frame_sender.cpp
  )
endif()

## Audio
list(APPEND LIB_CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/audio/audio_pcm.cpp
)

## C API
if(SENSCORD_API_C)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_SOURCE_DIR}/c_api/c_common.cpp
      ${PROJECT_SOURCE_DIR}/c_api/c_status.cpp
      ${PROJECT_SOURCE_DIR}/c_api/c_core.cpp
      ${PROJECT_SOURCE_DIR}/c_api/c_stream.cpp
      ${PROJECT_SOURCE_DIR}/c_api/c_frame.cpp
      ${PROJECT_SOURCE_DIR}/c_api/c_channel.cpp
      ${PROJECT_SOURCE_DIR}/c_api/c_environment.cpp
      ${PROJECT_SOURCE_DIR}/c_api/c_serialize.cpp
      ${PROJECT_SOURCE_DIR}/c_api/c_event_argument.cpp
      ${PROJECT_SOURCE_DIR}/c_api/c_config.cpp
      ${PROJECT_SOURCE_DIR}/c_api/c_config_reader.cpp
      ${PROJECT_SOURCE_DIR}/c_api/converter_dynamic_factory.cpp
      ${PROJECT_SOURCE_DIR}/c_api/converter_dynamic_loader.cpp
      ${PROJECT_SOURCE_DIR}/c_api/converter_manager.cpp
      ${PROJECT_SOURCE_DIR}/c_api/c_property_utils.cpp
      ${PROJECT_SOURCE_DIR}/c_api/c_audio.cpp
      ${PROJECT_SOURCE_DIR}/c_api/c_rawdata_utils.cpp
  )
endif()

if(WIN32)
  set(PROJECT_NAME_FULL "SensCord")
  set(PROJECT_DESCRIPTION "SensCord Core library")
  configure_file(
      ${TEMPLATE_WINDOWS_DLL_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND LIB_CORE_SOURCES
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

list(APPEND LIB_CORE_PUBLIC_LINKS senscord_osal)

if(SENSCORD_RECORDER_SKV)
  list(APPEND LIB_CORE_PRIVATE_LINKS skv)
endif()

# Create senscord.lib target.
add_library(${TARGET_NAME} SHARED ${LIB_CORE_SOURCES})

target_include_directories(${TARGET_NAME} PUBLIC
    $<BUILD_INTERFACE:${SSDK_BUILD_OUTPUT_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${SSDK_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>)

message(STATUS "senscord link libraries -> ${LIB_CORE_PUBLIC_LINKS} ${LIB_CORE_PRIVATE_LINKS}")

### Set build target.
# Setting the folder name displayed in the IDE
if (WIN32)
  # Set target folder name
  set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "core")
endif()

# Link libraries.
target_link_libraries(${TARGET_NAME}
    PUBLIC ${LIB_CORE_PUBLIC_LINKS}
    PRIVATE ${LIB_CORE_PRIVATE_LINKS})
if(TARGET msgpack_header)
  target_link_libraries(${TARGET_NAME} PUBLIC msgpack_header)
endif()

target_compile_options(${TARGET_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

target_compile_definitions(${TARGET_NAME} PRIVATE SENSCORD_CORE_BUILD)

if(SENSCORD_SERVER_SEARCH_SSDP)
  # link libraries for SSDP.
  if(WIN32)
    target_link_libraries(${TARGET_NAME} PRIVATE ws2_32.lib)
    target_link_libraries(${TARGET_NAME} PRIVATE Iphlpapi.lib)
  endif()
endif()

# Create output destination directory
add_custom_command(TARGET  ${TARGET_NAME} PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E make_directory
                   ${SSDK_LIBRARY_OUT_DIR})

### Copy Config file.
generate_senscord_xml(${TARGET_NAME}
                      ${PROJECT_SOURCE_DIR}/config/senscord.xml
                      ${SSDK_CONFIG_OUT_DIR})
add_configure_file(${TARGET_NAME}
                   ${PROJECT_SOURCE_DIR}/config/senscord_allocators.xml
                   ${SSDK_CONFIG_OUT_DIR})
if(SENSCORD_RECORDER_LOADER)
  add_configure_file(${TARGET_NAME}
                     ${PROJECT_SOURCE_DIR}/config/senscord_recorders.xml
                     ${SSDK_CONFIG_OUT_DIR})
endif()

# Install files.
file(GLOB LIB_CORE_INCLUDE_FILES "${SSDK_INCLUDE_DIR}/senscord/*.h")
set_target_properties(${TARGET_NAME}
                      PROPERTIES PUBLIC_HEADER "${LIB_CORE_INCLUDE_FILES}")

install_senscord(${TARGET_NAME})

install(FILES ${SSDK_BUILD_OUTPUT_INCLUDE_DIR}/senscord/config.h
        DESTINATION ${SSDK_INCLUDE_SENSCORD_INSTALL_DIR})

file(GLOB LIB_CORE_DEVELOP_INCLUDE_FILES
          "${SSDK_INCLUDE_DIR}/senscord/develop/*.h")

install(FILES       ${LIB_CORE_DEVELOP_INCLUDE_FILES}
        DESTINATION ${SSDK_INCLUDE_SENSCORD_DEVELOP_INSTALL_DIR})

if(SENSCORD_API_C)
  # Install C API header files.
  file(GLOB LIB_CORE_C_API_INCLUDE_FILES
       "${SSDK_INCLUDE_DIR}/senscord/c_api/*.h")
  install(FILES       ${LIB_CORE_C_API_INCLUDE_FILES}
          DESTINATION ${SSDK_INCLUDE_SENSCORD_C_API_INSTALL_DIR})
endif()

if(SENSCORD_INSTALL_CONFIG)
  list(APPEND INSTALL_FILES ${SSDK_CONFIG_OUT_DIR}/senscord.xml)
  list(APPEND INSTALL_FILES ${SSDK_CONFIG_OUT_DIR}/senscord_allocators.xml)

  if(SENSCORD_RECORDER_LOADER)
    list(APPEND INSTALL_FILES ${SSDK_CONFIG_OUT_DIR}/senscord_recorders.xml)
  endif()

  if(SENSCORD_SERVER)
    list(APPEND INSTALL_FILES ${SSDK_CONFIG_OUT_DIR}/senscord_connections.xml)
  endif()

  install(FILES       ${INSTALL_FILES}
          DESTINATION ${SSDK_CONIFIG_INSTALL_DIR})
endif()

if(WIN32)
  install(
      FILES       ${PROJECT_SOURCE_DIR}/scripts/setup_env.bat
      DESTINATION ./)
elseif(APPLE)
  install(
      FILES       ${PROJECT_SOURCE_DIR}/scripts/setup_env.zsh
      DESTINATION ${SSDK_SENSCORD_DATA_INSTALL_DIR}
      PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                  GROUP_EXECUTE GROUP_READ)
else()
  install(
      FILES       ${PROJECT_SOURCE_DIR}/scripts/setup_env.sh
      DESTINATION ${SSDK_SENSCORD_DATA_INSTALL_DIR}
      PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                  GROUP_EXECUTE GROUP_READ)
endif()
