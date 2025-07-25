/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_FRAME_RECORDER_H_
#define LIB_CORE_RECORD_FRAME_RECORDER_H_

#include "senscord/config.h"

#ifdef SENSCORD_RECORDER

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <sstream>
#include "senscord/osal.h"
#include "senscord/frame.h"
#include "senscord/property_types.h"
#include "senscord/develop/channel_recorder.h"
#include "record/channel_recorder_adapter.h"
#include "util/mutex.h"

namespace senscord {

// pre-definition
class StreamCore;
class SkvRecordLibrary;

/**
 * @brief Adapter class of implemented recorder.
 */
class FrameRecorder : private util::Noncopyable {
 public:
  /**
   * @brief Constructor
   * @param[in] (stream) Parent stream.
   */
  explicit FrameRecorder(StreamCore* stream);

  /**
   * @brief Destructor
   */
  ~FrameRecorder();

  /**
   * @brief Start to record.
   * @param[in] (setting) Recording settings.
   * @return Status object.
   */
  Status Start(const RecordProperty& setting);

  /**
   * @brief Stop to record.
   * @return Status object.
   */
  Status Stop();

  /**
   * @brief Serialize and push the recording frame.
   * @param[in] (frame) Recording frame.
   */
  void PushFrame(Frame* frame);

  /**
   * @brief Monitor and write frames for threading.
   */
  void MonitorFrames();

  /**
   * @brief Get the current settings.
   * @param[out] (setting) Current settings.
   */
  void GetState(RecordProperty* setting) const;

  /**
   * @brief Get the recordable format list.
   * @param[out] (formats) List of formats.
   * @return Status object.
   */
  Status GetRecordableFormats(std::vector<std::string>* formats) const;

 private:
  /**
   * @brief Recorder thread state.
   */
  enum RecordThreadState {
    kRecordThreadReady = 0,
    kRecordThreadRunning,
    kRecordThreadStopping,
  };

  /**
   * @brief Serialized and copied frame data.
   */
  struct SerializedFrame {
    uint64_t sequence_number;   /**< Sequence number of frame */
    uint64_t sent_time;         /**< Time when frame was sent */
    std::map<uint32_t, SerializedChannel> channels;   /**< Channel data */
  };

  /**
   * @brief Create the new recorder adapter.
   * @param[in] (format_name) Recording format type.
   * @param[out] (recorder) New recorder.
   * @return Status object.
   */
  Status CreateRecorderAdapter(
    const std::string& format_name,
    ChannelRecorderAdapter** recorder);

  /**
   * @brief Stop procces to record.
   * @return Status object.
   */
  Status StopProccess();

  /**
   * @brief Remove empty directories.
   */
  void RemoveEmptyDirectory();

  /**
   * @brief Release the recorder.
   * @param[in] (format_name) Recording format type.
   * @param[in] (recorder) Created recorder.
   * @return Status object.
   */
  Status ReleaseRecorder(
    const std::string& format_name, ChannelRecorderAdapter* recorder);

  /**
   * @brief Pop the serialized frame.
   * @param[in] (timeout) Timeout, in nanoseconds. (If '0', not wait)
   * @return Serialized frame.
   */
  SerializedFrame* PopFrame(uint64_t timeout);

  /**
   * @brief Write a frame.
   * @param[in] (frame) Copied and serialized frame.
   */
  void WriteFrame(const SerializedFrame& frame);

  /**
   * @brief Write a channel.
   * @param[in] (sequence_number) Frame sequence number.
   * @param[in] (sent_time) Time when frame was sent.
   * @param[in] (channel) Copied and serialized channel.
   */
  void WriteChannel(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel);

  /**
   * @brief Serialize the frame data.
   * @param[in] (frame) Source frame.
   * @param[out] (serialized) Destination data.
   */
  void CreateSerializedFrame(
    const Frame* frame, SerializedFrame* serialized) const;

  /**
   * @brief Serialize the channel data.
   * @param[in] (channel) Source channel.
   * @param[out] (serialized) Destination data.
   */
  void CreateSerializedChannel(
    const Channel* channel, SerializedChannel* serialized) const;

  /**
   * @brief Append recorders.
   * @param[in] (path) Directory path.
   * @param[in] (formats) The formats of append channels.
   * @return Status object.
   */
  Status AppendRecorders(
      const std::string& path,
      const std::map<uint32_t, std::string>& formats);

  /**
   * @brief Create recorder.
   * @param[in] (path) Directory path.
   * @param[in] (channel_id) Recording channel id.
   * @param[in] (format_name) Recording format type.
   * @return Status object.
   */
  Status CreateRecorder(
      const std::string& path, const uint32_t channel_id,
      const std::string& format_name);

  /**
   * @brief Remove recorders.
   * @param[in] (formats) Channel formats.
   */
  void RemoveRecorders(const std::map<uint32_t, std::string>& formats);

  /**
   * @brief Write the xml file for recording informations.
   * @param[in] (path) Directory path.
   * @return Status object.
   */
  Status WriteInfoFile(const std::string& path) const;

  /**
   * @brief Write the stream properties when started.
   * @param[in] (path) Directory path.
   * @return Status object.
   */
  Status WriteProperties(const std::string& path) const;

#ifdef SENSCORD_RECORDER_SKV
  /**
   * @brief Write the stream properties when started.
   * @param[in] (library) skv library.
   * @return Status object.
   */
  Status WritePropertiesForSkv(SkvRecordLibrary*) const;

  /**
   * @brief Write the skv write property.
   * @param[in] (library) skv library.
   * @return Status object.
   */
  Status WriteSkvWriteProperty(SkvRecordLibrary*) const;
#endif  // SENSCORD_RECORDER_SKV

  /**
   * @brief Write the new binary file.
   * @param[in] (filepath) File path.
   * @param[in] (buffer) Write data.
   * @param[in] (buffer_size) Size of write data.
   * @return Status object.
   */
  Status WriteBinaryFile(
    const std::string& filepath, uint8_t* buffer, size_t buffer_size) const;

  /**
   * @brief Check invalid formats.
   * @param[in] (formats) The formats of request property.
   * @param[out] (detected_skv) The flag indicates the detection of skv format.
   * @return Status object.
   */
  Status CheckInvalidFormats(
      const std::map<uint32_t, std::string>& formats,
      bool* detected_skv) const;

  /**
   * @brief Set the thread state
   * @param[in] (state) The state of recorder thread
   */
  void SetThreadState(RecordThreadState state);

  /**
   * @brief Return whether the thread state matches
   * @param[in] (state) The state of recorder thread
   * @return True if match
   */
  bool IsThreadState(RecordThreadState state) const;

 private:
  StreamCore* stream_;
  RecordProperty setting_;
  uint32_t recorded_count_;
  std::map<uint32_t, ChannelRecorderAdapter*> recorders_;
  util::Mutex mutex_recorders_;
  mutable util::Mutex mutex_state_;

  osal::OSThread* thread_;
  RecordThreadState thread_state_;
  bool is_skv_record_;

  std::queue<SerializedFrame*> frames_;
  util::Mutex mutex_frames_;
  osal::OSCond* cond_frames_;

#ifdef SENSCORD_RECORDER_SKV
  // skv common writer
  SkvRecordLibrary* skv_record_library_;
#endif  // SENSCORD_RECORDER_SKV
};

}  // namespace senscord

#else

#include "senscord/noncopyable.h"
#include "senscord/status.h"

namespace senscord {

class FrameRecorder : private util::Noncopyable {
 public:
  explicit FrameRecorder(void*) {}
  Status Stop() { return Status::OK(); }
  void PushFrame(void*) {}
};

}  // namespace senscord

#endif  // SENSCORD_RECORDER
#endif  // LIB_CORE_RECORD_FRAME_RECORDER_H_
