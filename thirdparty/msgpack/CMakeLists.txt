# SPDX-FileCopyrightText: 2017-2025 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

if(NOT SENSCORD_USE_BUNDLED_MSGPACK)
  # Use the installed package.
  find_package(msgpack QUIET)

  if(${msgpack_FOUND})
    message(STATUS "Use the installed msgpack: ${msgpack_VERSION}")
  else()
    message(FATAL_ERROR "Install msgpack or set the cmake option -DSENSCORD_USE_BUNDLED_MSGPACK=ON")
  endif()
else()
  # Use the bundled package.
  set(MSGPACK_TARGET msgpack-3.2.0)
  set(MSGPACK_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${MSGPACK_TARGET}/include)
  set(MSGPACK_SOURCE_TAR ${CMAKE_CURRENT_SOURCE_DIR}/${MSGPACK_TARGET}.tar.gz)

  message(STATUS "Use the bundled ${MSGPACK_TARGET}")

  # Unpacking msgpack-x.x.x.tar.gz
  add_custom_command(OUTPUT ${MSGPACK_INCLUDE_DIR}
      COMMAND ${CMAKE_COMMAND} -E tar xzf "${MSGPACK_SOURCE_TAR}"
      DEPENDS "${MSGPACK_SOURCE_TAR}"
      COMMENT "Unpacking ${MSGPACK_SOURCE_TAR}"
      VERBATIM)

  add_custom_target(msgpack_include ALL
      DEPENDS ${MSGPACK_INCLUDE_DIR})

  # Add interface library
  add_library(msgpack_header INTERFACE)

  add_dependencies(msgpack_header msgpack_include)

  target_include_directories(msgpack_header INTERFACE
      $<BUILD_INTERFACE:${MSGPACK_INCLUDE_DIR}>
      $<INSTALL_INTERFACE:include>)

  # Install msgpack header
  install(TARGETS msgpack_header
      EXPORT ${SSDK_EXPORT_NAME}
      PUBLIC_HEADER DESTINATION ${SSDK_INCLUDE_INSTALL_DIR})
  install(FILES
      "${MSGPACK_INCLUDE_DIR}/msgpack.hpp"
      DESTINATION ${SSDK_INCLUDE_INSTALL_DIR})
  install(DIRECTORY
      "${MSGPACK_INCLUDE_DIR}/msgpack"
      DESTINATION ${SSDK_INCLUDE_INSTALL_DIR})
endif()
