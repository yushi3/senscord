#!/bin/zsh

# SPDX-FileCopyrightText: 2023-2025 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

SENSCORD_INSTALL_PATH=$(cd $(dirname $0)/../.. && pwd)

echo SENSCORD_INSTALL_PATH=${SENSCORD_INSTALL_PATH}

export PATH=${SENSCORD_INSTALL_PATH}/bin:${PATH}

FILE_PATH=${SENSCORD_INSTALL_PATH}/share/senscord/config
FILE_PATH=${FILE_PATH}:${SENSCORD_INSTALL_PATH}/lib
FILE_PATH=${FILE_PATH}:${SENSCORD_INSTALL_PATH}/lib/senscord/component
FILE_PATH=${FILE_PATH}:${SENSCORD_INSTALL_PATH}/lib/senscord/allocator
FILE_PATH=${FILE_PATH}:${SENSCORD_INSTALL_PATH}/lib/senscord/connection
FILE_PATH=${FILE_PATH}:${SENSCORD_INSTALL_PATH}/lib/senscord/converter
FILE_PATH=${FILE_PATH}:${SENSCORD_INSTALL_PATH}/lib/senscord/recoder
FILE_PATH=${FILE_PATH}:${SENSCORD_INSTALL_PATH}/lib/senscord/extension
export SENSCORD_FILE_PATH=${FILE_PATH}

export DYLD_LIBRARY_PATH=${SENSCORD_INSTALL_PATH}/lib:${SENSCORD_INSTALL_PATH}/lib/senscord/utility:${DYLD_LIBRARY_PATH}

exec "$@"
