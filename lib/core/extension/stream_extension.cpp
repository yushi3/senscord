/*
 * SPDX-FileCopyrightText: 2022-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/status.h"
#include "senscord/develop/extension.h"

#include "stream/stream_core.h"
#include "stream/property_history_book.h"

namespace senscord {

/**
 * @brief Constructor
 */
FrameExtensionAdapter::FrameExtensionAdapter()
    : frame_extension_type_(kFrameExtensionNormal), history_book_(),
      frame_extension_() {
  history_book_ = new PropertyHistoryBook();
}

/**
 * @brief Destructor
 */
FrameExtensionAdapter::~FrameExtensionAdapter() {
  delete frame_extension_;
  frame_extension_ = NULL;
  delete history_book_;
  history_book_ = NULL;
}

/**
 * @brief Get the memory allocator by the name.
 * @param[in] (name) The memory allocator name.
 * @param[out] (allocator) The accessor of memory allocator.
 * @return Status object.
 */
Status FrameExtensionAdapter::GetAllocator(
    const std::string& name, MemoryAllocator** allocator) const {
  if (allocator == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseInvalidArgument,
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

/**
 * @brief Initialize of the this class.
 * @param[in] (frame_extension) FrameExtension class.
 * @param[in] (type) Type of frame extension.
 * @param[in] (channels) Channel information to extend.
 * @param[in] (arguments) Arguments for FrameExtension.
 * @param[in] (allocators) Allocators for FrameExtension.
 * @return Status object.
 */
Status FrameExtensionAdapter::Init(
    FrameExtension* frame_extension,
    FrameExtensionType type,
    const std::map<uint32_t, ChannelInfo>& channels,
    const std::map<std::string, std::string>& arguments,
    const std::map<std::string, MemoryAllocator*>& allocators) {
  if (frame_extension_) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation, "already registered");
  }
  frame_extension_type_ = type;
  allocators_ = allocators;
  channel_info_ = channels;
  arguments_ = arguments;
  frame_extension->Init(this);
  frame_extension_ = frame_extension;
  return Status::OK();
}

/**
 * @brief Extension of Stream::GetFrame processing.
 * @param[in] (frame) Frame obtained by GetFrame
 * @param[out] (frameinfo) The information about extension frames.
 */
void FrameExtensionAdapter::ExtendFrame(
    const Frame* frame, ExtensionFrameInfo* frameinfo) {
  if (frame_extension_) {
    frame_extension_->ExtendFrame(frame, frameinfo);
  }
}

/**
 * @brief Extension of Stream::ReleaseFrame processing.
 * @param[in] (frameinfo) The information about used extension frames.
 */
void FrameExtensionAdapter::ReleaseFrame(const ExtensionFrameInfo& frameinfo) {
  if (frame_extension_) {
    frame_extension_->ReleaseFrame(frameinfo);
  }
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
Status FrameExtensionAdapter::SetUpdateChannelProperty(
    uint32_t channel_id, const std::string& key,
    const void* property, size_t size) {
  Status status = history_book_->SetProperty(channel_id, key, property, size);
  return SENSCORD_STATUS_TRACE(status);
}

#else
/**
 * @brief Set property to PropertyHistoryBook.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (property) Property to updated.
 * @param[in] (factory) Property factory.
 * @return Status object.
 */
Status FrameExtensionAdapter::SetUpdateChannelProperty(
    uint32_t channel_id, const std::string& key,
    const void* property, const PropertyFactoryBase& factory) {
  Status status = history_book_->SetProperty(
      channel_id, key, property, factory);
  return SENSCORD_STATUS_TRACE(status);
}
#endif

/* ------------------------------------------------------------------------- */

/**
 * @brief Constructor.
 */
StreamExtension::StreamExtension() : stream_(), adapter_() {}

/**
 * @brief Destructor.
 */
StreamExtension::~StreamExtension() {}

/**
 * @brief Initializes stream extension.
 * @param[in] (stream) stream to bind.
 * @param[in] (allocators) Allocators for FrameExtension.
 * @param[in] (adapter) FrameExtension adapter.
 */
void StreamExtension::Init(
    Stream* stream,
    const std::map<std::string, MemoryAllocator*>& allocators,
    FrameExtensionAdapter* adapter) {
  stream_ = stream;
  allocators_ = allocators;
  adapter_ = adapter;
}

/**
 * @brief Returns the stream pointer.
 */
Stream* StreamExtension::GetStream() const {
  return stream_;
}

/**
 * @brief Registers the property in the stream.
 * @param[in] (type) type of stream property.
 * @param[in] (accessor) property accessor to register.
 * @return Status object.
 */
Status StreamExtension::RegisterPropertyAccessor(
    StreamPropertyType type, PropertyAccessor* accessor) {
  Status status;
  StreamCore* stream = static_cast<StreamCore*>(stream_);
  if (type == kNormal) {
    status = stream->RegisterInternalPropertyAccessor(accessor);
    SENSCORD_STATUS_TRACE(status);
  } else {
    status = stream->RegisterSharedPropertyAccessor(accessor);
    SENSCORD_STATUS_TRACE(status);
  }
  return status;
}

}  // namespace senscord
