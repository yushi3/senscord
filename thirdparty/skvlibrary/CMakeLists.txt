# SPDX-FileCopyrightText: 2021-2025 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

find_package(Skv REQUIRED)

# Copy skv library to output destination directory
add_custom_command(
    OUTPUT ${SSDK_LIBRARY_OUT_DIR}/${SKV_LIB_NAME}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SKV_LIBRARY_DIRS}/${SKV_LIB_NAME}
        ${SSDK_LIBRARY_OUT_DIR}/${SKV_LIB_NAME}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${H5LZ4_LIBRARIES}
        ${SSDK_LIBRARY_OUT_DIR}/${H5LZ4_LIB_NAME}
)

add_custom_target(skv_library DEPENDS ${SSDK_LIBRARY_OUT_DIR}/${SKV_LIB_NAME})

add_library(skv SHARED IMPORTED GLOBAL)
add_dependencies(skv skv_library)

if(WIN32)
  # Set skv.dll and skv.lib location
  set_target_properties(skv
      PROPERTIES IMPORTED_LOCATION ${SKV_LIBRARIES}
                 IMPORTED_IMPLIB ${SKV_IMPORT_LIBRARIES}
                 INTERFACE_INCLUDE_DIRECTORIES "${SKV_INCLUDE_DIRS}")

  set(SKV_INSTALL_DIR ${SSDK_BIN_INSTALL_DIR})
else()
  # Set libskv.so location
  set_target_properties(skv
      PROPERTIES IMPORTED_LOCATION ${SKV_LIBRARIES}
                 INTERFACE_INCLUDE_DIRECTORIES "${SKV_INCLUDE_DIRS}")

  set(SKV_INSTALL_DIR ${SSDK_LIBRARY_INSTALL_DIR})
endif()

# Install SkvLibrary
install(FILES       ${SKV_LIBRARY_DIRS}/${SKV_LIB_NAME}
        DESTINATION ${SKV_INSTALL_DIR})

# Install H5LZ4Library
install(FILES       ${H5LZ4_LIBRARIES}
        DESTINATION ${SKV_INSTALL_DIR})
