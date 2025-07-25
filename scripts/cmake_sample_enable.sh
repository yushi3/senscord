# SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################
#!/bin/sh

cd `dirname $0`

# Execute common processing script
. ./common.sh

echo "Set the following option in CMake ->"
echo "- SENSCORD_SAMPLE=ON"
echo ""

cmake ../ -DSENSCORD_SAMPLE=ON
