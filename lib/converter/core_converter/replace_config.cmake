# SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

set(TARGET_TAG "<!-- Position to add converter -->")
set(ADD_STRING "<!-- Core converter -->
      <converter name=\"senscord_core_converter\" type=\"property|rawdata\"/>
")

# Read file
if(EXISTS ${TARGET_FILE})
  file(READ ${TARGET_FILE} ORIGINAL_CONTENTS)

  string(FIND ${ORIGINAL_CONTENTS} ${ADD_STRING} POSITION)
  if(${POSITION} EQUAL -1)
    # Set replace string
    set(REPLACE_STRING
      "${ADD_STRING}
      ${TARGET_TAG}")

    string(REPLACE ${TARGET_TAG} ${REPLACE_STRING} REPLACED_CONTENTS ${ORIGINAL_CONTENTS})

    # Write file
    file(WRITE ${TARGET_FILE} ${REPLACED_CONTENTS})
  endif()
endif()
