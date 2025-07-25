/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "frame/frame_core.h"

#include <stdint.h>
#include <string>
#include <vector>
#include <algorithm>    // std::find
#include <utility>      // std::make_pair

#include "senscord/rawdata_types.h"
#include "logger/logger.h"
#include "frame/channel_core.h"
#include "stream/stream_core.h"
#include "stream/property_history_book.h"
#include "util/autolock.h"

namespace {

/**
 * @brief Resource for channel list.
 */
const char kResourceChannelList[] = "channel_list";

struct ResourceChannelList : public senscord::ResourceData {
  senscord::ChannelList channel_list;
};

}  // namespace

namespace senscord {

/**
 * @brief Constructor
 * @param[in] (stream) Parent stream.
 * @param[in] (info) Frame information.
 * @param[in] (sent_time) Time when this frame was sent.
 */
FrameCore::FrameCore(
    const StreamCore* stream, const FrameInfo& info,
    uint64_t sent_time)
    : parent_stream_(stream)
    , frame_info_(info)
    , mask_disabled_(false)
    , extension_frame_info_()
#ifdef SENSCORD_RECORDER
    , is_recorded_(false)
#endif  // SENSCORD_RECORDER
{
  if (sent_time > 0) {
    frame_info_.sent_time = sent_time;
  }

  // setup channel list
  for (std::vector<ChannelRawData>::const_iterator
      itr = frame_info_.channels.begin(),
      end = frame_info_.channels.end(); itr != end; ++itr) {
    ChannelCore* channel = new ChannelCore(
        *itr, parent_stream_, parent_stream_->GetPropertyHistoryBook());
    channel_list_.push_back(channel);
  }

  resources_.Create<ResourceChannelList>(kResourceChannelList);
}

/**
 * @brief Destructor
 */
FrameCore::~FrameCore() {
  while (!channel_list_.empty()) {
    ChannelCore* channel = channel_list_.back();
    channel_list_.pop_back();
    delete channel;
  }
}

/**
 * @brief Get the sequential number of frame.
 * @param[out] (sequence_number) The number of this frame.
 * @return Status object.
 */
Status FrameCore::GetSequenceNumber(uint64_t* sequence_number) const {
  SENSCORD_STATUS_ARGUMENT_CHECK(sequence_number == NULL);
  *sequence_number = frame_info_.sequence_number;
  return Status::OK();
}

/**
 * @brief Get the time of frame was sent. Based on GetTime() of OSAL.
 * @param[out] (sent_time) Time when frame was sent.
 * @return Status object.
 */
Status FrameCore::GetSentTime(uint64_t* sent_time) const {
  SENSCORD_STATUS_ARGUMENT_CHECK(sent_time == NULL);
  *sent_time = frame_info_.sent_time;
  return Status::OK();
}

/**
 * @brief Get type.
 * @param[out] (type) Type of this frame.
 * @return Status object.
 */
Status FrameCore::GetType(std::string* type) const {
  SENSCORD_STATUS_ARGUMENT_CHECK(type == NULL);
  *type = parent_stream_->GetType();
  return Status::OK();
}

/**
 * @brief Get channel list.
 */
const ChannelList& FrameCore::GetChannelList() const {
  util::AutoLock lock(parent_stream_->GetFrameMutex());
  ResourceChannelList* resource =
      resources_.Get<ResourceChannelList>(kResourceChannelList);
  if (resource->channel_list.empty()) {
    ChannelList tmp_list;
    for (std::vector<ChannelCore*>::const_iterator
        itr = channel_list_.begin(), end = channel_list_.end();
        itr != end; ++itr) {
      uint32_t id = (*itr)->GetChannelId();
      if (IsMaskedChannel(id)) {
        continue;
      }
      tmp_list.insert(std::make_pair(id, *itr));
    }
    resource->channel_list.swap(tmp_list);
  }
  return resource->channel_list;
}

/**
 * @brief Get channel list.
 * @param[out] (list) Location of all channel list.
 * @return Status object.
 */
Status FrameCore::GetChannelList(ChannelList* list) const {
  SENSCORD_STATUS_ARGUMENT_CHECK(list == NULL);
  *list = GetChannelList();
  return Status::OK();
}

/**
 * @brief Get channel data.
 * @param[in] (channel_id) Channel ID to get.
 * @param[out] (channel) Channel object.
 * @return Status object.
 */
Status FrameCore::GetChannel(uint32_t channel_id,
                             Channel** channel) const {
  SENSCORD_STATUS_ARGUMENT_CHECK(channel == NULL);
  const ChannelList& list = GetChannelList();
  ChannelList::const_iterator itr = list.find(channel_id);
  if (itr != list.end()) {
    *channel = itr->second;
    return Status::OK();
  }
  return SENSCORD_STATUS_FAIL(
      kStatusBlockCore, Status::kCauseNotFound, "no exist channel id.");
}

/**
 * @brief  Get the user data.
 * @param[out] (user_data) the user data.
 * @return Status object.
 */
Status FrameCore::GetUserData(Frame::UserData* user_data) const {
  SENSCORD_STATUS_ARGUMENT_CHECK(user_data == NULL);
  if (user_data_.empty()) {
    user_data->address = NULL;
    user_data->size = 0;
  } else {
    user_data->address = const_cast<uint8_t*>(user_data_.data());
    user_data->size = user_data_.size();
  }
  return Status::OK();
}

/**
 * @brief Set user data.
 * @param[in] (user_data) User data of this frame.
 */
void FrameCore::SetUserData(const FrameUserData& user_data) {
  uint8_t* ptr = reinterpret_cast<uint8_t*>(user_data.data_address);
  user_data_.reserve(user_data.data_size);
  user_data_.assign(ptr, ptr + user_data.data_size);
}

/**
 * @brief Set the masking channels.
 * @param[in] the list of channel IDs.
 */
void FrameCore::SetChannelMask(const std::vector<uint32_t>& mask) {
  masked_channels_ = mask;
}

/**
 * @brief Disable channel mask.
 * @param[in] (disabled) Channel mask disable.
 */
void FrameCore::SetDisableChannelMask(bool disabled) {
  mask_disabled_ = disabled;
}

/**
 * @brief Check whether channel is masked.
 * @param[in] (channel_id) channel id.
 * @return true means channel is masked.
 */
bool FrameCore::IsMaskedChannel(uint32_t channel_id) const {
  if (mask_disabled_) {
    return false;
  }
  std::vector<uint32_t>::const_iterator itr = std::find(
      masked_channels_.begin(), masked_channels_.end(), channel_id);
  if (itr != masked_channels_.end()) {
    return true;
  }
  return false;
}

/**
 * @brief Set the extension frame info.
 * @param[in] (frameinfo) Extension frame info
 * @param[in] (history_book) Extension PropertyHistoryBook.
 */
void FrameCore::SetExtensionFrameInfo(
    ExtensionFrameInfo* frame_info, PropertyHistoryBook* history_book) {
  bool list_update = false;
  if (frame_info == NULL || extension_frame_info_ != NULL) {
    for (std::vector<ExtensionChannelRawData>::const_iterator
        itr = extension_frame_info_->channels.begin(),
        end = extension_frame_info_->channels.end(); itr != end; ++itr) {
      for (std::vector<ChannelCore*>::const_iterator
          ch_itr = channel_list_.begin(), ch_end = channel_list_.end();
          ch_itr != ch_end; ++ch_itr) {
        if (itr->channel_id == (*ch_itr)->GetChannelId()) {
          ChannelCore* channel = (*ch_itr);
          channel_list_.erase(ch_itr);
          delete channel;
          list_update = true;
          break;
        }
      }
    }
    delete extension_frame_info_;
    extension_frame_info_ = NULL;
  }
  if (frame_info != NULL) {
    extension_frame_info_ = new ExtensionFrameInfo(*frame_info);
    for (std::vector<ExtensionChannelRawData>::const_iterator
        itr = extension_frame_info_->channels.begin(),
        end = extension_frame_info_->channels.end(); itr != end; ++itr) {
      // check already exist
      bool found = false;
      for (std::vector<ChannelCore*>::const_iterator
          ch_itr = channel_list_.begin(), ch_end = channel_list_.end();
          ch_itr != ch_end; ++ch_itr) {
        if (itr->channel_id == (*ch_itr)->GetChannelId()) {
          found = true;
          break;
        }
      }
      if (found) {
        continue;   // next extend channel
      }
      ChannelCore* channel =
          new ChannelCore(*itr, parent_stream_, history_book);
      channel_list_.push_back(channel);
      list_update = true;
    }
  }
  if (list_update) {
    ResourceChannelList* resource =
        resources_.Get<ResourceChannelList>(kResourceChannelList);
    resource->channel_list.clear();
  }
}

/**
 * @brief Get the extension frame info.
 * @return Extension frame info.
 */
ExtensionFrameInfo* FrameCore::GetExtensionFrameInfo() {
  return extension_frame_info_;
}

}  // namespace senscord
