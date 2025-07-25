# SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

CURRENT_PATH=$(cd $(dirname $0) && pwd)

pushd ${CURRENT_PATH}/../../../../build/output > /dev/null

SENSCORD_OUTPUT_PATH=$(pwd)
echo "SENSCORD_OUTPUT_PATH=${SENSCORD_OUTPUT_PATH}"

popd > /dev/null

FILE_PATH=${CURRENT_PATH}/config_client_native
FILE_PATH=${FILE_PATH}:${SENSCORD_OUTPUT_PATH}/lib
FILE_PATH=${FILE_PATH}:${SENSCORD_OUTPUT_PATH}/lib/component
FILE_PATH=${FILE_PATH}:${SENSCORD_OUTPUT_PATH}/lib/allocator
FILE_PATH=${FILE_PATH}:${SENSCORD_OUTPUT_PATH}/lib/connection
FILE_PATH=${FILE_PATH}:${SENSCORD_OUTPUT_PATH}/lib/converter
FILE_PATH=${FILE_PATH}:${SENSCORD_OUTPUT_PATH}/lib/recoder
FILE_PATH=${FILE_PATH}:${SENSCORD_OUTPUT_PATH}/lib/extension
export SENSCORD_FILE_PATH=${FILE_PATH}

export LD_LIBRARY_PATH=${SENSCORD_OUTPUT_PATH}/lib:${LD_LIBRARY_PATH}

exec ${SENSCORD_OUTPUT_PATH}/bin/SensCordSimpleStreamPlayerC -k pseudo_image_stream.0 -f 1000000
