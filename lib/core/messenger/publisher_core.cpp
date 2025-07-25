/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "messenger/publisher_core.h"

#include <utility>

#include "allocator/memory_manager.h"

namespace senscord {

/**
 * @brief Constructor
 * @param[in] (key) Publisher key.
 * @param[in] (topic) Parent messenger topic.
 * @param[in] (callback) Release frame callback.
 */
PublisherCore::PublisherCore(MessengerTopic* topic)
    : callback_(), callback_user_data_(), topic_(topic),
      state_(kPublisherStateInit), state_mutex_() {
  state_mutex_ = new util::Mutex();
}

/**
 * @brief Destructor
 */
PublisherCore::~PublisherCore() {
  delete state_mutex_;
  state_mutex_ = NULL;
}

/**
 * @brief Open the publisher.
 * @param[in] (allocator_keys) Memory allocator keys.
 * @return Status object.
 */
Status PublisherCore::Open(
    const std::string& key,
    Core::OnReleaseFrameCallback callback,
    const std::map<std::string, std::string>& allocator_keys) {
  util::AutoLock lock(state_mutex_);
  if (GetState() != kPublisherStateInit) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidOperation,
        "already initialized");
  }
  key_ = key;
  callback_ = callback;
  Status status = SetAllocators(allocator_keys);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    SetState(kPublisherStateOpen);
  }
  return status;
}

/**
 * @brief Close the publisher.
 * @return Status object.
 */
Status PublisherCore::Close() {
  // allocators_ is not cleared because ReleaseFrame
  // may be called after Close.
  util::AutoLock lock(state_mutex_);
  SetState(kPublisherStateClose);
  return Status::OK();
}

/**
 * @brief Publish frames to connected stream.
 * @param[in] (frames) The publish frames.
 * @return Status object.
 */
Status PublisherCore::PublishFrames(const std::vector<FrameInfo>& frames) {
  Status status = topic_->PublishFrames(this, frames);
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get the memory allocator by the name.
 * @param[in] (name) The memory allocator name.
 * @param[out] (allocator) The accessor of memory allocator.
 * @return Status object.
 */
Status PublisherCore::GetAllocator(
    const std::string& name, MemoryAllocator** allocator) {
  if (allocator == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "allocator is NULL");
  }
  std::map<std::string, MemoryAllocator*>::const_iterator found =
      allocators_.find(name);
  if (found != allocators_.end()) {
    *allocator = found->second;
    return Status::OK();
  }
  return SENSCORD_STATUS_FAIL(kStatusBlockCore,
      Status::kCauseNotFound, "not found allocator name=%s", name.c_str());
}

#ifdef SENSCORD_SERIALIZE
/**
 * @brief Set property to PropertyHistoryBook.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (property) Property to updated.
 * @param[in] (size) Size of property.
 * @return Status object.
 */
Status PublisherCore::SetUpdateChannelProperty(
    uint32_t channel_id, const std::string& key,
    const void* property, size_t property_size) {
  PropertyHistoryBook* history_book = topic_->GetPropertyHistoryBook();
  Status status = history_book->SetProperty(
      channel_id, key, property, property_size);
  return SENSCORD_STATUS_TRACE(status);
}
#else  // SENSCORD_SERIALIZE
/**
 * @brief Set property to PropertyHistoryBook.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (property) Property to updated.
 * @param[in] (factory) Property factory.
 * @return Status object.
 */
Status PublisherCore::SetUpdateChannelProperty(
    uint32_t channel_id, const std::string& key,
    const void* property, const PropertyFactoryBase& factory) {
  PropertyHistoryBook* history_book = topic_->GetPropertyHistoryBook();
  Status status = history_book->SetProperty(
      channel_id, key, property, factory);
  return SENSCORD_STATUS_TRACE(status);
}
#endif  // SENSCORD_SERIALIZE

/**
 * @brief Release the used frame.
 * @param[in] (frameinfo) The information about used frame.
 * @return The status of function.
 */
void PublisherCore::ReleaseFrame(const FrameInfo& frameinfo) {
  PublisherParam param(this, callback_user_data_);
  callback_(param, frameinfo);
}

/**
  * @brief Set the user data for release frame callback.
  * @param[in] (user_data) The user data.
  * @return Status object.
  */
Status PublisherCore::SetCallbackUserData(uintptr_t user_data) {
  callback_user_data_ = user_data;
  return Status::OK();
}

/**
 * @brief Set allocators by keys.
 * @param[in] (allocator_keys) Memory allocator keys.
 * @return Status object.
 */
Status PublisherCore::SetAllocators(
    const std::map<std::string, std::string>& allocator_keys) {
  Status status;
  MemoryManager* memory_manager = MemoryManager::GetInstance();
  MemoryAllocator* allocator = NULL;
  for (std::map<std::string, std::string>::const_iterator
      itr = allocator_keys.begin(),
      end = allocator_keys.end(); itr != end; ++itr) {
    status = memory_manager->GetAllocator(itr->second, &allocator);
    SENSCORD_STATUS_TRACE(status);
    if (!status.ok()) {
      break;
    }
    allocators_.insert(std::make_pair(itr->first, allocator));
  }
  // if empty, add default allocator
  if (status.ok() && allocators_.empty()) {
    status = memory_manager->GetAllocator(kAllocatorDefaultKey, &allocator);
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      allocators_.insert(std::make_pair(kAllocatorNameDefault, allocator));
    }
  }
  return status;
}

/**
 * @brief Get the publisher state.
 * @return Publisher state.
 */
PublisherCore::PublisherState PublisherCore::GetState() const {
  util::AutoLock lock(state_mutex_);
  return state_;
}

/**
 * @brief Set the publisher state.
 * @param[in] (state) Pulbisher state.
 */
void PublisherCore::SetState(PublisherState state) {
  util::AutoLock lock(state_mutex_);
  state_ = state;
}

/* ------------------------------------------------------------------------- */

/**
 * @brief Constructor.
 * @param[in] (publisher) Parent publisher.
 * @param[in] (user_data) User data.
 */
PublisherParam::PublisherParam(Publisher* publisher, uintptr_t user_data)
    : publisher_(publisher), user_data_(user_data) {}

/**
 * @brief Destructor.
 */
PublisherParam::~PublisherParam() {}

/**
 * @brief Get the memory allocator by the name.
 * @param[in] (name) The memory allocator name.
 * @param[out] (allocator) The accessor of memory allocator.
 * @return Status object.
 */
Status PublisherParam::GetAllocator(
    const std::string& name, MemoryAllocator** allocator) const {
  return publisher_->GetAllocator(name, allocator);
}

/**
 * @brief Get the key when publisher is opened.
 * @return The pulbisher key.
 */
std::string PublisherParam::GetKey() const {
  return publisher_->GetKey();
}

/**
 * @brief Get the user data.
 * @return The user data.
 */
uintptr_t PublisherParam::GetUserData() const {
  return user_data_;
}

}   // namespace senscord
