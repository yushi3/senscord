/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_PLAYER_SRC_PLAYER_COMPONENT_TYPES_H_
#define LIB_COMPONENT_PLAYER_SRC_PLAYER_COMPONENT_TYPES_H_

#include <inttypes.h>
#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "./player_common.h"
#include "senscord/develop/component.h"
#include "senscord/develop/recorder_common.h"
#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"
#include "senscord/property_types.h"

typedef std::map<uint64_t, BinaryPropertyList>
    PlayerComponentPropertyListBySeqNo;

struct PlayerComponentChannelProperty {
  uint32_t channel_number;
  int32_t port_id;
  const BinaryPropertyList* property_list;
};

// typedefs
typedef std::vector<std::string> PropertyKeyList;
typedef std::map<senscord::ComponentPort*, PropertyKeyList*> PortPropertyKeyMap;

struct PlayerComponentChannelData {
  std::string type;         // get from xml
  std::string description;  // get from xml
  PlayerComponentPropertyListBySeqNo property_list;

  PlayerComponentChannelData() : type(""), description(""), property_list() {}

  ~PlayerComponentChannelData() {
    type = std::string("");
    description = std::string("");

    std::map<uint64_t, BinaryPropertyList>::iterator itr1;
    for (itr1 = property_list.begin(); itr1 != property_list.end(); ++itr1) {
      // uint64_t seq_no = itr1->first;
      BinaryPropertyList* p1 = &(itr1->second);
      BinaryPropertyList::iterator itr2;
      for (itr2 = p1->begin(); itr2 != p1->end(); ++itr2) {
        senscord::BinaryProperty* p2 = &(itr2->second);
        p2->data.clear();
      }
      p1->clear();
    }
    property_list.clear();
  }
};

#endif  // LIB_COMPONENT_PLAYER_SRC_PLAYER_COMPONENT_TYPES_H_
