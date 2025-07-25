/*
 * SPDX-FileCopyrightText: 2020-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_PLAYER_SRC_PLAYER_STREAM_FILE_MANAGER_H_
#define LIB_COMPONENT_PLAYER_SRC_PLAYER_STREAM_FILE_MANAGER_H_

#include <inttypes.h>
#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "./player_common.h"
#include "./player_component_types.h"
#include "senscord/develop/component.h"
#include "senscord/develop/recorder_common.h"
#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"

/**
 * @brief Managing stream files.
 */
class PlayerStreamFileManager {
 public:
  /**
   * @brief Constructor.
   */
  PlayerStreamFileManager();

  /**
   * @brief Destructor.
   */
  ~PlayerStreamFileManager();

  /**
   * @brief Read the xml info from record file.
   * @param[in] (target_path) Target path of play property.
   * @return Status object.
   */
  senscord::Status ReadStreamFile(const std::string& target_path);

  /**
   * @brief Set stream property
   * @param[in] (key) The key of property
   * @param[in] (serialized_property) The serialized property
   * @param[in] (serialized_size) The size of serialized property
   * @return status object
   */
  senscord::Status SetStreamProperty(
    const std::string& key, const void* serialized_property,
    size_t serialized_size);

  /**
   * @brief Get stream property
   * @param[in] (key) The key of property
   * @return The pointer of stream property data.
   *         If NULL, the data is not find
   */
  const std::vector<uint8_t>* GetStreamProperty(const std::string& key);

  /**
   * @brief Get key list of stream property.
   * @param[out] (key_list) The list of stream properties.
   * @return Status object.
   */
  senscord::Status GetStreamPropertyList(std::vector<std::string>* key_list);

  /**
   * @brief Clear for stream property.
   */
  void ClearStreamProperty();

  /**
   * @brief Get the properties of xml info.
   * @param[out] (prop) Frame rate property.
   */
  void GetFrameRate(senscord::FrameRateProperty* prop);

  /**
   * @brief Get stream type
   * @return The type of stream
   */
  const std::string& GetStreamType();

  /**
   * @brief Get the properties of xml info.
   * @param[out] (property) PlayFileInfo property.
   */
  void GetPlayFileInfo(senscord::PlayFileInfoProperty* property);

  /**
   * @brief Get the parameter of info.xml.
   * @param[out] (channels) The parameter of info.xml.
   */
  void GetInfoXmlChannels(InfoXmlChannelList* channels);

 private:
  // The analysis result of info.xml
  InfoXmlParameter info_xml_;

  // XML Parser Class for info.xml
  senscord::osal::OSXmlParser* parser_;

  // stream property
  std::vector<std::string> property_key_list_;
  typedef std::vector<uint8_t> StreamPropertyData;
  typedef std::map<std::string, StreamPropertyData> StreamPropertyList;
  StreamPropertyList property_list_;

  /**
   * @brief Read the xml info from record file.
   * @param[in] (target_path) Target path of play property.
   * @param[out] (info_xml) The analysis result of info.xml.
   * @return Status object.
   */
  senscord::Status ReadXmlInfo(
      const std::string& target_path, InfoXmlParameter* info_xml);

  /**
   * @brief Parse record element and obtain it as info.xml.
   * @param[out] (info_xml) The analysis result of info.xml.
   * @return Status object.
   */
  senscord::Status ParseRecord(InfoXmlParameter* info_xml);

  /**
   * @brief Read the stream property from record file.
   * @param[in] (target_path) Target path of play property.
   * @param[in] (key_list) The list of stream property key.
   * @return Status object.
   */
  senscord::Status ReadStreamProperty(
      const std::string& target_path, const std::vector<std::string>& key_list);

  /**
   * @brief Parse stream element and obtain it as info.xml.
   * @param[out] (stream) The stream information of info.xml
   * @return Status object.
   */
  senscord::Status ParseStream(InfoXmlStreamInfo* stream);

  /**
   * @brief Parse properties element and obtain it as info xml.
   * @param[out] (property_keys) The list of stream property key of info.xml
   * @return Status object.
   */
  senscord::Status ParseProperties(std::vector<std::string>* property_keys);

  /**
   * @brief Parse stream element and obtain it as Config.
   * @param[out] (channel) The channel information of info.xml
   * @return Status object.
   */
  senscord::Status ParseChannels(InfoXmlChannelList* channels);
};
#endif  // LIB_COMPONENT_PLAYER_SRC_PLAYER_STREAM_FILE_MANAGER_H_
