/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAY_LIBRARY_H_
#define LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAY_LIBRARY_H_

#include <inttypes.h>
#include <string>
#include <vector>
#include <map>
#include "senscord/status.h"
#include "senscord/stream.h"
#include "senscord/osal.h"
#include "softkinetic/skv/core.h"

// Arguments
const char kSkvArgTargetPath[]           = "target_path";
const char kSkvArgStartOffset[]          = "start_offset";
const char kSkvArgCount[]                = "count";
const char kSkvArgRepeat[]               = "repeat";

// Default frame rate values
static const uint32_t kDefaultFrameRateNum = 60;
static const uint64_t kDefaultFrameRate =
                    (1000ULL * 1000 * 1000) / kDefaultFrameRateNum;  // 60fps

// skv stream names for channel rawdata
// depth (ch_0)
const char kSkvStreamNameDepth[]           = "depth";
const char kSkvStreamNameIntZ[]            = "int_z";
// depth-float(ch_3)
const char kSkvStreamNameDepthFloat[]      = "depth_float";
const char kSkvStreamNameFloatZ[]          = "float_z";
// confidence (ch_1)
const char kSkvStreamNameConfidence[]      = "confidence";
const char kSkvStreamNameIntConfidence[]   = "int_confidence";
// confidence-float (ch_4)
const char kSkvStreamNameFloatConfidence[] = "float_confidence";
// point_cloud (ch_2)
const char kSkvStreamNamePointCloud[]      = "point-cloud";
const char kSkvStreamNameIntPointCloud[]   = "int_point-cloud";
// point_cloud-float (ch_5)
const char kSkvStreamNamePointCloudFloat[] = "point-cloud_float";
const char kSkvStreamNameFloatPointCloud[] = "float_point-cloud";
// image_raw (ch_0x80000000)
const char kSkvStreamNameRawData[]         = "raw_data";
const char kSkvStreamNameTofRawData[]      = "tof_raw_data";
// image_raw (ch_0x80000001)
const char kSkvStreamNameSecondRawData[]   = "second_raw_data";

// skv stream names for channel property
const char kSkvPropertyNameDepth[]  =
                                "senscord_channel_property_depth";
const char kSkvPropertyNameDepthFloat[] =
                                "senscord_channel_property_depth_float";
const char kSkvPropertyNameConfidence[] =
                                "senscord_channel_property_confidence";
const char kSkvPropertyNameConfidenceFloat[] =
                                "senscord_channel_property_float_confidence";
const char kSkvPropertyNamePointCloud[] =
                                "senscord_channel_property_point-cloud";
const char kSkvPropertyNamePointCloudFloat[] =
                                "senscord_channel_property_point-cloud_float";
const char kSkvPropertyNameRawData[] =
                                "senscord_channel_property_raw_data";
const char kSkvPropertyNameSecondRawData[] =
                                "senscord_channel_property_second_raw_data";

// skv stream property name
const char kSkvStreamPropertyName[] = "senscord_stream_property";

// skv names for ModuleInformationProperty
const char kSkvSoftwareIdName[]     = "software_id";
const char kSkvCalibrationName[]    = "calibration";
const char kSkvCalibrationDataName[] = "calibration_data";

// skv names for FrameExtentionProperty.
const char kFrameIdStreamName[]              = "frame_id";
const char kHostTimestampStreamName[]        = "host_timestamp";
const char kErrorInformationTypeStreamName[] = "error_information_type";
const char kErrorInformationStreamName[]     = "error_information";
const char kLowAccuracyDataStreamName[]  = "low_accuracy_data";
const char kFrameRateStreamName[]        = "frame_rate";
const char kModeStreamName[]             = "mode";
const char kNumberOfFramesStreamName[]   = "number_of_frames";
const char kDelayStreamName[]            = "delay";
const char kSamplingModeStreamName[]     = "sampling_mode";
const char kSkvUidName[]           = "uid";

// skv stream names for TemperatureProperty.
const char kLaserTemperature[]  = "laser_temperature";
const char kSensorTemperature[] = "sensor_temperature";

// skv stream name for exposure_time of ExposureProperty.
const char kSkvStreamNameExposure[] = "exposure";

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

/**
* Skv Stream Info
*/
struct SkvStreamInfo{
    uint32_t id;
    uint32_t width;
    uint32_t height;
    size_t frame_size;
    skv_image_type type;
};

/**
 * @brief Recorder for bin format.
 */
class SkvPlayLibrary : private senscord::util::Noncopyable {
 public:
  /**
   * @brief Open skv file.
   * @param[in] (target_path) skv file path to be opened.
   * @return Status object.
   */
  senscord::Status OpenFile(const std::string& target_path);

  /**
   * @brief Close skv file.
   * @return Status object.
   */
  senscord::Status CloseFile();

  /**
   * @brief Check if the file is created by SensCord.
   * @param[in] (is_senscord_format) file format flag
   * @return Status object.
   */
  senscord::Status IsSensCordFormat(bool* is_senscord_format);

  /**
   * @brief Create a map of streams in a skv file
   * @param[in] (skv_stream_map) map of the stream
   * @return Status object.
   */
  senscord::Status CreateStreamMap(
    std::map <std::string, SkvStreamInfo>* skv_stream_map);

  /**
   * @brief get the size of a frame in the stream
   * @param[in] (stream_id) stream id
   * @param[in] (frame_size) frame size
   * @return Status object.
   */
  senscord::Status GetStreamFrameSize(uint32_t stream_id, size_t* frame_size);

  /**
   * @brief set name and stream info pair
   * @param[in] (stream_id) stream id
   * @param[in] (name) unique name of stream
   * @param[in] (skv_stream_info) frame_size and stream_xid
   * @return Status object.
   */
  senscord::Status SetStreamPairedInfo(
  uint32_t stream_id, std::string *name, SkvStreamInfo *skv_stream_info);

  /**
   * @brief get the buffer size and pointer to the data
   * @param[in] (buffer_name) name of custom buffer
   * @param[in] (buffer_size) size of custom buffer
   * @return Status object.
   */
  senscord::Status GetCustomBufferSize(
    const std::string& buffer_name, size_t* buffer_size);

  /**
   * @brief get the buffer size and pointer to the data
   * @param[in] (buffer_name) name of custom buffer
   * @param[in] (buffer_data)
   * @return Status object.
   */
  senscord::Status GetCustomBufferData(
    const std::string& buffer_name, void* buffer_data);

  /**
   * @brief get stream frame data.
   * @param[in] (stream_id) stream_id
   * @param[in] (frame_index) frame index
   * @param[in] (frame_data) frame data
   * @return Status object.
   */
  senscord::Status GetFrameData(
    uint32_t stream_id, uint32_t frame_index, void* frame_data);

  /**
   * @brief Get file playback time.
   * @param[in]  (stream_id) stream id
   * @param[out] (time_stamps) series of time stamp of frame
   * @return Status object.
   */
  senscord::Status GetAllFrameTimestamp(uint32_t stream_id,
    std::vector<uint64_t>* time_stamp);

  /**
   * @brief Get frame index and its timestamp closest to the given timestamp.
   * @param[in]  (stream_id) stream id
   * @param[in]  (specified_time) specified timestmap
   * @param[out] (frame_index) frame index closest to the given timestamp.
   * @param[out] (time_stamp) time stamp of the frame closest
   *                          to the given timestamp.
   * @return Status object.
   */
  senscord::Status GetClosestFrameInfoByTimeStamp(uint32_t stream_id,
    uint64_t specified_time, uint32_t* frame_index, uint64_t* time_stamp);

  /**
   * @brief Constructor
   */
  SkvPlayLibrary();

  /**
   * @brief Destructor
   */
  virtual ~SkvPlayLibrary();

 private:
  skv_handle* file_handle_;

  /**
   * @brief Calculates the pinhole value.
   * @param[in] (value) Num of width or height.
   * @return Status object.
   */
  float CalculatesPinholeValue(uint32_t num);
};

#endif   // LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAY_LIBRARY_H_
