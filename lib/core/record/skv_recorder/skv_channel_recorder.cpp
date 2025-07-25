/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/skv_recorder/skv_channel_recorder.h"

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <map>

#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"
#include "senscord/status.h"
#include "senscord/logger.h"
#include "record/skv_recorder/skv_record_library_manager.h"

namespace senscord {

/**
 * @brief Constructor
 */
SkvChannelRecorder::SkvChannelRecorder() : skv_record_library_(NULL) {}

/**
 * @brief Destructor
 */
SkvChannelRecorder::~SkvChannelRecorder() {}

/**
 * @brief Write the channel.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Serialized channel data.
 * @return Status object.
 */
Status SkvChannelRecorder::Write(
    uint64_t sequence_number,
    uint64_t sent_time,
    const SerializedChannel& channel) {
  // invalid data (skip)
  if (channel.rawdata.size() == 0) {
    return Status::OK();
  }

  // write rawdata to skv file
  Status status = WriteRawData(sent_time, channel);
  if (status.ok()) {
    // write property to skv file
    status = WriteProperty(sequence_number, sent_time, channel);
  }

  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get the CameraCalibrationProperty from stream.
 * @param[in] (library) Skv record library.
 * @param[out] (parameter) CameraCalibrationParameters of stream.
 * @return Status object.
 */
Status SkvChannelRecorder::GetCameraCalibrationParameter(
    SkvRecordLibrary* library, CameraCalibrationParameters* parameter) const {
  // Get parent stream.
  Status status;
  Stream* stream = NULL;
  {
    SkvRecordLibraryManager* manager = SkvRecordLibraryManager::GetInstance();
    status = manager->GetStreamFromLibrary(library, &stream);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }

  // Get camera calibration property.
  CameraCalibrationProperty prop = {};
  status = stream->GetProperty(kCameraCalibrationPropertyKey, &prop);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Get camera calibration parameter.
  std::map<uint32_t, CameraCalibrationParameters>::const_iterator itr =
      prop.parameters.begin();
  if (itr == prop.parameters.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "Non camera calibration parameter");
  }

  *parameter = itr->second;

  return Status::OK();
}

/**
 * @brief Get the buffer size of serialized channel properties.
 * @param[in] (channel) Serialized channel data.
 * @return Buffer size. (1024Bytes * n)
 */
size_t SkvChannelRecorder::GetChannelPropertyBufferSize(
    const SerializedChannel& channel) {
  // set write data
  ChannelPropertiesForRecord rec = {};
  rec.properties = channel.properties;

  // serialize
  serialize::SerializedBuffer buf;
  serialize::Encoder enc(&buf);
  enc.Push(rec);

  size_t buffer_size = 0;
  do {
    buffer_size += kPropertySizeBase;
  } while (buf.size() > buffer_size);

  return buffer_size;
}

}   // namespace senscord
