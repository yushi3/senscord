/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_INFO_WRITER_H_
#define LIB_CORE_RECORD_INFO_WRITER_H_

#include <string>
#include <vector>
#include "senscord/noncopyable.h"
#include "senscord/osal.h"
#include "senscord/status.h"
#include "senscord/stream.h"
#include "senscord/property_types.h"

namespace senscord {

/**
 * @brief Writer for the record infromation xml file.
 */
class InfoFileWriter : private util::Noncopyable {
 public:
  /**
   * @brief Write the info file.
   * @param[in] (path) Target xml file name.
   * @param[in] (stream) Parent stream.
   * @return Status object.
   */
  Status Write(const std::string& path, Stream* stream) const;

  /**
   * @brief Get the property list from the stream.
   *        List contains the properties of recording.
   * @param[in] (stream) Target stream.
   * @param[out] (key_list) List of the property keys.
   * @return Status object.
   */
  static Status GetPropertyListOnlyRecording(
    Stream* stream, std::vector<std::string>* key_list);

  /**
   * @brief Constructor
   */
  InfoFileWriter() {}

  /**
   * @brief Destructor
   */
  ~InfoFileWriter() {}

 private:
  /**
   * @brief Write the record element
   * @param[in] (xml) XNK Creator.
   * @param[in] (stream) Parent stream.
   * @return Status object.
   */
  Status WriteRecordElement(osal::OSXmlCreator* xml, Stream* stream) const;

  /**
   * @brief Write the stream element
   * @param[in] (xml) XNK Creator.
   * @param[in] (stream) Parent stream.
   * @return Status object.
   */
  Status WriteStreamElement(osal::OSXmlCreator* xml, Stream* stream) const;

  /**
   * @brief Write the channels element
   * @param[in] (xml) XNK Creator.
   * @param[in] (stream) Parent stream.
   * @return Status object.
   */
  Status WriteChannelsElement(osal::OSXmlCreator* xml, Stream* stream) const;

  /**
   * @brief Write the channel element
   * @param[in] (xml) XNK Creator.
   * @param[in] (stream) Parent stream.
   * @param[in] (channel_id) Channel ID.
   * @param[in] (channel_info) Channel Information.
   * @param[in] (channelmask) Channel Mask.
   */
  void WriteChannelElement(
    osal::OSXmlCreator* xml, Stream* stream,
    uint32_t channel_id, const ChannelInfo& channel_info,
    const ChannelMaskProperty& channelmask) const;
};

}   // namespace senscord
#endif    // LIB_CORE_RECORD_INFO_WRITER_H_
