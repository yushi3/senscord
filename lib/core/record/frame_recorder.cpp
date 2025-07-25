/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/frame_recorder.h"
#include <string>
#include <vector>
#include <map>
#include <utility>    // make_pair

#include "senscord/status.h"
#include "senscord/osal.h"
#include "senscord/develop/recorder_common.h"
#include "logger/logger.h"
#include "util/autolock.h"
#include "stream/stream_core.h"
#include "frame/frame_core.h"
#include "record/record_utility.h"
#include "record/info_writer.h"
#include "record/recorder_manager.h"
#ifdef SENSCORD_RECORDER_RAW
#include "record/raw_recorder.h"
#include "record/composite_raw_recorder.h"
#endif  // SENSCORD_RECORDER_RAW
#ifdef SENSCORD_RECORDER_SKV
#include "senscord/develop/property_types_rosemary.h"
#include "record/skv_recorder/skv_recorder.h"
#include "record/skv_recorder/skv_record_library_manager.h"
#endif  // SENSCORD_RECORDER_SKV

namespace {

// record event
enum RecordStateForEvent {
  kRecordStopped = 0,
  kRecordStarted,  // not used
};

// 100 ms
const uint64_t kMonitorThreadTimeout = 100 * 1000 * 1000;

/**
 * @brief Thread for frame writing.
 * @param[in] (arg) Adapter for frame recorder.
 */
senscord::osal::OSThreadResult FrameWriteThread(void* arg) {
  senscord::FrameRecorder* adapter =
      reinterpret_cast<senscord::FrameRecorder*>(arg);
  if (adapter != NULL) {
    adapter->MonitorFrames();
  }
  return 0;
}

}  // namespace

namespace senscord {

/**
 * @brief Constructor
 * @param[in] (stream) Parent stream.
 */
FrameRecorder::FrameRecorder(StreamCore* stream)
    : stream_(stream), setting_(), recorded_count_(), thread_(),
      thread_state_(kRecordThreadReady),
      is_skv_record_(false) {
#ifdef SENSCORD_RECORDER_SKV
  skv_record_library_ = NULL;
#endif
  osal::OSCreateCond(&cond_frames_);
}

/**
 * @brief Destructor
 */
FrameRecorder::~FrameRecorder() {
  osal::OSDestroyCond(cond_frames_);
}

/**
 * @brief Start to record.
 * @param[in] (setting) Recording settings.
 * @return Status object.
 */
Status FrameRecorder::Start(const RecordProperty& setting) {
  util::AutoLock autolock(&mutex_state_);
  if (IsThreadState(kRecordThreadStopping)) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseBusy,
        "Recorder is busy (Thread stopping)");
  }

  bool detected_skv = is_skv_record_;
  Status status = CheckInvalidFormats(setting.formats, &detected_skv);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  if (IsThreadState(kRecordThreadRunning)) {
    std::map<uint32_t, std::string> append_formats;
    status = RecordPropertyUtility::GetAppendFormat(
        setting_, setting, &append_formats);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    if (!append_formats.empty()) {
      status = AppendRecorders(setting_.path, append_formats);
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
      // Apply appended format of channels.
      setting_.formats.swap(append_formats);  // swap & insert -> overwrite
      setting_.formats.insert(append_formats.begin(), append_formats.end());
    }
    return Status::OK();
  }

  // apply temporary property
  RecordProperty property = setting;
  if (property.buffer_num == 0) {
    ++property.buffer_num;
  }
  if (property.path.empty()) {
    property.path = ".";    // current
  }

  // top directory path
  status = RecordPropertyUtility::CreateTopDirectory(
      &property, stream_->GetKey());
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  if (!detected_skv) {
    // create info.xml
    status = WriteInfoFile(property.path);
    if (!status.ok()) {
      // remove but if directory is not empty, will be failed.
      osal::OSRemoveDirectory(property.path.c_str());
      return SENSCORD_STATUS_TRACE(status);
    }

    // write stream properties
    status = WriteProperties(property.path);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  } else {
#ifdef SENSCORD_RECORDER_SKV
    std::string stream_type = stream_->GetType();
    if (stream_type != kStreamTypeDepth) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseNotSupported,
          "Unsupported stream type : %s", stream_type.c_str());
    }

    // setup library manager.
    SkvRecordLibraryManager* manager = SkvRecordLibraryManager::GetInstance();
    status = manager->Init();

    // create skv record library.
    SkvRecordLibrary* library = NULL;
    if (status.ok()) {
      status = manager->CreateSkvRecordLibrary(stream_, &library);
    }

    // create skv file.
    if ((status.ok()) && (library != NULL)) {
      const char file_name[] = "senscord_data.skv";
      property.path += osal::kDirectoryDelimiter;
      property.path += file_name;
      status = library->CreateFile(property.path);
    }

    // write stream properties
    if ((status.ok()) && (library != NULL)) {
      status = WritePropertiesForSkv(library);
    }

    // write skv write property
    if ((status.ok()) && (library != NULL)) {
      status = WriteSkvWriteProperty(library);
      if (!status.ok()) {
        SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
        // This Property is not mandatory,
        // so the recording process will continue.
        status = Status::OK();
      }
    }

    if ((!status.ok()) || (library == NULL)) {
      // roll-back
      if (library != NULL) {
        library->CloseFile();
        manager->ReleaseSkvRecordLibrary(library);
      }
      return SENSCORD_STATUS_TRACE(status);
    }

    // keep record library.
    skv_record_library_ = library;
#endif  // SENSCORD_RECORDER_SKV
  }

  // instantiate the implemented recorders
  status = AppendRecorders(property.path, property.formats);
  if (status.ok()) {
    setting_ = property;
    // start threading
    SetThreadState(kRecordThreadRunning);
    int32_t ret = osal::OSCreateThread(&thread_, FrameWriteThread, this, NULL);
    if (ret != 0) {
      SetThreadState(kRecordThreadReady);
      RemoveRecorders(property.formats);
      status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidOperation,
          "failed to create thread: %" PRIx32, ret);
    }
  }

  if (!status.ok()) {
    setting_ = RecordProperty();
    if (detected_skv) {
#ifdef SENSCORD_RECORDER_SKV
      skv_record_library_->CloseFile();
      SkvRecordLibraryManager* manager = SkvRecordLibraryManager::GetInstance();
      manager->ReleaseSkvRecordLibrary(skv_record_library_);
      skv_record_library_ = NULL;
#endif  // SENSCORD_RECORDER_SKV
    }
    return SENSCORD_STATUS_TRACE(status);
  }

  // apply skv record flag
  is_skv_record_ = detected_skv;

  return Status::OK();
}

/**
 * @brief Stop to record.
 * @return Status object.
 */
Status FrameRecorder::Stop() {
  osal::OSThread* thread = NULL;
  {
    util::AutoLock autolock(&mutex_state_);
    if (thread_ == NULL) {
      return Status::OK();  // already stopped
    }
    thread = thread_;
    thread_ = NULL;

    if (IsThreadState(kRecordThreadRunning)) {
      SetThreadState(kRecordThreadStopping);
    }
  }
  {
    util::AutoLock frames_lock(&mutex_frames_);
    osal::OSSignalCond(cond_frames_);
  }

  osal::OSJoinThread(thread, NULL);

  return Status::OK();
}

/**
 * @brief Stop procces to record.
 * @return Status object.
 */
Status FrameRecorder::StopProccess() {
  RemoveEmptyDirectory();
  // all stop force
  RemoveRecorders(setting_.formats);

  if (is_skv_record_) {
#ifdef SENSCORD_RECORDER_SKV
    // close skv file.
    Status status = skv_record_library_->CloseFile();
    if (!status.ok()) {
      SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
      // Continue the stop process.
    }

    // release skv library
    SkvRecordLibraryManager* manager = SkvRecordLibraryManager::GetInstance();
    status = manager->ReleaseSkvRecordLibrary(skv_record_library_);
    if (!status.ok()) {
      SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
      // Continue the stop process.
    }
    is_skv_record_ = false;
#endif  // SENSCORD_RECORDER_SKV
  }

  // reset settings
  setting_ = RecordProperty();
  recorded_count_ = 0;

  return Status::OK();
}

/**
 * @brief Remove empty directories.
 */
void FrameRecorder::RemoveEmptyDirectory() {
  if (is_skv_record_) {
    return;   // do nothing
  }
  for (std::map<uint32_t, ChannelRecorderAdapter*>::iterator itr =
      recorders_.begin(); itr != recorders_.end(); ++itr) {
    std::string channel_dir_path;
    RecordUtility::GetChannelDirectoryName(itr->first, &channel_dir_path);
    channel_dir_path =
        setting_.path + osal::kDirectoryDelimiter + channel_dir_path;
    std::vector<std::string> file_list;
    int32_t ret = osal::OSGetRegularFileList(channel_dir_path, &file_list);
    if (ret == 0) {
      if (file_list.empty()) {
        osal::OSRemoveDirectory(channel_dir_path.c_str());
      }
    } else {
      SENSCORD_LOG_WARNING(
          "failed to get file list: ret=0x%" PRIx32 ", path=%s",
          ret, channel_dir_path.c_str());
    }
  }
}

/**
 * @brief Serialize and push the recording frame.
 * @param[in] (frame) Recording frame.
 */
void FrameRecorder::PushFrame(Frame* frame) {
  uint32_t buffer_num = 0;
  {
    util::AutoLock autolock(&mutex_state_);
    if (!IsThreadState(kRecordThreadRunning)) {
      return;
    }
    buffer_num = setting_.buffer_num;
  }

  {
    util::AutoLock frames_lock(&mutex_frames_);
    if (frames_.size() >= buffer_num) {
      // buffer overflow
      SENSCORD_LOG_DEBUG("recording buffer is full: %s",
          stream_->GetKey().c_str());
      return;
    }

    // serialize and hold the frame data.
    SerializedFrame* serialized = new SerializedFrame();
    CreateSerializedFrame(frame, serialized);
    frames_.push(serialized);

    // wakeup monitor
    osal::OSSignalCond(cond_frames_);
  }

  // turn on the accessed flag.
  {
    FrameCore* frame_core = static_cast<FrameCore*>(frame);
    frame_core->NotifyRecorded();
  }
}

/**
 * @brief Get the current settings.
 * @param[out] (setting) Current settings.
 */
void FrameRecorder::GetState(RecordProperty* setting) const {
  util::AutoLock autolock(&mutex_state_);
  *setting = setting_;
  setting->count = recorded_count_;
}

/**
 * @brief Get the recordable format list.
 * @param[out] (formats) List of formats.
 * @return Status object.
 */
Status FrameRecorder::GetRecordableFormats(
    std::vector<std::string>* formats) const {
  if (formats == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  formats->clear();

  Status status;
#ifdef SENSCORD_RECORDER_RAW
  // add standard formats
  formats->push_back(kRecordingFormatRaw);
  formats->push_back(kRecordingFormatCompositeRaw);
#endif  // SENSCORD_RECORDER_RAW
#ifdef SENSCORD_RECORDER_SKV
  formats->push_back(kRecordingFormatSkv);
#endif  // SENSCORD_RECORDER_SKV
#ifdef SENSCORD_RECORDER_LOADER
  // add user formats
  RecorderManager* manager = RecorderManager::GetInstance();
  status = manager->GetRecordableFormats(formats);
  SENSCORD_STATUS_TRACE(status);
#endif  // SENSCORD_RECORDER_LOADER
  return status;
}

/**
 * @brief  Pop the serialized frame.
 * @param[in] (timeout) Timeout, in nanoseconds. (If '0', not wait)
 * @return Serialized frame.
 */
FrameRecorder::SerializedFrame* FrameRecorder::PopFrame(uint64_t timeout) {
  util::AutoLock autolock(&mutex_frames_);
  SerializedFrame* frame = NULL;
  if (!frames_.empty()) {
    frame = frames_.front();
    frames_.pop();
  } else if (timeout > 0) {
    // if no frames, wait to push or record stop.
    int32_t ret = osal::OSRelativeTimedWaitCond(
        cond_frames_, mutex_frames_.GetObject(), timeout);
    if (osal::error::IsError(ret) && !osal::error::IsTimeout(ret)) {
      SENSCORD_LOG_ERROR("Thread wait error: ret=0x%" PRIx32, ret);
    }
  }
  return frame;
}

/**
 * @brief Monitor and write frames for threading.
 */
void FrameRecorder::MonitorFrames() {
  std::string stream_key = stream_->GetKey();
  SENSCORD_LOG_DEBUG("start the recoding thread: %s", stream_key.c_str());

  uint32_t count = 0;
  uint32_t max_count = 0;
  std::string path;
  {
    util::AutoLock autolock(&mutex_state_);
    max_count = setting_.count;
    path = setting_.path;
  }

  while (1) {
    // Get the frame from queue
    SerializedFrame* frame = PopFrame(kMonitorThreadTimeout);
    if (frame != NULL) {
      WriteFrame(*frame);
      ++count;
      delete frame;
    }

    util::AutoLock autolock(&mutex_state_);
    // Updated the number of recorded frames
    recorded_count_ = count;
    if ((max_count != 0) && (count >= max_count)) {
      SetThreadState(kRecordThreadStopping);
    }
    if (!IsThreadState(kRecordThreadRunning)) {
      break;
    }
  }

  // release the remaining frames.
  while (1) {
    SerializedFrame* frame = PopFrame(0);
    if (frame == NULL) {
      break;
    }
    if (count < max_count) {
      WriteFrame(*frame);
      ++count;
    }
    delete frame;
  }

  // send event
  EventArgument args;
  args.Set(kEventArgumentRecordState, kRecordStopped);
  args.Set(kEventArgumentRecordCount, count);
  args.Set(kEventArgumentRecordPath, path);
  stream_->SendEvent(kEventRecordState, args);

  {
    util::AutoLock autolock(&mutex_state_);
    if ((max_count != 0) && (thread_ != NULL)) {
      // If the number of recording frames is specified, detach the thread.
      int32_t ret = osal::OSDetachThread(thread_);
      if (ret == 0) {
        thread_ = NULL;
      } else {
        SENSCORD_LOG_WARNING(
            "failed to detach the recording thread (0x%" PRIx32 ")", ret);
      }
    }
    StopProccess();
    SetThreadState(kRecordThreadReady);
  }

  SENSCORD_LOG_DEBUG("stop the recoding thread: %s", stream_key.c_str());
}

/**
 * @brief Write a frame.
 * @param[in] (frame) Copied and serialized frame.
 */
void FrameRecorder::WriteFrame(const SerializedFrame& frame) {
  for (std::map<uint32_t, SerializedChannel>::const_iterator
      itr = frame.channels.begin(), end = frame.channels.end();
      itr != end; ++itr) {
    WriteChannel(frame.sequence_number, frame.sent_time, itr->second);
  }
}

/**
 * @brief Write a channe;.
 * @param[in] (sequence_number) Frame sequence number.
 * @param[in] (sent_time) Time when frame was sent.
 * @param[in] (channel) Copied and serialized channel.
 */
void FrameRecorder::WriteChannel(
    uint64_t sequence_number, uint64_t sent_time,
    const SerializedChannel& channel) {
  util::AutoLock autolock(&mutex_recorders_);
  std::map<uint32_t, ChannelRecorderAdapter*>::const_iterator itr =
      recorders_.find(channel.id);
  if (itr == recorders_.end()) {
    // no recording
    return;
  }
  ChannelRecorderAdapter* recorder = itr->second;

  // fault tolerant
  if (recorder->IsOccuredWriteError()) {
    return;
  }

  // write
  Status status = recorder->Write(sequence_number, sent_time, channel);
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);
    SENSCORD_LOG_WARNING("failed to record channel: %s",
        status.ToString().c_str());
  }
}

/**
 * @brief Serialize the frame data.
 * @param[in] (frame) Source frame.
 * @param[out] (serialized) Destination data.
 */
void FrameRecorder::CreateSerializedFrame(
    const Frame* frame, SerializedFrame* serialized) const {
  // sequence number, timestamp
  frame->GetSequenceNumber(&serialized->sequence_number);
  frame->GetSentTime(&serialized->sent_time);

  // each channels
  ChannelList list;
  frame->GetChannelList(&list);
  ChannelList::const_iterator itr = list.begin();
  ChannelList::const_iterator end = list.end();
  for (; itr != end; ++itr) {
    // channel id
    uint32_t channel_id = 0;
    itr->second->GetChannelId(&channel_id);

    // serialize
    SerializedChannel* serialized_channel = &serialized->channels[channel_id];
    CreateSerializedChannel(itr->second, serialized_channel);
  }
}

/**
 * @brief Serialize the channel data.
 * @param[in] (channel) Source channel.
 * @param[out] (serialized) Destination data.
 */
void FrameRecorder::CreateSerializedChannel(
    const Channel* channel, SerializedChannel* serialized) const {
  channel->GetChannelId(&serialized->id);

  // copy rawdata
  Channel::RawData rawdata = {};
  channel->GetRawData(&rawdata);
  serialized->timestamp = rawdata.timestamp;
  serialized->type = rawdata.type;
  if ((rawdata.address != NULL) && (rawdata.size > 0)) {
    serialized->rawdata.resize(rawdata.size);
    osal::OSMemcpy(&serialized->rawdata[0], serialized->rawdata.size(),
        rawdata.address, rawdata.size);
  }

  // copy properties
  std::vector<std::string> key_list;
  channel->GetPropertyList(&key_list);
  std::vector<std::string>::const_iterator itr = key_list.begin();
  std::vector<std::string>::const_iterator end = key_list.end();
  for (; itr != end; ++itr) {
    if (!RecordUtility::IsRecordableChannelProperty(*itr)) {
      continue;
    }
    BinaryProperty binary = {};
    Status status = channel->GetProperty(*itr, &binary);
    if (!status.ok()) {
      SENSCORD_LOG_DEBUG(
          "failed to channel[0x%" PRIx32 "]GetProperty(%s): %s",
          serialized->id, itr->c_str(), status.ToString().c_str());
    } else {
      serialized->properties.insert(std::make_pair(*itr, binary));
    }
  }

  // get updated list
  channel->GetUpdatedPropertyList(&serialized->updated_property_keys);
}

/**
 * @brief Append recorders.
 * @param[in] (path) Directory path.
 * @param[in] (formats) The formats of append channels.
 * @return Status object.
 */
Status FrameRecorder::AppendRecorders(
    const std::string& path,
    const std::map<uint32_t, std::string>& formats) {
  Status status;
  std::map<uint32_t, std::string>::const_iterator itr = formats.begin();
  std::map<uint32_t, std::string>::const_iterator end = formats.end();
  for (; itr != end; ++itr) {
    status = CreateRecorder(path, itr->first, itr->second);
    if (!status.ok()) {
      SENSCORD_STATUS_TRACE(status);
      break;
    }
  }

  if (!status.ok()) {
    // roll-back
    RemoveRecorders(formats);
  }
  return status;
}

/**
 * @brief Create recorder.
 * @param[in] (path) Directory path.
 * @param[in] (channel_id) Recording channel id.
 * @param[in] (format_name) Recording format type.
 * @return Status object.
 */
Status FrameRecorder::CreateRecorder(
    const std::string& path, const uint32_t channel_id,
    const std::string& format_name) {
  Status status;
  // create recorder
  ChannelRecorderAdapter* adapter = NULL;
  status = CreateRecorderAdapter(format_name, &adapter);
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);
    return status;
  }

  // output path (actually creating directory when writing)
  std::string channel_dir_path = "";
  if (format_name != kRecordingFormatSkv) {
    RecordUtility::GetChannelDirectoryName(channel_id, &channel_dir_path);
    channel_dir_path = path + osal::kDirectoryDelimiter + channel_dir_path;
  }

  // start to record
  status = adapter->Start(channel_dir_path, format_name, stream_);
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);

    // roll-back
    Status rel_status = ReleaseRecorder(format_name, adapter);
    if (!rel_status.ok()) {
      SENSCORD_STATUS_TRACE(rel_status);
      SENSCORD_LOG_WARNING("%s", rel_status.ToString().c_str());
    }
    return status;
  }

  // registration
  util::AutoLock autolock(&mutex_recorders_);
  if (!recorders_.insert(std::make_pair(channel_id, adapter)).second) {
    // roll-back
    Status rel_status = ReleaseRecorder(format_name, adapter);
    if (!rel_status.ok()) {
      SENSCORD_LOG_WARNING("%s", rel_status.ToString().c_str());
    }
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAlreadyExists,
        "Already registered channel: id=%" PRIu32, channel_id);
  }

  return Status::OK();
}

/**
 * @brief Remove recorders.
 * @param[in] (formats) The formats of remove channels..
 */
void FrameRecorder::RemoveRecorders(
    const std::map<uint32_t, std::string>& formats) {
  util::AutoLock autolock(&mutex_recorders_);
  std::map<uint32_t, std::string>::const_iterator itr = formats.begin();
  std::map<uint32_t, std::string>::const_iterator end = formats.end();
  for (; itr != end; ++itr) {
    std::map<uint32_t, ChannelRecorderAdapter*>::iterator found =
        recorders_.find(itr->first);
    if (found != recorders_.end()) {
      Status status = ReleaseRecorder(itr->second, found->second);
      if (!status.ok()) {
        SENSCORD_STATUS_TRACE(status);
        SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
      }
      // pop
      recorders_.erase(found);
    }
  }
}

/**
 * @brief Create the new recorder adapter.
 * @param[in] (format_name) Recording format type.
 * @param[out] (recorder) New recorder.
 * @return Status object.
 */
Status FrameRecorder::CreateRecorderAdapter(
    const std::string& format_name, ChannelRecorderAdapter** recorder) {
  Status status;
  ChannelRecorder* origin = NULL;

#ifdef SENSCORD_RECORDER_RAW
  if (format_name == kRecordingFormatRaw) {
    // standard recorder
    origin = new RawRecorder();
  } else if (format_name == kRecordingFormatCompositeRaw) {
    // standard recorder
    origin = new CompositeRawRecorder();
  }
#endif  // SENSCORD_RECORDER_RAW
#ifdef SENSCORD_RECORDER_SKV
  if (origin == NULL && format_name == kRecordingFormatSkv) {
    // standard recorder
    origin = new SkvRecorder(skv_record_library_);
  }
#endif  // SENSCORD_RECORDER_SKV
  if (origin == NULL) {
#ifdef SENSCORD_RECORDER_LOADER
    // user recorder
    RecorderManager* manager = RecorderManager::GetInstance();
    status = manager->CreateRecorder(format_name, &origin);
    SENSCORD_STATUS_TRACE(status);
#else
    status = SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseNotSupported,
        "Unsupported recorder: format_name=%s", format_name.c_str());
#endif  // SENSCORD_RECORDER_LOADER
  }

  // create the adapter.
  if (status.ok()) {
    *recorder = new ChannelRecorderAdapter(origin);
  }
  return status;
}

/**
 * @brief Release the recorder.
 * @param[in] (format_name) Recording format type.
 * @param[in] (recorder) Created recorder.
 * @return Status object.
 */
Status FrameRecorder::ReleaseRecorder(
    const std::string& format_name, ChannelRecorderAdapter* recorder) {
  Status status;
  recorder->Stop();
  ChannelRecorder* origin = recorder->GetOrigin();

#ifdef SENSCORD_RECORDER_RAW
  if ((format_name == kRecordingFormatRaw) ||
      (format_name == kRecordingFormatCompositeRaw)) {
    delete origin;
    origin = NULL;
  }
#endif  // SENSCORD_RECORDER_RAW
#ifdef SENSCORD_RECORDER_SKV
  if (origin != NULL && format_name == kRecordingFormatSkv) {
    delete origin;
    origin = NULL;
  }
#endif  // SENSCORD_RECORDER_SKV
  if (origin != NULL) {
#ifdef SENSCORD_RECORDER_LOADER
    // user recorder
    RecorderManager* manager = RecorderManager::GetInstance();
    status = manager->ReleaseRecorder(format_name, origin);
    SENSCORD_STATUS_TRACE(status);
#endif  // SENSCORD_RECORDER_LOADER
  }

  // release the adapter.
  delete recorder;
  return status;
}

/**
 * @brief Write the xml file for recording informations.
 * @param[in] (path) Directory path.
 * @return Status object.
 */
Status FrameRecorder::WriteInfoFile(const std::string& path) const {
  std::string filename;
  RecordUtility::GetInfoFilePath(&filename);
  filename = path + osal::kDirectoryDelimiter + filename;

  InfoFileWriter writer;
  Status status = writer.Write(filename, stream_);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Write the stream properties when started.
 * @param[in] (path) Directory path.
 * @return Status object.
 */
Status FrameRecorder::WriteProperties(const std::string& path) const {
  std::vector<std::string> propertylist;
  Status status = InfoFileWriter::GetPropertyListOnlyRecording(
      stream_, &propertylist);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  std::vector<std::string>::const_iterator itr = propertylist.begin();
  std::vector<std::string>::const_iterator end = propertylist.end();
  for (; itr != end; ++itr) {
    // get each properties with BinaryProperty.
    BinaryProperty property = {};
    status = stream_->GetProperty(*itr, &property);
    if (!status.ok()) {
      SENSCORD_LOG_DEBUG("failed to GetProperty(%s). skip recording.",
          itr->c_str());
    } else {
      // create directory
      // ignore the error if existed directory.
      std::string directoryname;
      RecordUtility::GetStreamPropertyDirectoryName(&directoryname);
      directoryname = path + osal::kDirectoryDelimiter + directoryname;
      osal::OSMakeDirectory(directoryname.c_str());

      // create and write file
      std::string filepath;
      RecordUtility::GetStreamPropertyFilePath(*itr, &filepath);
      filepath = path + osal::kDirectoryDelimiter + filepath;

      status = WriteBinaryFile(filepath,
          &property.data[0], property.data.size());
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
    }
  }
  return Status::OK();
}

#ifdef SENSCORD_RECORDER_SKV
/**
 * @brief Write the stream properties when started.
 * @param[in] (library) skv library.
 * @return Status object.
 */
Status FrameRecorder::WritePropertiesForSkv(SkvRecordLibrary* library) const {
  std::vector<std::string> propertylist;
  Status status = stream_->GetPropertyList(&propertylist);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // Ignore unrecord property.
  std::vector<std::string>::iterator itr = propertylist.begin();
  while (itr != propertylist.end()) {
    if (!RecordUtility::IsRecordablePropertyForSkv(*itr)) {
      itr = propertylist.erase(itr);
    } else {
      ++itr;
    }
  }

  // Set recording property.
  StreamPropertiesForRecord stream_properties = {};
  for (itr = propertylist.begin(); itr != propertylist.end(); ++itr) {
    BinaryProperty property = {};
    if (*itr == kPointCloudPropertyKey) {
      // get PointCloudProperty
      PointCloudProperty point_cloud = {};
      status = stream_->GetProperty(*itr, &point_cloud);
      if (!status.ok()) {
        SENSCORD_LOG_DEBUG(
            "failed to GetProperty(%s). skip recording.", itr->c_str());
        continue;
      }

      // Convert the pixel format
      if (point_cloud.pixel_format == kPixelFormatXYZ16Planar) {
        point_cloud.pixel_format = kPixelFormatXYZ16;
      } else if (point_cloud.pixel_format == kPixelFormatXYZ32FPlanar) {
        point_cloud.pixel_format = kPixelFormatXYZ32F;
      }

      // Serialize
      serialize::SerializedBuffer serialized_property;
      serialize::Encoder enc(&serialized_property);
      enc.Push(point_cloud);

      // Set binary property
      property.data.reserve(serialized_property.size());
      property.data.assign(
          reinterpret_cast<const uint8_t*>(serialized_property.data()),
          reinterpret_cast<const uint8_t*>(serialized_property.data()) +
          serialized_property.size());
    } else {
      // get each properties with BinaryProperty.
      status = stream_->GetProperty(*itr, &property);
      if (!status.ok()) {
        SENSCORD_LOG_DEBUG(
            "failed to GetProperty(%s). skip recording.", itr->c_str());
        continue;
      }
    }

    // Insert to property list
    stream_properties.properties.insert(
        std::make_pair(itr->c_str(), property));
  }

  // serialize
  serialize::SerializedBuffer buf;
  serialize::Encoder enc(&buf);
  enc.Push(stream_properties);

  // write stream properties to CustomBuffer.
  status = library->AddCustomBuffer(
      kSkvBufferStreamProperty, buf.data(), buf.size());
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return Status::OK();
}

/**
 * @brief Write the skv write property data.
 * @param[in] (library) skv library.
 * @return Status object.
 */
Status FrameRecorder::WriteSkvWriteProperty(SkvRecordLibrary* library) const {
  // Get skv write property.
  SkvWriteProperty property = {};
  Status status = stream_->GetProperty(
      kSkvWritePropertyKey, &property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  for (std::map<std::string, SkvWriteData>::iterator
      itr = property.write_list.begin(), end = property.write_list.end();
      itr != end; ++itr) {
    const std::string& name = itr->first;
    if (name.empty() || (name.size() > 255)) {
      continue;  // not record
    }

    const SkvWriteData& write = itr->second;
    if (write.type != kSkvRecordTypeCustomBuffer) {
      continue;  // not record
    }

    // Write data to CustomBuffer.
    status = library->AddCustomBuffer(name,
        &write.data[0], write.data.size());
    if (!status.ok()) {
      SENSCORD_LOG_WARNING("%s", status.ToString().c_str());
      continue;  // not record
    }
  }

  return Status::OK();
}
#endif  // SENSCORD_RECORDER_SKV

/**
 * @brief Write the new binary file.
 * @param[in] (filepath) File path.
 * @param[in] (buffer) Write data.
 * @param[in] (buffer_size) Size of write data.
 * @return Status object.
 */
Status FrameRecorder::WriteBinaryFile(
    const std::string& filepath, uint8_t* buffer, size_t buffer_size) const {
  osal::OSFile* file = NULL;
  int32_t ret = osal::OSFopen(filepath.c_str(), "wb", &file);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "failed to open file: path=%s, ret=0x%" PRIx32,
        filepath.c_str(), ret);
  }
  ret = osal::OSFwrite(buffer, buffer_size, 1, file, NULL);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "failed to write file: path=%s, ret=0x%" PRIx32,
        filepath.c_str(), ret);
  }
  osal::OSFclose(file);
  return Status::OK();
}

/**
 * @brief Check invalid formats.
 * @param[in] (request_formats) The formats of request property.
 * @param[out] (detected_skv) The flag indicates the detection of skv format.
 * @return Status object.
 */
Status FrameRecorder::CheckInvalidFormats(
    const std::map<uint32_t, std::string>& formats,
    bool* detected_skv) const {
  // is empty.
  if (formats.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "Invalid formats: formats is empty.");
  }
  if (detected_skv == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // is unsupported combination.
  bool combination_any = false;
  std::map<uint32_t, std::string>::const_iterator itr = formats.begin();
  for (; itr != formats.end(); ++itr) {
    if (itr->second == kRecordingFormatSkv) {
      *detected_skv = true;
    } else {
      combination_any = true;
    }
  }

#ifndef SENSCORD_RECORDER_SKV
  if (*detected_skv) {
    // SKV format is valid only when the flag is SENSCORD_RECORDER_SKV=ON
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotSupported,
        "Unsupported format type : %s", kRecordingFormatSkv);
  }
#endif  // SENSCORD_RECORDER_SKV

  // Skv format cannot be combined with other formats.
  if (*detected_skv && combination_any) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "Invalid formats: unsupported combination.");
  }

  return Status::OK();
}

/**
 * @brief Set the thread state
 * @param[in] (state) The state of recorder thread
 */
void FrameRecorder::SetThreadState(RecordThreadState state) {
  util::AutoLock autolock(&mutex_state_);
  thread_state_ = state;
}

/**
 * @brief Return whether the thread state matches
 * @param[in] (state) The state of recorder thread
 * @return True if match
 */
bool FrameRecorder::IsThreadState(RecordThreadState state) const {
  util::AutoLock autolock(&mutex_state_);
  return (thread_state_ == state);
}

}   // namespace senscord
