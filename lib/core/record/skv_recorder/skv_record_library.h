/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_SKV_RECORDER_SKV_RECORD_LIBRARY_H_
#define LIB_CORE_RECORD_SKV_RECORDER_SKV_RECORD_LIBRARY_H_

#include <inttypes.h>
#include <string>
#include <vector>
#include <map>
#include "senscord/status.h"
#include "senscord/stream.h"
#include "senscord/osal.h"
#include "softkinetic/skv/core.h"

namespace senscord {
// Pointcloud size
const uint8_t kPointCloudXyzPlane  = 3;
const uint8_t kPointCloudXyz16Bpp  = 2;
const uint8_t kPointCloudXyz32Bpp  = 4;

// The name of the channel rawdata to record in the skv stream.
const char kSkvStreamDepth[]           = "depth";
const char kSkvStreamDepthFloat[]      = "depth_float";
const char kSkvStreamConfidence[]      = "confidence";
const char kSkvStreamFloatConfidence[] = "float_confidence";
const char kSkvStreamPointcloud[]      = "point-cloud";
const char kSkvStreamPointcloudFloat[] = "point-cloud_float";
const char kSkvStreamRawData[]         = "raw_data";
const char kSkvStreamRawDataSecond[]   = "second_raw_data";

// The prefix of the channel property to record in the skv stream.
// - Used in combination with the name of channel rawdata.
//   ex: "senscord_channel_property_depth"
const char kSkvStreamChannelPropertyPrefix[] = "senscord_channel_property_";

// The name of the stream property to record in the skv buffer.
const char kSkvBufferStreamProperty[] = "senscord_stream_property";

// The name of the SensorProperty to record in the skv buffer.
const char kSkvBufferSoftwareId[]     = "software_id";
const char kSkvBufferCalibration[]    = "calibration";

// The name of the FrameExtentionProperty to record in the skv stream.
const char kSkvStreamFrameId[]              = "frame_id";
const char kSkvStreamHostTimestamp[]        = "host_timestamp";
const char kSkvStreamErrorInformationType[] = "error_information_type";
const char kSkvStreamErrorInformation[]     = "error_information";
const char kSkvStreamRawLaserTemperature[]  = "raw_laser_temperature";
const char kSkvStreamLowAccuracyData[]      = "low_accuracy_data";
const char kSkvStreamFrameRate[]            = "frame_rate";
const char kSkvStreamMode[]                 = "mode";
const char kSkvStreamNumberOfFrames[]       = "number_of_frames";
const char kSkvStreamDelay[]                = "delay";
const char kSkvStreamSamplingMode[]         = "sampling_mode";
// The name of the FrameExtentionProperty to record in the skv buffer.
const char kSkvBufferUidName[] = "uid";

// The name of the TemperatureProperty to record in the skv stream.
const char kSkvStreamLaserTemperature[]  = "laser_temperature";
const char kSkvStreamSensorTemperature[] = "sensor_temperature";

// The name of the TemperatureProperty to record in the skv stream.
const char kSkvStreamExposure[] = "exposure";

// skv image types
enum SKV_IMAGE_TYPE {
  SKV_IMAGE_TYPE_UNKNOWN   = skv_image_type_unknown,
  SKV_IMAGE_TYPE_INT8,    // skv_image_type_int8
  SKV_IMAGE_TYPE_UINT8,   // skv_image_type_uint8
  SKV_IMAGE_TYPE_INT16,   // skv_image_type_int16
  SKV_IMAGE_TYPE_UINT16,  // skv_image_type_uint16
  SKV_IMAGE_TYPE_INT32,   // skv_image_type_int32
  SKV_IMAGE_TYPE_UINT32,  // skv_image_type_uint32
  SKV_IMAGE_TYPE_BGR24,   // skv_image_type_bgr24
  SKV_IMAGE_TYPE_YUV16,   // skv_image_type_yuv16
  SKV_IMAGE_TYPE_FLOAT,   // skv_image_type_float
  SKV_IMAGE_TYPE_RGB24,   // skv_image_type_rgb24
  SKV_IMAGE_TYPE_BGRA32,  // skv_image_type_bgra32
  SKV_IMAGE_TYPE_RGBA32,  // skv_image_type_rgba32
  SKV_IMAGE_TYPE_DOUBLE   // skv_image_type_double
};

enum SKV_COMP_TYPE {
  SKV_COMPRESSION_TYPE_NONE     = skv_compression_none,
  SKV_COMPRESSION_TYPE_SNAPPY   = skv_compression_snappy,
  SKV_COMPRESSION_TYPE_ZLIB     = skv_compression_zlib,
  SKV_COMPRESSION_TYPE_LZ4      = skv_compression_lz4
};

const SKV_COMP_TYPE kSkvDefaultCompressionType = SKV_COMPRESSION_TYPE_LZ4;

/**
 * @brief Skv ImageStream information.
 */
struct SkvImageStreamInfo {
  std::string name;
  SKV_IMAGE_TYPE type;
  uint32_t width;
  uint32_t height;
  uint32_t stream_id;
};

/**
 * @brief Skv CustomStream information.
 */
struct SkvCustomStreamInfo {
  std::string name;
  size_t size;
  uint32_t stream_id;
};

/**
 * @brief Recorder for bin format.
 */
class SkvRecordLibrary : private util::Noncopyable {
 public:
  /**
   * @brief Create skv file.
   * @param[in] (path) skv file output path.
   * @return Status object.
   */
  Status CreateFile(const std::string& path);

  /**
   * @brief Close skv file.
   * @return Status object.
   */
  Status CloseFile();

  /**
   * @brief Add ImageStream to skv file.
   * @param[in] (stream_name) Name of ImageStream.
   * @param[in] (pixel_type) pixel type of frame.
   * @param[in] (width) width of frame.
   * @param[in] (height) height of frame.
   * @param[out] (stream_id) id of added ImageStream.
   * @return Status object.
   */
  Status AddImageStream(
      const SkvImageStreamInfo& info,
      uint32_t* stream_id);

  /**
   * @brief Add CustomStream to skv file.
   * @param[in] (stream_name) Name of CustomStream.
   * @param[in] (frame_size) size of frame.
   * @param[out] (stream_id) id of added CustomStream.
   * @return Status object.
   */
  Status AddCustomStream(
      const SkvCustomStreamInfo& info,
      uint32_t* stream_id);

  /**
   * @brief Add frame to ImageStream or CustomStream.
   * @param[in] (stream_id) id of add stream.
   * @param[in] (sent_time) frame sent time.
   * @param[in] (buffer) write data.
   * @param[in] (size) write size.
   * @return Status object.
   */
  Status AddFrame(
      const uint32_t stream_id,
      const uint64_t sent_time,
      const void* buffer,
      const size_t size);

  /**
   * @brief Add CustomBuffer.
   * @param[in] (buffer_name) name of buffer.
   * @param[in] (data) write data.
   * @return Status object.
   */
  Status AddCustomBuffer(
      const std::string& buffer_name,
      const void* buffer,
      const size_t size);

  /**
   * @brief Set intrinsics model.
   * @param[in] (stream_id) name of buffer.
   * @param[in] (width) width of frame.
   * @param[in] (height) height of frame.
   * @return Status object.
   */
  Status SetIntrinsicsModel(
      uint32_t stream_id,
      uint32_t width,
      uint32_t height,
      const CameraCalibrationParameters& calibration);

  /**
   * @brief Constructor
   */
  SkvRecordLibrary();

  /**
   * @brief Destructor
   */
  virtual ~SkvRecordLibrary();

 private:
  skv_handle* file_handle_;
  // key:stream_id, value:sent_time(micro_sec)
  std::map<uint32_t, uint64_t> last_add_frame_time_;
};

}   // namespace senscord

#endif  // LIB_CORE_RECORD_SKV_RECORDER_SKV_RECORD_LIBRARY_H_
