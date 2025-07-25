/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "component/component_port_core.h"

#include <inttypes.h>
#include <utility>      // std::make_pair
#include <algorithm>    // std::count, find, sort
#include <iterator>     // std::back_inserter
#include <sstream>      // std::ostringstream

#include "logger/logger.h"
#include "senscord/osal_inttypes.h"
#include "util/autolock.h"
#include "stream/property_history_book.h"

namespace senscord {

/**
 * @brief Constructor
 * @param[in] (component) Parent component.
 * @param[in] (component_instance_name) Parent instance name.
 * @param[in] (port_type) Port type.
 * @param[in] (port_id) Port ID.
 * @param[in] (history_book) Property history book.
 */
ComponentPortCore::ComponentPortCore(
    Component* component,
    const std::string& component_instance_name,
    const std::string& port_type,
    int32_t port_id,
    PropertyHistoryBook* history_book)
    : component_(component), component_instance_name_(component_instance_name)
    , port_type_(port_type), port_id_(port_id)
    , is_client_port_(port_type_ == kPortTypeClient)
    , history_book_(history_book) {
  property_locker_ = new PropertyLockManager(this);
}

/**
 * @brief Destructor
 */
ComponentPortCore::~ComponentPortCore() {
  delete property_locker_;
}

/**
 * @brief Connect stream.
 * @param[in] (stream) Connecting stream.
 * @return Status object.
 */
Status ComponentPortCore::Open(StreamCore* stream) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "%s(%s.%" PRId32 "): invalid parameter: stream=NULL",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_);
  }

  util::AutoLock autolock(&mutex_state_change_);
  {
    util::AutoLock streams_lock(&mutex_streams_opened_);
    if (IsOpenedStream(stream)) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidOperation,
          "%s(%s.%" PRId32 "): "
          "already connected with same stream: stream=%p",
          component_instance_name_.c_str(), port_type_.c_str(), port_id_,
          stream);
    }
    streams_opened_.push_back(stream);
    if (streams_opened_.size() > 1) {
      // already connected with other stream(s)
      Status status = IsSameStreamArguments(stream);
      if (!status.ok()) {
        RemoveStream(&streams_opened_, stream);
        history_book_->RemoveUpdatedPropertyList(stream);
      }
      return SENSCORD_STATUS_TRACE(status);
    }
  }

  // first open
  port_args_.stream_key = stream->GetKey();
  port_args_.arguments = stream->GetInitialSetting().arguments;

  Status status = component_->OpenPort(port_type_, port_id_, port_args_);
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);
    status.SetBlock(component_instance_name_);
    SENSCORD_LOG_ERROR("%s(%s.%" PRId32 "): "
        "OpenPort failed: status=%s",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_,
        status.ToString().c_str());

    // cancel to open
    util::AutoLock streams_lock(&mutex_streams_opened_);
    RemoveStream(&streams_opened_, stream);
    history_book_->RemoveUpdatedPropertyList(stream);
  }
  return status;
}

/**
 * @brief Disconnect stream.
 * @param[in] (stream) Disconnecting stream.
 * @return Status object.
 */
Status ComponentPortCore::Close(const StreamCore* stream) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "%s(%s.%" PRId32 "): invalid parameter: stream=NULL",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_);
  }

  util::AutoLock autolock(&mutex_state_change_);
  {
    {
      util::AutoLock streams_lock(&mutex_streams_opened_);
      Status status = RemoveStream(&streams_opened_, stream);
      if (!status.ok()) {
        SENSCORD_LOG_ERROR("%s(%s.%" PRId32 "): "
            "not connected stream: stream=%p",
            component_instance_name_.c_str(), port_type_.c_str(), port_id_,
            stream);
        return SENSCORD_STATUS_TRACE(status);
      }
    }

    // if unlock property when locked.
    property_locker_->ForceUnlockProperty(stream);

    {
      util::AutoLock streams_lock(&mutex_streams_opened_);
      // check last close
      if (!streams_opened_.empty()) {
        history_book_->RemoveUpdatedPropertyList(stream);
        return Status::OK();
      }
    }
  }

  // last close
  Status status = component_->ClosePort(port_type_, port_id_);
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);
    status.SetBlock(component_instance_name_);
    SENSCORD_LOG_ERROR("%s(%s.%" PRId32 "): "
        "ClosePort failed: status=%s",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_,
        status.ToString().c_str());

    util::AutoLock streams_lock(&mutex_streams_opened_);
    streams_opened_.push_back(const_cast<StreamCore*>(stream));
  } else {
    history_book_->RemoveUpdatedPropertyList(stream);
  }
  return status;
}

/**
 * @brief Start by stream.
 * @param[in] (stream) Startting stream.
 * @return Status object.
 */
Status ComponentPortCore::Start(StreamCore* stream) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "%s(%s.%" PRId32 "): invalid parameter: stream=NULL",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_);
  }

  util::AutoLock autolock(&mutex_state_change_);
  {
    if (!IsOpenedStream(stream)) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidOperation,
          "%s(%s.%" PRId32 "): "
          "not connected stream: stream=%p",
          component_instance_name_.c_str(), port_type_.c_str(), port_id_,
          stream);
    }
    util::AutoLock streams_lock(&mutex_streams_started_);
    if (streams_started_.empty()) {
      util::AutoLock lock(&mutex_frames_);
      if (!sent_frames_.empty()) {
#ifdef SENSCORD_STATUS_MESSAGE_ENABLED
        std::string dump;
        GetFrameSendingStateString(&dump);
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseBusy,
            "%s(%s.%" PRId32 "): "
            "Unreleased frames exist: stream=%p, %s",
            component_instance_name_.c_str(), port_type_.c_str(), port_id_,
            stream, dump.c_str());
#else
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseBusy, "");
#endif  // SENSCORD_STATUS_MESSAGE_ENABLED
      }
    } else {
      if (IsStartedStream(stream)) {
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseInvalidOperation,
            "%s(%s.%" PRId32 "): "
            "already started stream: stream=%p",
            component_instance_name_.c_str(), port_type_.c_str(), port_id_,
            stream);
      }
    }
    streams_started_.push_back(stream);
    if (streams_started_.size() > 1) {
      return Status::OK();
    }
  }

  // If first stating, call component implementation
  Status status = component_->StartPort(port_type_, port_id_);
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);
    status.SetBlock(component_instance_name_);
    SENSCORD_LOG_ERROR("%s(%s.%" PRId32 "): "
        "StartPort failed: status=%s",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_,
        status.ToString().c_str());

    // cancel to start
    util::AutoLock streams_lock(&mutex_streams_started_);
    RemoveStream(&streams_started_, stream);
  }
  return status;
}

/**
 * @brief Start by stream.
 * @param[in] (stream) Startting stream.
 * @return Status object.
 */
Status ComponentPortCore::Stop(const StreamCore* stream) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "%s(%s.%" PRId32 "): invalid parameter: stream=NULL",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_);
  }

  util::AutoLock autolock(&mutex_state_change_);
  {
    util::AutoLock streams_lock(&mutex_streams_started_);
    Status status = RemoveStream(&streams_started_, stream);
    if (!status.ok()) {
      SENSCORD_LOG_ERROR("%s(%s.%" PRId32 "): "
          "not connected stream: stream=%p",
          component_instance_name_.c_str(), port_type_.c_str(), port_id_,
          stream);
      return SENSCORD_STATUS_TRACE(status);
    }
    if (!streams_started_.empty()) {
      return Status::OK();
    }
  }

  // If last stopping, call component implementation
  Status status = component_->StopPort(port_type_, port_id_);
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);
    status.SetBlock(component_instance_name_);
    SENSCORD_LOG_ERROR("%s(%s.%" PRId32 "): "
        "StopPort failed: status=%s",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_,
        status.ToString().c_str());

    util::AutoLock streams_lock(&mutex_streams_started_);
    streams_started_.push_back(const_cast<StreamCore*>(stream));
  }
  return status;
}

/**
 * @brief Get connected streams information.
 * @return Opened streams count.
 */
uint32_t ComponentPortCore::GetOpenedStreamCount() {
  util::AutoLock autolock(&mutex_streams_opened_);
  return static_cast<uint32_t>(streams_opened_.size());
}

/**
 * @brief Get whether be connected or not.
 * @return State of connecting.
 */
bool ComponentPortCore::IsConnected() const {
  util::AutoLock autolock(&mutex_streams_opened_);
  return (!streams_opened_.empty());
}

/**
 * @brief Send the multiple frames to the connected stream.
 * @param[in] (frames) List of frame information to send.
 * @param[out] (dropped_frames) List of pointer of dropped frames.
 * @return Status object.
 */
Status ComponentPortCore::SendFrames(
    const std::vector<FrameInfo>& frames,
    std::vector<const FrameInfo*>* dropped_frames) {
  if (frames.empty()) {
    return Status::OK();
  }

  // get timestamp
  uint64_t ts = 0;
  if (!is_client_port_) {
    osal::OSGetTime(&ts);
  }

  // send
  std::vector<const FrameInfo*> dropped_result;

  do {
    util::AutoLock autolock(&mutex_streams_started_);
    if (streams_started_.empty()) {
      std::vector<FrameInfo>::const_iterator frame_itr;
      std::vector<FrameInfo>::const_iterator frame_end = frames.end();
      for (frame_itr = frames.begin(); frame_itr != frame_end; ++frame_itr) {
        dropped_result.push_back(&(*frame_itr));
      }
      break;
    }
    StreamCoreConstIterator stream_itr;
    StreamCoreConstIterator stream_begin = streams_started_.begin();
    StreamCoreConstIterator stream_end = streams_started_.end();

    // Set all destination streams.
    {
      util::AutoLock lock(&mutex_frames_);
      std::vector<FrameInfo>::const_iterator frame_itr;
      std::vector<FrameInfo>::const_iterator frame_end = frames.end();
      for (frame_itr = frames.begin(); frame_itr != frame_end; ++frame_itr) {
        FrameSending& send_state =
            sent_frames_[frame_itr->sequence_number].sending_state;
        for (stream_itr = stream_begin;
            stream_itr != stream_end; ++stream_itr) {
          send_state.insert(std::make_pair(*stream_itr, kNotSendingYet));
        }
      }
    }

    // Send to all streams.
    for (stream_itr = stream_begin; stream_itr != stream_end; ++stream_itr) {
      std::vector<const FrameInfo*> dropped;
      Status status = (*stream_itr)->SendFrames(frames, ts, &dropped);
      if (status.ok()) {
        dropped_result.clear();
      } else {
        // Frames dropped.
        std::vector<const FrameInfo*>::const_iterator drop_itr;
        std::vector<const FrameInfo*>::const_iterator drop_end = dropped.end();
        for (drop_itr = dropped.begin(); drop_itr != drop_end; ++drop_itr) {
          ReleaseFrame(*stream_itr, *(*drop_itr), NULL, kSendingFailed);
        }

        if (stream_itr == stream_begin) {
          dropped_result.swap(dropped);
        } else if (!dropped_result.empty()) {
          // Calculate the intersection of dropped frames.
          std::vector<const FrameInfo*> intersection_set;
          std::set_intersection(dropped_result.begin(), dropped_result.end(),
                                dropped.begin(), dropped.end(),
                                std::back_inserter(intersection_set));
          dropped_result.swap(intersection_set);
        }
      }
    }
  } while (false);

  if (dropped_result.empty()) {
    return Status::OK();
  }

  Status status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
      Status::kCauseBusy, "%s(%s.%" PRId32 "): %s frames dropped.",
      component_instance_name_.c_str(), port_type_.c_str(), port_id_,
      (dropped_result.size() == frames.size()) ? "all" : "some");

  if (dropped_frames != NULL) {
    dropped_frames->swap(dropped_result);
  }

  return status;
}

/**
 * @brief Send the event to the connected stream.
 * @param[in] (event) Event type.
 * @param[in] (args) Event argument.
 * @return Status object.
 */
Status ComponentPortCore::SendEvent(
    const std::string& event, const EventArgument& args) {
  if (event == kEventAny) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "%s(%s.%" PRId32 "): unsupported event type: %s",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_,
        event.c_str());
  }

  bool success = false;
  {
    util::AutoLock autolock(&mutex_streams_opened_);
    StreamCoreConstIterator itr = streams_opened_.begin();
    StreamCoreConstIterator end = streams_opened_.end();
    for (; itr != end; ++itr) {
      Status status = (*itr)->SendEvent(event, args);
      if (status.ok()) {
        success = true;
      }
    }
  }
  if (success) {
    return Status::OK();
  }
  return SENSCORD_STATUS_FAIL(kStatusBlockCore,
      Status::kCauseBusy,
      "%s(%s.%" PRId32 "): "
      "failed to all sending event: event=%s",
      component_instance_name_.c_str(), port_type_.c_str(), port_id_,
      event.c_str());
}

/**
 * @brief Register property accessor.
 * @param[in] (accessor) Property accessor.
 * @return Status object.
 */
Status ComponentPortCore::RegisterPropertyAccessor(
    PropertyAccessor* accessor) {
  if (accessor == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "%s(%s.%" PRId32 "): "
        "invalid parameter: accessor=NULL",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_);
  }
  const std::string& key = accessor->GetKey();
  if (key.empty()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "%s(%s.%" PRId32 "): property key is none",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_);
  }
  bool ret = properties_.insert(std::make_pair(key, accessor)).second;
  if (!ret) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseAlreadyExists,
        "%s(%s.%" PRId32 "): already registered key: key=%s",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_,
        key.c_str());
  }
  return Status::OK();
}

/**
 * @brief Unregister property accessor.
 * @param[in] (property_key) Key of property.
 * @param[out] (accessor) Registered accessor address. (optional)
 * @return Status object.
 */
Status ComponentPortCore::UnregisterPropertyAccessor(
    const std::string& property_key, PropertyAccessor** accessor) {
  std::map<std::string, PropertyAccessor*>::iterator itr =
      properties_.find(property_key);
  if (itr == properties_.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation,
        "%s(%s.%" PRId32 "): not registered key: key=%s",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_,
        property_key.c_str());
  }
  if (accessor != NULL) {
    *accessor = itr->second;
  }
  properties_.erase(itr);
  return Status::OK();
}

/**
 * @brief Get property interface.
 * @param[in] (key) Key of property.
 * @return Property accessor interface.
 */
PropertyAccessor* ComponentPortCore::GetPropertyAccessor(
    const std::string& key) {
  std::map<std::string, PropertyAccessor*>::const_iterator itr =
      properties_.find(key);
  if (itr != properties_.end()) {
    return itr->second;
  }
  return NULL;
}

/**
 * @brief Get the supported property key list on this port.
 * @param[out] (key_list) Supported property key list.
 * @return Status object.
 */
Status ComponentPortCore::GetSupportedPropertyList(
    std::set<std::string>* key_list) {
  if (key_list == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "%s(%s.%" PRId32 "): "
        "invalid parameter: key_list=NULL",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_);
  }
  for (std::map<std::string, PropertyAccessor*>::const_iterator
      itr = properties_.begin(), end = properties_.end(); itr != end; ++itr) {
    key_list->insert(itr->first);
  }
  return Status::OK();
}

/**
 * @brief Get property locker.
 * @return Property locker.
 */
PropertyLockManager* ComponentPortCore::GetPropertyLocker() {
  return property_locker_;
}

/**
 * @brief Register the callback for LockProperty
 * @param [in] (callback) The callback called by LockProperty.
 * @param [in] (private_data) Value with callback called.
 */
void ComponentPortCore::RegisterLockPropertyCallback(
    OnLockPropertyCallback callback, void* private_data) {
  property_locker_->RegisterLockPropertyCallback(callback, private_data);
}

/**
 * @brief Register the callback for UnlockProperty
 * @param [in] (callback) The callback called by UnlockProperty.
 * @param [in] (private_data) Value with callback called.
 */
void ComponentPortCore::RegisterUnlockPropertyCallback(
    OnUnlockPropertyCallback callback, void* private_data) {
  property_locker_->RegisterUnlockPropertyCallback(callback, private_data);
}

#ifdef SENSCORD_SERIALIZE
/**
 * @brief Update the serialized property for frame channel.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (property) Serialized property.
 * @param[in] (property_size) Serialized property size.
 * @return Status object.
 */
Status ComponentPortCore::UpdateFrameSerializedProperty(
    uint32_t channel_id,
    const std::string& key,
    const void* property,
    size_t property_size) {
  Status status = history_book_->SetProperty(
      channel_id, key, property, property_size);
  return SENSCORD_STATUS_TRACE(status);
}
#else
/**
 * @brief Update frame channel property.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (property) Property to updated.
 * @param[in] (factory) Property factory.
 * @return Status object.
 */
Status ComponentPortCore::UpdateFramePropertyWithFactory(
    uint32_t channel_id, const std::string& key, const void* property,
    const PropertyFactoryBase& factory) {
  if (!IsConnected()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation,
        "port is not connected");
  }
  Status status = history_book_->SetProperty(
      channel_id, key, property, factory);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_SERIALIZE

/**
 * @brief Get property history book address.
 * @return Property history book address.
 */
PropertyHistoryBook* ComponentPortCore::GetPropertyHistoryBook() {
  return history_book_;
}

/**
 * @brief Set user data to all connected streams.
 * @param[in] (user_data) New user data.
 * @return Status object.
 */
Status ComponentPortCore::SetUserData(const FrameUserData& user_data) {
  bool success = false;
  {
    util::AutoLock autolock(&mutex_streams_opened_);
    StreamCoreConstIterator itr = streams_opened_.begin();
    StreamCoreConstIterator end = streams_opened_.end();
    for (; itr != end; ++itr) {
      Status status = (*itr)->SetUserData(user_data);
      if (status.ok()) {
        success = true;
      }
    }
  }
  if (success) {
    return Status::OK();
  }
  return SENSCORD_STATUS_FAIL(kStatusBlockCore,
      Status::kCauseBusy,
      "%s(%s.%" PRId32 "): "
      "failed to user data updating: adr=0x%" PRIxPTR ", size=%" PRIdS,
      component_instance_name_.c_str(), port_type_.c_str(), port_id_,
      user_data.data_address, user_data.data_size);
}

#ifdef SENSCORD_PLAYER
/**
 * @brief Update the port (stream) type. For only the player component.
 * @param[in] (port_type) New port (stream) type.
 * @return Status object.
 */
Status ComponentPortCore::SetType(const std::string& type) {
  util::AutoLock autolock(&mutex_streams_opened_);
  StreamCoreConstIterator itr = streams_opened_.begin();
  StreamCoreConstIterator end = streams_opened_.end();
  for (; itr != end; ++itr) {
    // TODO: stub
  }
  return Status::OK();
}
#endif  // SENSCORD_PLAYER

/**
 * @brief Release frame list from stream.
 * @param[in] (stream) Stream sent frame.
 * @param[in] (frameinfo) Sent frame information.
 * @param[in] (referenced_channel_ids) List of referenced channel IDs.
 * @return Status object.
 */
Status ComponentPortCore::ReleaseFrame(
    StreamCore* stream, const FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids) {
  Status status = ReleaseFrame(stream, frameinfo, referenced_channel_ids,
                               kReleased);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Release frame list from stream.
 * @param[in] (stream) Stream sent frame.
 * @param[in] (frameinfo) Sent frame information.
 * @param[in] (referenced_channel_ids) List of referenced channel IDs.
 * @param[in] (state) Release causation.
 * @return Status object.
 */
Status ComponentPortCore::ReleaseFrame(
    StreamCore* stream, const FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids,
    FrameSendingState state) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "%s(%s.%" PRId32 "): invalid parameter: stream=NULL",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_);
  }

  bool call_implement = false;
  std::vector<uint32_t> tmp_channel_ids;
  {
    util::AutoLock lock(&mutex_frames_);

    // search frame sending state
    SentFramesMap::iterator itr_frame = sent_frames_.find(
        frameinfo.sequence_number);
    if (itr_frame == sent_frames_.end()) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
          "%s(%s.%" PRId32 "): "
          "unknown frame: num=%" PRIu64,
          component_instance_name_.c_str(), port_type_.c_str(), port_id_,
          frameinfo.sequence_number);
    }
    FrameSending* sending_map = &itr_frame->second.sending_state;

    // search and update sending state
    FrameSending::iterator itr_state = sending_map->find(stream);
    if (itr_state == sending_map->end()) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
          "%s(%s.%" PRId32 "): "
          "release from no sending stream: num=%" PRIu64 ", stream=%p",
          component_instance_name_.c_str(), port_type_.c_str(), port_id_,
          frameinfo.sequence_number, stream);
    }
    itr_state->second = state;
    if (referenced_channel_ids != NULL) {
      itr_frame->second.referenced_channel_ids.insert(
          referenced_channel_ids->begin(), referenced_channel_ids->end());
    }

    // check to release from all streams.
    // do nothing when not finished yet.
    FrameSending::const_iterator itr_end = sending_map->end();
    for (itr_state = sending_map->begin(); itr_state != itr_end; ++itr_state) {
      if (itr_state->second == kNotSendingYet) {
        return Status::OK();
      }
      if (itr_state->second == kReleased) {
        call_implement = true;
      }
    }
    // When calling ReleasePortFrame, create a list.
    if (call_implement) {
      tmp_channel_ids.reserve(itr_frame->second.referenced_channel_ids.size());
      tmp_channel_ids.assign(itr_frame->second.referenced_channel_ids.begin(),
                             itr_frame->second.referenced_channel_ids.end());
    }

    // if last releasing, erase frame seq num
    sent_frames_.erase(itr_frame);
  }

  // last release
  if (!call_implement) {
    return Status::OK();
  }
  Status status;
  if (!tmp_channel_ids.empty()) {
    status = component_->ReleasePortFrame(
        port_type_, port_id_, frameinfo, &tmp_channel_ids);
  } else {
    status = component_->ReleasePortFrame(
        port_type_, port_id_, frameinfo, NULL);
  }
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);
    status.SetBlock(component_instance_name_);
    SENSCORD_LOG_ERROR("%s(%s.%" PRId32 "): "
        "ReleasePortFrame failed: status=%s",
        component_instance_name_.c_str(), port_type_.c_str(), port_id_,
        status.ToString().c_str());
  }
  return status;
}

/**
 * @brief Get to whether opened or not.
 * @param[in] (stream) Owner stream.
 * @return Whether opened or not.
 */
bool ComponentPortCore::IsOpenedStream(const StreamCore* stream) const {
  util::AutoLock autolock(&mutex_streams_opened_);
  return (std::count(streams_opened_.begin(), streams_opened_.end(),
      stream) > 0);
}

/**
 * @brief Get to whether started or not.
 * @param[in] (stream) Owner stream.
 * @return Whether started or not.
 */
bool ComponentPortCore::IsStartedStream(const StreamCore* stream) const {
  util::AutoLock autolock(&mutex_streams_started_);
  return (std::count(streams_started_.begin(), streams_started_.end(),
      stream) > 0);
}

/**
 * @brief Remove stream from list.
 * @param[in] (list) Stream list.
 * @param[in] (stream) Removing stream.
 * @return Status object.
 */
Status ComponentPortCore::RemoveStream(StreamCoreList* list,
                                       const StreamCore* stream) const {
  if ((list == NULL) || (stream == NULL)) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  StreamCoreIterator itr = std::find(list->begin(), list->end(), stream);
  if (itr == list->end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotFound,
        "stream not found");
  }
  if (list == &streams_started_) {
    // releases unused frames when stopped.
    (*itr)->ClearFrames(NULL);
  }
  list->erase(itr);
  return Status::OK();
}

/**
 * @brief Check if the arguments are for the same stream.
 * @param[in] (stream) new stream.
 * @return Status object.
 */
Status ComponentPortCore::IsSameStreamArguments(
    const StreamCore* stream) const {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  const std::map<std::string, std::string>* arguments =
      &stream->GetInitialSetting().arguments;
  if (port_args_.arguments == *arguments) {
    return Status::OK();
  }
  std::string args_log;
#ifdef SENSCORD_STATUS_MESSAGE_ENABLED
  typedef std::map<std::string, std::string> StreamArguments;
  std::ostringstream dump;
  dump << "[current]: ";
  {
    StreamArguments::const_iterator itr = port_args_.arguments.begin();
    StreamArguments::const_iterator end = port_args_.arguments.end();
    for (; itr != end; ++itr) {
      dump << "{" << itr->first << "," << itr->second << "},";
    }
  }
  dump << " [arrival]: ";
  {
    StreamArguments::const_iterator itr = arguments->begin();
    StreamArguments::const_iterator end = arguments->end();
    for (; itr != end; ++itr) {
      dump << "{" << itr->first << "," << itr->second << "},";
    }
  }
  args_log =  dump.str();
#endif  // SENSCORD_STATUS_MESSAGE_ENABLED
  return SENSCORD_STATUS_FAIL(kStatusBlockCore,
      Status::kCauseInvalidArgument,
      "unmatched stream arguments: stream=%p, %s", stream, args_log.c_str());
}

#ifdef SENSCORD_STATUS_MESSAGE_ENABLED
/**
 * @brief Get the frame sending state as a string. (For analysis)
 * @param[out] (value) String in sending state.
 */
void ComponentPortCore::GetFrameSendingStateString(std::string* value) const {
  std::ostringstream dump;
  SentFramesMap::const_iterator itr = sent_frames_.begin();
  SentFramesMap::const_iterator end = sent_frames_.end();
  for (; itr != end; ++itr) {
    std::ostringstream::pos_type pos = dump.tellp();
    if (pos > 0) {
      dump << ", ";
    }
    dump << "{seq_num:" << std::dec << itr->first;
    const FrameSending& state = itr->second.sending_state;
    FrameSending::const_iterator state_itr = state.begin();
    FrameSending::const_iterator state_end = state.end();
    for (; state_itr != state_end; ++state_itr) {
      dump << ", " << std::hex << state_itr->first << ":";
      if (state_itr->second == kNotSendingYet) {
        dump << "unreleased";
      } else if (state_itr->second == kReleased) {
        dump << "released";
      } else {
        dump << "send-failed";
      }
    }
    dump << "}";
  }
  *value = dump.str();
}
#endif  // SENSCORD_STATUS_MESSAGE_ENABLED

}   // namespace senscord
