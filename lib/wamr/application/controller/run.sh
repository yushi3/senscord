# SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

CURRENT_PATH=$(cd $(dirname $0) && pwd)

pushd ${CURRENT_PATH}/../../../../build/output > /dev/null

SENSCORD_OUTPUT_PATH=$(pwd)
echo "SENSCORD_OUTPUT_PATH=${SENSCORD_OUTPUT_PATH}"

popd > /dev/null

exec ${SENSCORD_OUTPUT_PATH}/bin/senscord_iwasmctrl \
    --heap-size=1048576 \
    ${SENSCORD_OUTPUT_PATH}/wasm/senscord_simple_stream.wasm -k pseudo_image_stream.0 -f 100
