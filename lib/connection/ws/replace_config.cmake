# SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

set(TARGET_TAG "<!-- Position to add Connection -->")
set(ADD_STRING "<!-- Web socket -->
  <connection key=\"ws\" library=\"ws_connection\"/>
")

# Read file
file(READ ${TARGET_FILE} ORIGINAL_CONTENTS)

# Set replace string
set(REPLACE_STRING
 "${ADD_STRING}
  ${TARGET_TAG}")

string(REPLACE ${TARGET_TAG} ${REPLACE_STRING} REPLACED_CONTENTS ${ORIGINAL_CONTENTS})

# Write file
file(WRITE ${TARGET_FILE} ${REPLACED_CONTENTS})

