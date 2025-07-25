# SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0


################################################################################

################################################################################

project(synchronizer VERSION ${SensCord_VERSION})

# header files is copied to the include/OUTPUT_INCLUDE_DIR.
set(OUTPUT_INCLUDE_DIR senscord/synchronizer)

# library file name
set(TARGET_NAME ${PROJECT_NAME})

file(GLOB TARGET_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

if(WIN32)
  set(PROJECT_NAME_FULL "Synchronizer")
  set(PROJECT_DESCRIPTION "Synchronizer library for SensCord")
  configure_file(
      ${TEMPLATE_WINDOWS_DLL_VERSION}
      ${PROJECT_BINARY_DIR}/version.rc
      @ONLY)
  list(APPEND TARGET_SOURCES
      ${PROJECT_BINARY_DIR}/version.rc)
endif()

add_library(${TARGET_NAME} SHARED ${TARGET_SOURCES})

# compile config
target_include_directories(
  ${TARGET_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${SYNCHRONIZER_INCLUDE_DIR}
  ${senscord_INCLUDE_DIR})

target_link_libraries(${TARGET_NAME} PUBLIC senscord ${COMMON_GCOV_LIBRARY})

target_compile_options(${TARGET_NAME} PRIVATE ${SSDK_COMPILE_OPTIONS})

# add_library(${TARGET_NAME}_static STATIC ${TARGET_SOURCES})

# target_link_libraries(${TARGET_NAME}_static PRIVATE senscord ${COMMON_GCOV_LIBRARY})

# this library requires C++11 support
# target_compile_features(${TARGET_NAME} PUBLIC cxx_std_11)

# add synchronizer lib to IDE project tree.
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER
                                                ${SYNCHRONIZER_FOLDER_NAME})

# install config
if (WIN32)
  install(TARGETS ${TARGET_NAME} 
          EXPORT ${SSDK_EXPORT_NAME}
          RUNTIME DESTINATION ${UTILITY_SHARED_LIB_INSTALL_DIR}
          ARCHIVE DESTINATION ${UTILITY_WIN_LIB_INSTALL_DIR})
else()
  install(TARGETS ${TARGET_NAME}
          EXPORT ${SSDK_EXPORT_NAME}
          LIBRARY DESTINATION ${UTILITY_SHARED_LIB_INSTALL_DIR})
endif()

file(GLOB SYNCHRONIZER_PUBLIC_HEADER_FILES
     ${SYNCHRONIZER_INCLUDE_DIR}/senscord/synchronizer/*.h)
install(FILES ${SYNCHRONIZER_PUBLIC_HEADER_FILES} DESTINATION include/${OUTPUT_INCLUDE_DIR})
