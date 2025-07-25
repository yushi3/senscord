# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

# Extract file name from path
get_filename_component(file_name "${SOURCE_FILE}" NAME)
# Copy to output directory
set(DEST_FILE "${DEST_DIR}/${file_name}")
file(COPY "${SOURCE_FILE}" DESTINATION ${DEST_DIR})

# Function to replace tag.
#  - #0 : replace target string
#  - #1 : replace tag
function(replace_tag REPLASE_TARGET SOURCE_TAG)
  # Read senscord.xml
  file(READ "${DEST_FILE}" XML_CONTENTS)
  # Set replace string
  set(SOURCE_TAG "${SOURCE_TAG}
    ${REPLASE_TARGET}")
  #message(STATUS ${SOURCE_TAG})
  string(REPLACE ${REPLASE_TARGET} ${SOURCE_TAG} REPLACED_CONTENTS ${XML_CONTENTS})
  # Write senscord.xml
  file(WRITE "${DEST_FILE}" ${REPLACED_CONTENTS})
endfunction()

# Function to replace stream tag.
#  - #0 : replace tag
function(replace_stream STREAM_TAG)
  set(REPLASE_TARGET "<!-- Position to add stream -->")
  replace_tag(${REPLASE_TARGET} ${STREAM_TAG})
endfunction()

# Function to replace instance tag.
#  - #0 : replace tag
function(replace_instance INSTANCE_TAG)
  set(REPLASE_TARGET "<!-- Position to add instance -->")
  replace_tag(${REPLASE_TARGET} ${INSTANCE_TAG})
endfunction()

# === Pseudo ===
if(SENSCORD_COMPONENT_PSEUDO)
  replace_stream("<!-- pseudo image stream -->
    <stream key=\"pseudo_image_stream.0\">
      <address instanceName=\"pseudo_image_instance\" type=\"image\" port=\"0\"/>
      <arguments>
        <argument name=\"width\" value=\"640\"/>
        <argument name=\"height\" value=\"480\"/>
        <argument name=\"fps\" value=\"60\"/>
      </arguments>
      <!-- <client instanceName=\"client_instance.0\"/> -->
    </stream>")

  replace_instance("<!-- pseudo image instance -->
    <instance name=\"pseudo_image_instance\" component=\"component_pseudo_image\">
    </instance>")
endif()

# === V4L2 ===
if(SENSCORD_COMPONENT_V4L2)
  replace_stream("<!-- webcam image stream -->
    <stream key=\"webcam_image_stream.0\">
      <address instanceName=\"v4l2_image_instance\" type=\"image\" port=\"0\"/>
    </stream>")

  replace_instance("<!-- webcam (Linux) instance -->
    <instance name=\"v4l2_image_instance\" component=\"component_v4l2_image\">
      <arguments>
        <argument name=\"device\" value=\"/dev/video0\"/>
        <argument name=\"buffer_num\" value=\"8\"/>
        <argument name=\"width\" value=\"640\"/>
        <argument name=\"height\" value=\"480\"/>
        <argument name=\"pixel_format\" value=\"image_nv16\"/>
        <argument name=\"yuyv_to_nv16\" value=\"true\"/>
        <argument name=\"framerate\" value=\"30\"/>
      </arguments>
    </instance>")
endif()

# === SKV Player ===
if(SENSCORD_PLAYER_SKV)
  replace_stream("<!-- skv player stream -->
    <stream key=\"skv_player_stream.0\">
      <address instanceName=\"skv_player_instance\" type=\"depth\" port=\"0\"/>
      <arguments>
        <!-- <argument name=\"target_path\" value=\"/home/user/YYYYMMDD_hhmmss_streamkey/senscord_data.skv\"/> -->
        <argument name=\"target_path\" value=\"\"/>
        <argument name=\"repeat\" value=\"true\"/>
        <argument name=\"start_offset\" value=\"0\"/>
        <argument name=\"count\" value=\"all\"/>
        <argument name=\"speed\" value=\"framerate\"/>
      </arguments>
    </stream>")

  replace_instance("<!-- skv player instance -->
    <instance name=\"skv_player_instance\" component=\"component_skv_player\">
    </instance>")
endif()
