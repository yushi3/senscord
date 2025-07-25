/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_SENSCORD_TYPES_H_
#define SENSCORD_SENSCORD_TYPES_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/event_argument.h"
#include "senscord/property_types.h"

namespace senscord {

// Allocator type
const char kAllocatorTypeHeap[] = "heap";
#ifdef SENSCORD_ALLOCATOR_SHARED_MEMORY
const char kAllocatorTypeSharedMemory[] = "shared_memory";
#endif  // SENSCORD_ALLOCATOR_SHARED_MEMORY

// Allocator name
const char kAllocatorNameDefault[] = "";

// Stream types
const char kStreamTypeImage[] = "image";
const char kStreamTypeDepth[] = "depth";
const char kStreamTypeImu[]   = "imu";
const char kStreamTypeSlam[]  = "slam";
const char kStreamTypeObjectDetection[]  = "object_detection";
const char kStreamTypeKeyPoint[]  = "key_point";
const char kStreamTypeTemporalContrast[]  = "pixel_polarity";
const char kStreamTypeObjectTracking[]  = "object_tracking";
const char kStreamTypeAudio[] = "audio";
/**
 * @deprecated will be replaced by kStreamTypeTemporalContrast
 */
const char kStreamTypePixelPolarity[]  = "pixel_polarity";

/**
 * @brief The information of stream key.
 */
struct StreamTypeInfo {
  std::string key;    /**< Stream key. */
  std::string type;   /**< Stream type. */
  std::string id;     /**< ID. */

  SENSCORD_SERIALIZE_DEFINE(key, type, id)
};

typedef FrameBufferingProperty FrameBuffering;

/**
 * @brief Open stream setting.
 */
struct OpenStreamSetting {
  FrameBuffering frame_buffering;     /**< Frame buffering setting. */
  std::map<std::string, std::string> arguments;   /**< Stream arguments. */
};

// Stream with no destination
const int32_t kDestinationStreamNone = -1;

typedef VersionProperty Version;

/**
 * @brief Stream version information.
 */
struct StreamVersion {
  Version stream_version;                   /**< Stream version. */
  std::vector<Version> linkage_versions;    /**< Stream linkage versions. */
  int32_t destination_id;                   /**< Destination ID. */

  SENSCORD_SERIALIZE_DEFINE(stream_version, linkage_versions, destination_id)
};

/**
 * @brief SensCord version information.
 */
struct SensCordVersion {
  Version senscord_version;   /**< SensCord version. */
  Version project_version;    /**< Project version. */
  /** Stream versions(Key=Streamkey). */
  std::map<std::string, StreamVersion> stream_versions;
  /** Server versions(Key=Destination ID). */
  std::map<int32_t, SensCordVersion> server_versions;

  SENSCORD_SERIALIZE_DEFINE(senscord_version, project_version,
      stream_versions, server_versions)
};

#ifdef SENSCORD_SERVER_SETTING
/**
 * @brief The address of server stream
 */
struct ServerStreamAddress {
  std::string instance_name; /**< Component instance name. */
  std::string port_type;     /**< Port type. */
  int32_t port_id;           /**< Connection port ID. */

  SENSCORD_SERIALIZE_DEFINE(instance_name, port_type, port_id)
};

/**
 * @brief Server stream setting.
 */
struct ServerStreamSetting {
  std::string stream_key; /**< Stream key. */

  /** Connecting port address. */
  ServerStreamAddress address;
  /** Radical address for client stream. */
  ServerStreamAddress radical_address;

  FrameBuffering frame_buffering;   /**< Frame buffering setting. */
  std::string client_instance_name; /**< Client instance name. */
  bool client_specified; /**< Whether or not the client tag is specified. */
  std::string identification;

  SENSCORD_SERIALIZE_DEFINE(stream_key, address, radical_address,
                            frame_buffering, client_instance_name,
                            client_specified, identification)
};

/**
 * @brief Server component instance configuration.
 */
struct ServerComponentInstanceConfig {
  std::string instance_name;
  std::string component_name;
  std::map<std::string, std::string> allocator_key_list;

  SENSCORD_SERIALIZE_DEFINE(instance_name, component_name, allocator_key_list)
};

/**
 * @brief Server configuration.
 */
struct ServerConfig {
  std::vector<ServerStreamSetting> stream_list;
  std::vector<ServerComponentInstanceConfig> instance_list;

  SENSCORD_SERIALIZE_DEFINE(stream_list, instance_list)
};
#endif  // SENSCORD_SERVER_SETTING

/**
 * @brief Raw data informations.
 */
struct RawData {
  void* address;        /**< virtual address */
  size_t size;          /**< data size */
  std::string type;     /**< data type */
  uint64_t timestamp;   /**< nanoseconds timestamp caputured by the device */
};

// Event definitions
const char kEventAny[] = "EventAny";    // only for event receiving

/**
 * @brief Error event.
 *
 * - [key] kEventArgumentCause, [type] int32_t (Cast to Status::Cause)
 * - [key] kEventArgumentMessage, [type] std::string
 */
const char kEventError[] = "EventError";

/**
 * @brief Fatal error event.
 *
 * - [key] kEventArgumentCause, [type] int32_t (Cast to Status::Cause)
 * - [key] kEventArgumentMessage, [type] std::string
 */
const char kEventFatal[] = "EventFatal";

/**
 * @brief Frame dropped event.
 *
 * - [key] kEventArgumentSequenceNumber, [type] uint64_t
 */
const char kEventFrameDropped[] = "EventFrameDropped";

/**
 * @brief Property updated event.
 *
 * - [key] kEventArgumentPropertyKey, [type] std::string
 */
const char kEventPropertyUpdated[] = "EventPropertyUpdated";

const char kEventPlugged[] = "EventPlugged";
const char kEventUnplugged[] = "EventUnplugged";

/**
 * @brief Record state event.
 *
 * - [key] kEventArgumentRecordState, [type] uint8_t
 * - [key] kEventArgumentRecordCount, [type] uint32_t
 */
const char kEventRecordState[] = "EventRecordState";

// Event argument key.
/**
 * @brief Event argument: "cause", [type] int32_t (Cast to Status::Cause)
 */
const char kEventArgumentCause[] = "cause";

/**
 * @brief Event argument: "message", [type] std::string
 */
const char kEventArgumentMessage[] = "message";

/**
 * @brief Event argument: "sequence_number", [type] uint64_t
 */
const char kEventArgumentSequenceNumber[] = "sequence_number";

/**
 * @brief Event argument: "property_key", [type] std::string
 */
const char kEventArgumentPropertyKey[] = "property_key";

/**
 * @brief Event argument: "state", [type] uint8_t (0:stopped, 1:started)
 */
const char kEventArgumentRecordState[] = "state";

/**
 * @brief Event argument: "count", [type] uint32_t
 */
const char kEventArgumentRecordCount[] = "count";

/**
 * @brief Event argument: "path", [type] std::string
 */
const char kEventArgumentRecordPath[] = "path";

// Channel ID Definitions
const uint32_t kChannelIdBase = 0;
const uint32_t kChannelIdVendorBase = 0x80000000;

// Image frame
inline uint32_t kChannelIdImage(uint32_t index) {
  return kChannelIdBase + index;
}

// Depth frame
inline uint32_t kChannelIdDepth(uint32_t index) {
  return kChannelIdBase + (index * 3) + 0;
}

inline uint32_t kChannelIdDepthConfidence(uint32_t index) {
  return kChannelIdBase + (index * 3) + 1;
}

inline uint32_t kChannelIdDepthPointCloud(uint32_t index) {
  return kChannelIdBase + (index * 3) + 2;
}

// SLAM frame
const uint32_t kChannelIdSlamPose       = kChannelIdBase + 0;
const uint32_t kChannelIdSlamPointCloud = kChannelIdBase + 1;
const uint32_t kChannelIdSlamGridMap    = kChannelIdBase + 2;

// IMU frame
const uint32_t kChannelIdImuAcceleration    = kChannelIdBase + 0;
const uint32_t kChannelIdImuAngularVelocity = kChannelIdBase + 1;
const uint32_t kChannelIdImuMagneticField   = kChannelIdBase + 2;

// ObjectDetection frame
const uint32_t kChannelIdObjectDetecion = kChannelIdBase + 0;

// KeyPoint frame
const uint32_t kChannelIdKeyPoint = kChannelIdBase + 0;

// TemporalContrast frame
const uint32_t kChannelIdTemporalContrastData = kChannelIdBase + 0;
const uint32_t kChannelIdTemporalContrastImage = kChannelIdBase + 1;

/**
 * @deprecated will be replaced by kChannelIdTemporalContrastData
 */
const uint32_t kChannelIdPixelPolarityData = kChannelIdTemporalContrastData;
/**
 * @deprecated will be replaced by kChannelIdTemporalContrastImage
 */
const uint32_t kChannelIdPixelPolarityImage = kChannelIdTemporalContrastImage;

// ObjectTracking frame
const uint32_t kChannelIdObjectTracking = kChannelIdBase + 0;

// Audio frame
inline uint32_t kChannelIdAudio(uint32_t index) {
  return kChannelIdBase + index;
}

}  // namespace senscord
#endif    // SENSCORD_SENSCORD_TYPES_H_
