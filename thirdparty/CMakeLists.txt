# SPDX-FileCopyrightText: 2017-2025 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

if(SENSCORD_SERIALIZE)
  add_subdirectory(msgpack)
endif()

if(SENSCORD_RECORDER_SKV OR SENSCORD_PLAYER_SKV)
  add_subdirectory(skvlibrary)
endif()
