/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/info_writer.h"
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>    // find
#include "senscord/osal.h"
#include "senscord/develop/recorder_common.h"
#include "record/record_utility.h"

namespace senscord {

/**
 * @brief Write the info file.
 * @param[in] (path) Target xml file name.
 * @param[in] (stream) Parent stream.
 * @return Status object.
 */
Status InfoFileWriter::Write(const std::string& path, Stream* stream) const {
  osal::OSXmlCreator xml;
  int32_t ret = xml.Open(path);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation,
        "failed to create xml file: path=%s, ret=0x%" PRIx32,
        path.c_str(), ret);
  }
  Status status = WriteRecordElement(&xml, stream);
  xml.Close();

  // remove when error occured
  if (!status.ok()) {
    osal::OSRemove(path.c_str());
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Write the record element
 * @param[in] (xml) XNK Creator.
 * @param[in] (stream) Parent stream.
 * @return Status object.
 */
Status InfoFileWriter::WriteRecordElement(
    osal::OSXmlCreator* xml, Stream* stream) const {
  xml->WriteStartElement("record");
  {
    // add date
    osal::OSSystemTime time = {};
    osal::OSGetLocalTime(&time);

    std::stringstream ss;
    ss << ToZeroFilledString(time.year, 4) << '/';
    ss << ToZeroFilledString(time.month, 2) << '/';
    ss << ToZeroFilledString(time.day, 2) << ' ';
    ss << ToZeroFilledString(time.hour, 2) << ':';
    ss << ToZeroFilledString(time.minute, 2) << ':';
    ss << ToZeroFilledString(time.second, 2);
    xml->WriteAttribute("date", ss.str());

    // stream info
    Status status = WriteStreamElement(xml, stream);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    // channels info
    status = WriteChannelsElement(xml, stream);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }
  xml->WriteEndElement();
  return Status::OK();
}

/**
 * @brief Write the stream element
 * @param[in] (xml) XNK Creator.
 * @param[in] (stream) Parent stream.
 * @return Status object.
 */
Status InfoFileWriter::WriteStreamElement(
    osal::OSXmlCreator* xml, Stream* stream) const {
  Status status;
  xml->WriteStartElement("stream");
  {
    // attr (key)
    StreamKeyProperty key = {};
    status = stream->GetProperty(kStreamKeyPropertyKey, &key);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    xml->WriteAttribute("key", key.stream_key);

    // attr (type)
    StreamTypeProperty type = {};
    status = stream->GetProperty(kStreamTypePropertyKey, &type);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    xml->WriteAttribute("type", type.type);

    // framerate
    FrameRateProperty framerate = {};
    status = stream->GetProperty(kFrameRatePropertyKey, &framerate);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    xml->WriteStartElement("framerate");
    xml->WriteAttribute("num", ToString(framerate.num));
    xml->WriteAttribute("denom", ToString(framerate.denom));
    xml->WriteEndElement();  // framerate

    // skipframe
    SkipFrameProperty skipframe = {};
    status  = stream->GetProperty(kSkipFramePropertyKey, &skipframe);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    xml->WriteStartElement("skipframe");
    xml->WriteAttribute("rate", ToString(skipframe.rate));
    xml->WriteEndElement();  // skipframe

    // get property list only recordable
    std::vector<std::string> propertylist;
    status = GetPropertyListOnlyRecording(stream, &propertylist);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    // other properties
    if (propertylist.size() > 0) {
      xml->WriteStartElement("properties");
      std::vector<std::string>::const_iterator itr = propertylist.begin();
      std::vector<std::string>::const_iterator end = propertylist.end();
      for (; itr != end; ++itr) {
        xml->WriteStartElement("property");
        xml->WriteAttribute("key", *itr);
        xml->WriteEndElement();
      }
      xml->WriteEndElement();  // properties
    }
  }
  xml->WriteEndElement();
  return Status::OK();
}

/**
 * @brief Write the channels element
 * @param[in] (xml) XNK Creator.
 * @param[in] (stream) Parent stream.
 * @return Status object.
 */
Status InfoFileWriter::WriteChannelsElement(
    osal::OSXmlCreator* xml, Stream* stream) const {
  Status status;
  xml->WriteStartElement("channels");
  {
    // channel info
    ChannelInfoProperty channelinfo = {};
    status = stream->GetProperty(kChannelInfoPropertyKey, &channelinfo);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    // channel mask
    ChannelMaskProperty channelmask = {};
    status = stream->GetProperty(kChannelMaskPropertyKey, &channelmask);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }

    // channel elements
    std::map<uint32_t, ChannelInfo>::const_iterator itr =
        channelinfo.channels.begin();
    std::map<uint32_t, ChannelInfo>::const_iterator end =
        channelinfo.channels.end();
    for (; itr != end; ++itr) {
      WriteChannelElement(xml, stream, itr->first, itr->second, channelmask);
    }
  }
  xml->WriteEndElement();
  return Status::OK();
}

/**
 * @brief Write the channel element
 * @param[in] (xml) XNK Creator.
 * @param[in] (stream) Parent stream.
 * @param[in] (channel_id) Channel ID.
 * @param[in] (channel_info) Channel Information.
 * @param[in] (channelmask) Channel Mask.
 */
void InfoFileWriter::WriteChannelElement(
    osal::OSXmlCreator* xml, Stream* stream, uint32_t channel_id,
    const ChannelInfo& channel_info,
    const ChannelMaskProperty& channelmask) const {
  xml->WriteStartElement("channel");
  {
    xml->WriteAttribute("id", ToString(channel_id));
    xml->WriteAttribute("type", channel_info.raw_data_type);
    xml->WriteAttribute("description", channel_info.description);

    std::vector<uint32_t>::const_iterator found = std::find(
        channelmask.channels.begin(), channelmask.channels.end(),
        channel_id);
    if (found != channelmask.channels.end()) {
      xml->WriteAttribute("mask", "true");
    }
  }
  xml->WriteEndElement();  // channel
}

/**
 * @brief Get the property list from the stream.
 *        List contains the properties of recording.
 * @param[in] (stream) Target stream.
 * @param[out] (key_list) List of the property keys.
 * @return Status object.
 */
Status InfoFileWriter::GetPropertyListOnlyRecording(
    Stream* stream, std::vector<std::string>* key_list) {
  key_list->clear();
  Status status = stream->GetPropertyList(key_list);
  if (status.ok()) {
    std::vector<std::string>::iterator itr = key_list->begin();
    while (itr != key_list->end()) {
      if (!RecordUtility::IsRecordableProperty(*itr)) {
        itr = key_list->erase(itr);
      } else {
        ++itr;
      }
    }
  }
  return SENSCORD_STATUS_TRACE(status);
}

}   // namespace senscord
