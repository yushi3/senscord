# SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################
#!/bin/sh

cd `dirname $0`

# Execute common processing script
. ./common.sh

echo "Set the following option in CMake ->"
echo "- SENSCORD_LOG_ENABLED=ON"
echo "- SENSCORD_LOG_OSAL_ENABLED=ON"
echo "- SENSCORD_LOG_TIME_ENABLED=ON"
echo ""

cmake ../ -DSENSCORD_LOG_ENABLED=ON -DSENSCORD_LOG_TIME_ENABLED=ON -DSENSCORD_LOG_OSAL_ENABLED=ON
