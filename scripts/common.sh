# SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################
#!/bin/sh

# Directory to run cmake.
build_dir=build

# Current directory path.
current_dir=`pwd | egrep /scripts$`

# If the directory is /senscord/scripts, it moves up one level.
if [ -n `echo ${current_dir} | egrep /senscord/scripts$` ]; then
    cd ../
fi

# If the current directory is a build directory it will not move
if [ -n `echo ${current_dir} | egrep /senscord/${build_dir}$` ]; then

    # Create a directory if it does not exist.
    if [ ! -e ./${build_dir} ]; then
        mkdir ${build_dir}
    fi

    cd ${build_dir}
fi
