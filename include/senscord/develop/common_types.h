/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_COMMON_TYPES_H_
#define SENSCORD_DEVELOP_COMMON_TYPES_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

#include "senscord/config.h"
#include "senscord/memory_allocator.h"

namespace senscord {

// client port type
const char kPortTypeClient[] = "client";

// ComponentArgument argments key : port num (not configuerd config file)
const char kArgumentNamePortNum[] = "port_num";

// default allocator key
const char kAllocatorDefaultKey[] = "";

/**
 * @brief Component port starting arguments.
 */
struct ComponentPortArgument {
  /** Connected stream key. */
  std::string stream_key;

  /** Arguments. This pair is" argument name" and "value". */
  std::map<std::string, std::string> arguments;
};

/**
 * @brief Component starting arguments.
 */
struct ComponentArgument {
  /** The name of the component's instance. */
  std::string instance_name;

  /** Allocators. This pair is "allocator name" and "accessor". */
  std::map<std::string, MemoryAllocator*> allocators;

  /** Argument. This pair is "argument name" and "value". */
  std::map<std::string, std::string> arguments;
};

/**
 * @brief Channel information of frame.
 */
struct ChannelRawData {
  uint32_t channel_id;          /**< Channel ID. */

  Memory* data_memory;          /**< including raw data address */
  size_t data_size;             /**< Size of raw data */
  size_t data_offset;           /**< Offset of raw data */
  std::string data_type;        /**< Type of raw data */
  uint64_t captured_timestamp;  /**< Timestamp from component */
};

/**
 * @brief Frame source informations
 */
struct FrameInfo {
  uint64_t sequence_number;               /**< Sequential number of frame */
  std::vector<ChannelRawData> channels;   /**< Channel data list. */

  /** Time when this frame was sent (written by SDK inside) */
  uint64_t sent_time;
};

/**
 * @brief Frame user data.
 */
struct FrameUserData {
  size_t data_size;         /**< User data size. */
  uintptr_t data_address;   /**< User data address. */
};

}   // namespace senscord
#endif  // SENSCORD_DEVELOP_COMMON_TYPES_H_
