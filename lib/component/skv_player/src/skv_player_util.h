/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAYER_UTIL_H_
#define LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAYER_UTIL_H_

#include <map>
#include <vector>
#include <string>
#include "senscord/develop/stream_source.h"
#include "senscord/property_types.h"
#include "src/skv_play_library.h"

/**
 * Skv Player Namespace
 */
namespace skv_player {
/**
* Stream Properties
*/
typedef std::map<std::string, senscord::BinaryProperty>
    SerializedStreamProperties;

// Channel id
const uint32_t kChannelIdDepth           = 0;
const uint32_t kChannelIdConfidence      = 1;
const uint32_t kChannelIdPointCloud      = 2;
const uint32_t kChannelIdDepthFloat      = 3;
const uint32_t kChannelIdConfidenceFloat = 4;
const uint32_t kChannelIdPointCloudFloat = 5;
const uint32_t kChannelIdRawData         = 0x80000000;
const uint32_t kChannelIdRawDataSecond   = 0x80000001;

// Default values
// image property
const uint32_t kDefaultWidth = 640;
const uint32_t kDefaultHeight = 480;
const uint32_t kDefaultStrideBytes = 2560;
// depth property
const float kDefaultScale = 0.001F;
const float kDefaultDepthMinRange = 0;
const float kDefaultDepthMaxRange = 32000;
// channel info property
const char kDefaultDescriptionDepth[]      = "Depth Map in millimeters";
const char kDefaultDescriptionConfidence[] = "Confidence Data";
const char kDefaultDescriptionPointCloud[] =
    "The cartesian position in millimeters of the pixel along the xyz axis";
const char kDefaultDescriptionRawDataFirst[]  = "1st Raw Quad Data";
const char kDefaultDescriptionRawDataSecond[] = "2nd Raw Quad Data";
// frame rate property
const uint32_t kDefaultFrameRateNum   = 60;
const uint32_t kDefaultFrameRateDenom = 1;
// temperature property
const uint32_t kLaserTemperatureId  = 0;
const uint32_t kSensorTemperatureId = 1;
const char kDefaultDescriptionLaserTemperature[]  = "laser temperature";
const char kDefaultDescriptionSensorTemperature[] = "sensor temperature";

// point-cloud property
const uint32_t kPointCloudXyzPlane = 3;
const uint32_t kPointCloudXyz32FloatBpp = 4;
const uint32_t kPointCloudXyz16Bpp = 2;

// image property (RawQuad channel)
const uint32_t kRawQuadAminusBBpp = 2;
const uint32_t kRawQuadAandBBpp   = 4;
const uint32_t kRawQuadAminusBStrideBytes = kDefaultWidth * kRawQuadAminusBBpp;
const uint32_t kRawQuadAandBStrideBytes = kDefaultWidth * kRawQuadAandBBpp;

// Environment variable of compression library.
const char kHdf5PluginName[] = "h5lz4";
const char kHdf5PluginPathEnvStr[] = "HDF5_PLUGIN_PATH";

/**
 * @brief Get channel info property from skv stream.
 * @param[in] (target_name) The name of target skv stream.
 * @param[in] (skv_stream_list) The information of skv stream.
 * @param[out] (prop) channel info property.
 * @return Status object.
 */
senscord::Status GetChannelInfoPropertyFromSkvStream(
    const std::vector<std::string>& target_names,
    const std::map<std::string, SkvStreamInfo>& skv_stream_list,
    senscord::ChannelInfoProperty* prop);

/**
 * @brief Encode the property from deserialized to BinaryProperty.
 * @param[in] (deserialized) The property of deserialized.
 * @param[out] (serialized) The binary property.
 * @return Status object.
 */
template <typename T>
senscord::Status EncodeDeserializedProperty(
    const T& deserialized, senscord::BinaryProperty* serialized) {
  // Encode
  senscord::serialize::SerializedBuffer buffer;
  senscord::serialize::Encoder encoder(&buffer);
  senscord::Status status = encoder.Push(deserialized);
  if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
  }

  // Set binary property
  buffer.swap(&serialized->data);

  return senscord::Status::OK();
}

/**
 * @brief Decode the property from BinaryProperty to deserialized.
 * @param[in] (deserialized) The binary property.
 * @param[out] (serialized) The property of deserialized.
 * @return Status object.
 */
template <typename T>
senscord::Status DecodeSerializedProperty(
    const senscord::BinaryProperty& serialized, T* deserialized) {
  // Deserialize property
  senscord::serialize::Decoder decoder(
      &serialized.data[0], serialized.data.size());
  senscord::Status status = decoder.Pop(*deserialized);
  if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
  }

  return senscord::Status::OK();
}
}  // namespace skv_player

#endif  // LIB_COMPONENT_SKV_PLAYER_SRC_SKV_PLAYER_UTIL_H_
