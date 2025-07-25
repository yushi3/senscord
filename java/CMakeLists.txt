# SPDX-FileCopyrightText: 2022 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# Copy senscord/*/*.java
file(GLOB_RECURSE JAVA_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "senscord/*.java")

foreach(FILE_PATH ${JAVA_SOURCES})
  #message(${FILE_PATH})

  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}"
      "${SSDK_BUILD_OUTPUT_DIR}/java/${FILE_PATH}"
      COPYONLY)
endforeach()

# Install (copy from 'output/java' to '<install_prefix>/java')
install(DIRECTORY "${SSDK_BUILD_OUTPUT_DIR}/java" DESTINATION .)
