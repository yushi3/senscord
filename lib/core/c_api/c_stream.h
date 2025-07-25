/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_C_API_C_STREAM_H_
#define LIB_CORE_C_API_C_STREAM_H_

#include <vector>
#include <map>
#include <string>
#include "senscord/c_api/senscord_c_types.h"
#include "util/mutex.h"
#include "util/resource_list.h"

namespace senscord {
namespace c_api {

/**
 * @brief Resource for frame callback.
 */
struct FrameCallbackParam {
  /** Callback function */
  senscord_frame_received_callback callback;
  /** Private data */
  void* private_data;
};

const char kResourceFrameCallback[] = "c_frame_callback";

struct ResourceFrameCallback : public ResourceData {
  util::Mutex mutex;
  FrameCallbackParam* param;

  ResourceFrameCallback() : mutex(), param() {
  }

  ~ResourceFrameCallback() {
    delete param;
  }
};

/**
 * @brief Resource for event callback.
 */
struct EventCallbackParam {
  /** Callback function */
  senscord_event_received_callback2 callback;
  /** Callback function (old) */
  senscord_event_received_callback callback_old;
  /** Private data */
  void* private_data;
};

typedef std::map<std::string, EventCallbackParam*> EventCallbackList;

const char kResourceEventCallback[] = "c_event_callback";

struct ResourceEventCallback : public ResourceData {
  util::Mutex mutex;
  EventCallbackList list;

  ResourceEventCallback() : mutex(), list() {
  }

  ~ResourceEventCallback() {
    for (EventCallbackList::const_iterator
        itr = list.begin(), end = list.end(); itr != end; ++itr) {
      delete itr->second;
    }
  }
};

/**
 * @brief Resource for Stream property.
 */
const char kResourcePropertyList[] = "c_property_list";

struct ResourcePropertyList : public ResourceData {
  util::Mutex mutex;
  std::vector<std::string> property_list;
};

}  // namespace c_api
}  // namespace senscord

#endif  // LIB_CORE_C_API_C_STREAM_H_
