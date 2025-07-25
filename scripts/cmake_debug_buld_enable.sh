# SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################
#!/bin/sh

cd `dirname $0`

# Execute common processing script
. ./common.sh

echo "Set the following option in CMake ->"
echo "- CMAKE_BUILD_TYPE=Debug"
echo ""

cmake ../ -DCMAKE_BUILD_TYPE=Debug
