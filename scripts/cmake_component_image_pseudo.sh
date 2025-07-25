# SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################
#!/bin/sh

cd `dirname $0`

# Execute common processing script
. ./common.sh

echo "Set the following option in CMake ->"
echo "-DSENSCORD_COMPONENT_PSEUDO=ON"
echo ""

cmake ../ -DSENSCORD_COMPONENT_PSEUDO=ON
