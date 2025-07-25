/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_RAWDATA_TYPES_H_
#define SENSCORD_RAWDATA_TYPES_H_

#include <stdint.h>
#include <vector>

#include "senscord/config.h"
#include "senscord/property_types.h"
#include "senscord/serialize_define.h"
#include "senscord/develop/rawdata_utils.h"

namespace senscord {

/**
 * Basic raw data type names.
 */
const char kRawDataTypeMeta[] = "meta_data";
const char kRawDataTypeImage[] = "image_data";
const char kRawDataTypeDepth[] = "depth_data";
const char kRawDataTypeConfidence[] = "confidence_data";
const char kRawDataTypeAudio[] = "audio_data";

/**
 * AccelerationData.
 */
const char kRawDataTypeAcceleration[] = "acceleration_data";

/**
 * @brief Raw data type for Acceleration.
 */
typedef Vector3<float> AccelerationData;

/**
 * AngularVelocityData.
 */
const char kRawDataTypeAngularVelocity[] = "angular_velocity_data";

/**
 * @brief Raw data type for angular velocity.
 */
typedef Vector3<float> AngularVelocityData;

/**
 * MagneticField.
 */
const char kRawDataTypeMagneticField[] = "magnetic_field_data";

/**
 * @brief Raw data type for magnetic field.
 */
typedef Vector3<float> MagneticFieldData;

/**
 * RawDataType for rotation.
 */
const char kRawDataTypeRotation[] = "rotation_data";

/**
 * @brief Data for rotating posture.
 */
struct RotationData {
  float roll;   /**< Roll angle. */
  float pitch;  /**< Pitch angle. */
  float yaw;    /**< Yaw angle. */

  SENSCORD_SERIALIZE_DEFINE(roll, pitch, yaw)
};

// RawDataType for pose.
const char kRawDataTypePose[] = "pose_data";

/**
 * @brief Pose(quaternion) data.
 */
struct PoseQuaternionData {
  Vector3<float> position;          /**< Position(x,y,z) */
  Quaternion<float> orientation;    /**< Orientation(x,y,z,w) */

  SENSCORD_SERIALIZE_DEFINE(position, orientation)
};
/**
 * @brief Pose data.
 * @deprecated will be replaced by PoseQuaternionData
 */
typedef PoseQuaternionData PoseData;

/**
 * @brief Pose(rotation matrix) data.
 */
struct PoseMatrixData {
  Vector3<float> position;          /**< Position(x,y,z) */
  Matrix3x3<float> rotation;        /**< Rotation matrix */

  SENSCORD_SERIALIZE_DEFINE(position, rotation)
};

// RawDataType for PointCloudData
const char kRawDataTypePointCloud[] = "point_cloud_data";

// RawDataType for GridMapData
const char kRawDataTypeGridMap[] = "grid_map_data";

// RawDataType for ObjectDetectionData
const char kRawDataTypeObjectDetection[] = "object_detection_data";

/**
 * @brief Structure that object detection results.
 */
struct DetectedObjectInformation {
  uint32_t class_id;            /**< Class id of detected object. */
  float score;                  /**< Score of detected object. */
  RectangleRegionParameter box; /**< Detected object area. */

  SENSCORD_SERIALIZE_DEFINE(class_id, score, box)
};

/**
 * @brief Raw data type for object detection.
 */
struct ObjectDetectionData {
  std::vector<DetectedObjectInformation> data;    /**< Detected objects. */

  SENSCORD_SERIALIZE_DEFINE(data)
};

// RawDataType for KeyPointData
const char kRawDataTypeKeyPoint[] = "key_point_data";

/**
 * @brief Structure that key point.
 */
struct KeyPoint {
  uint32_t key_point_id;  /**< Key point id of detected object. */
  float score;            /**< Score of detected object. */
  Vector3<float> point;   /**< Point coordinates. */

  SENSCORD_SERIALIZE_DEFINE(key_point_id, score, point)
};

/**
 * @brief Structure that key point information.
 */
struct DetectedKeyPointInformation {
  uint32_t class_id;                  /**< Class id of detected object. */
  float score;                        /**< Score of detected object. */
  std::vector<KeyPoint> key_points;   /**< Detected points. */

  SENSCORD_SERIALIZE_DEFINE(class_id, score, key_points)
};

/**
 * @brief Raw data type for key points.
 */
struct KeyPointData {
  std::vector<DetectedKeyPointInformation> data;  /**< Detected points. */

  SENSCORD_SERIALIZE_DEFINE(data)
};

// RawDataType for TemporalContrast
const char kRawDataTypeTemporalContrast[] = "pixel_polarity_data";

/**
 * @brief Polarity of temporal contrast event.
 */
enum TemporalContrast {
  kTemporalContrastNegative = -1,  /**< Negative event. */
  kTemporalContrastNone,           /**< Event is none. */
  kTemporalContrastPositive,       /**< Positive event. */
};

/**
 * @brief Raw data type for a single temporal contrast event.
 */
struct TemporalContrastEvent {
  uint16_t x; /**< X value of the event. */
  uint16_t y; /**< Y value of the event. */
  uint8_t p;  /**< Polarity of the event. */
  uint8_t reserve;
};

/**
 * @brief Raw data type for temporal contrast events with same timestamp.
 */
struct TemporalContrastEventsTimeslice {
  /** the timestamp [nsec]. */
  uint64_t timestamp;
  /** the number of events contained. */
  uint32_t count;
  /** reserved area */
  uint8_t reserve[4];
  /** the array of events */
  struct TemporalContrastEvent* events;
};

/**
 * @brief Raw data type for temporal contrast events in one frame.
 */
struct TemporalContrastData {
  /** the number of event Timeslices contained. */
  uint32_t count;
  /** reserved area */
  uint8_t reserve[4];
  /** the array of event Timeslices */
  struct TemporalContrastEventsTimeslice* bundles;
};

// RawDataType for PixelPolarity
/**
 * @deprecated will be replaced by kRawDataTypeTemporalContrast
 */
const char kRawDataTypePixelPolarity[] = "pixel_polarity_data";

/**
 * @brief Raw data type for a single pixel polarity event.
 * @deprecated will be replaced by TemporalContrastEvent
 */
typedef TemporalContrastEvent PixelPolarityEvent;

/**
 * @brief Raw data type for pixel polarity events with same timestamp.
 * @deprecated will be replaced by TemporalContrastEventsTimeslice
 */
typedef TemporalContrastEventsTimeslice PixelPolarityEventsBundle;

/**
 * @brief Raw data type for pixel polarity events in one frame.
 * @deprecated will be replaced by TemporalContrastData.
 */
typedef TemporalContrastData PixelPolarityData;

// RawDataType for ObjectTrackingData
const char kRawDataTypeObjectTracking[] = "object_tracking_data";

/**
 * @brief Structure that object tracking results.
 */
struct TrackedObjectInformation {
  uint32_t track_id;            /**< Track id of tracked object. */
  uint32_t class_id;            /**< Class id of tracked object. */
  float score;                  /**< Score of tracked object. */
  Vector2<float> velocity;          /**< Velocity(x,y) of tracked object. */
  Vector2<uint32_t> position;          /**< Position(x,y) of tracked object. */
  RectangleRegionParameter box; /**< Tracked object area. */

  SENSCORD_SERIALIZE_DEFINE(track_id, class_id, score, velocity, position, box)
};

/**
 * @brief Raw data type for object tracking.
 */
struct ObjectTrackingData {
  std::vector<TrackedObjectInformation> data;    /**< Tracked objects. */

  SENSCORD_SERIALIZE_DEFINE(data)
};
}   // namespace senscord

#endif  // SENSCORD_RAWDATA_TYPES_H_
