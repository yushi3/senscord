/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/pseudo_image_source.h"
#include <string>
#include <vector>
#include <utility>
#include "senscord/senscord.h"

// blockname for status
static const char kBlockName[] = "pseudo_image";

// Default values
static const uint32_t kDefaultFrameRateNum = 60;
static const uint32_t kDefaultWidth = 200;
static const uint32_t kDefaultHeight = 200;
static const uint32_t kDefaultBufferNum = 8;

// Channel Info
static const uint32_t kChannelMax = 2;

#define LOG_E(fmt, ...) \
    SENSCORD_LOG_ERROR_TAGGED(kBlockName, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) \
    SENSCORD_LOG_WARNING_TAGGED(kBlockName, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...) \
    SENSCORD_LOG_INFO_TAGGED(kBlockName, fmt, ##__VA_ARGS__)
#define LOG_D(fmt, ...) \
    SENSCORD_LOG_DEBUG_TAGGED(kBlockName, fmt, ##__VA_ARGS__)

/**
 * @brief Check to different from two properties.
 */
static bool IsDifferent(
    const senscord::ImageProperty& a, const senscord::ImageProperty& b) {
  if ((a.height != b.height) || (a.width != b.width) ||
      (a.stride_bytes != b.stride_bytes)) {
    return true;
  }
  return false;
}

/**
 * @brief Check to different from two properties.
 */
static bool IsDifferent(const PseudoImageProperty& a,
    const PseudoImageProperty& b) {
  if ((a.x != b.x) || (a.y != b.y) || (a.z != b.z)) {
    return true;
  }
  return false;
}

/**
 * @brief Get the round-up value.
 */
static uint32_t RoundUp(uint32_t value, uint32_t step) {
  return ((value + (step - 1)) / step) * step;
}

/**
 * @brief Constructor
 * @param[in] (allocator) Memory allocator for this source.
 */
PseudoImageSource::PseudoImageSource()
    : util_(), allocator_(), frame_seq_num_(0) {
  LOG_D("[pseudo] constructor");
  senscord::osal::OSCreateMutex(&memory_queue_mutex_);

  buffer_num_ = kDefaultBufferNum;
  last_time_nsec_ = 0;

  framerate_.num   = kDefaultFrameRateNum;
  framerate_.denom = 1;
  sleep_nsec_ = (1000000000ULL * framerate_.denom) / framerate_.num;

  image_property_.width  = kDefaultWidth;
  image_property_.height = kDefaultHeight;
  image_property_.stride_bytes = RoundUp(image_property_.width, 16);
  image_property_.pixel_format = senscord::kPixelFormatGREY;

  pseudo_image_.x = 100;
  pseudo_image_.y = 200;
  pseudo_image_.z = "hoge";
}

/**
 * @brief Destructor
 */
PseudoImageSource::~PseudoImageSource() {
  senscord::osal::OSDestroyMutex(memory_queue_mutex_);
}

/**
 * @brief Open the stream source.
 * @param[in] (core) The core instance.
 * @param[in] (util) The utility accessor to core.
 * @return The status of function.
 */
senscord::Status PseudoImageSource::Open(
    senscord::Core* core,
    senscord::StreamSourceUtility* util) {
  LOG_D("[pseudo] open");
  util_ = util;

  // get allocator (use default)
  senscord::Status status = util_->GetAllocator(
      senscord::kAllocatorNameDefault, &allocator_);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // register optional properties.
  SENSCORD_REGISTER_PROPERTY(util,
      kPseudoImagePropertyKey, PseudoImageProperty);

  // parse arguments
  // width, height
  {
    uint64_t value = 0;
    status = util_->GetStreamArgument("width", &value);
    if (status.ok()) {
      image_property_.width = static_cast<uint32_t>(value);
      image_property_.stride_bytes = RoundUp(static_cast<uint32_t>(value), 16);
    }
    LOG_I("[pseudo] width = %" PRIu32, image_property_.width);

    status = util_->GetStreamArgument("height", &value);
    if (status.ok()) {
      image_property_.height = static_cast<uint32_t>(value);
    }
    LOG_I("[pseudo] height = %" PRIu32, image_property_.height);
  }

  // framerate
  {
    uint64_t fps = 0;
    status = util_->GetStreamArgument("fps", &fps);
    if (status.ok() && fps > 0) {
      senscord::FrameRateProperty framerate = {};
      framerate.num = static_cast<uint32_t>(fps);
      framerate.denom = 1;
      Set(senscord::kFrameRatePropertyKey, &framerate);
    }
    LOG_I("[pseudo] framerate = %" PRIu32 " / %" PRIu32,
          framerate_.num, framerate_.denom);
  }

  // buffer_num
  {
    uint64_t buffer_num = 0;
    status = util_->GetStreamArgument("buffer_num", &buffer_num);
    if (status.ok()) {
      buffer_num_ = static_cast<uint32_t>(buffer_num);
    }
    LOG_I("[pseudo] buffer_num = %" PRIu32, buffer_num_);
  }

  // set channel property
  for (uint32_t index = 0; index < kChannelMax; ++index) {
    util_->UpdateChannelProperty(senscord::kChannelIdImage(index),
        senscord::kImagePropertyKey, &image_property_);
  }

  return senscord::Status::OK();
}

/**
 * @brief Close the stream source.
 * @return The status of function.
 */
senscord::Status PseudoImageSource::Close() {
  LOG_D("[pseudo] close");
  ClearMemory();
  return senscord::Status::OK();
}

/**
 * @brief Clear all memory.
 */
void PseudoImageSource::ClearMemory() {
  for (std::vector<senscord::Memory*>::const_iterator
      itr = memory_list_.begin(), end = memory_list_.end();
      itr != end; ++itr) {
    allocator_->Free(*itr);
    LOG_D("Free: %p", *itr);
  }
  memory_list_.clear();
  senscord::osal::OSLockMutex(memory_queue_mutex_);
  memory_queue_.clear();
  senscord::osal::OSUnlockMutex(memory_queue_mutex_);
}

/**
 * @brief Create memory.
 * @return The status of function.
 */
senscord::Status PseudoImageSource::CreateMemory() {
  if (buffer_num_ > 0) {
    size_t frame_size = image_property_.height * image_property_.stride_bytes;
    for (uint32_t index = 0; index < buffer_num_; ++index) {
      senscord::Memory* memory = NULL;
      senscord::Status status = allocator_->Allocate(frame_size, &memory);
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
      memory_list_.push_back(memory);
      senscord::osal::OSLockMutex(memory_queue_mutex_);
      memory_queue_.push_back(memory);
      senscord::osal::OSUnlockMutex(memory_queue_mutex_);
      LOG_D("Allocate: %p, %" PRIuS, memory, frame_size);
    }
  }
  return senscord::Status::OK();
}

/**
 * @brief Get memory.
 * @return The memory.
 */
senscord::Memory* PseudoImageSource::GetMemory() {
  senscord::Memory* memory = NULL;
  if (buffer_num_ > 0) {
    senscord::osal::OSLockMutex(memory_queue_mutex_);
    if (!memory_queue_.empty()) {
      memory = memory_queue_.front();
      memory_queue_.pop_front();
    }
    senscord::osal::OSUnlockMutex(memory_queue_mutex_);
  } else {
    size_t frame_size = image_property_.height * image_property_.stride_bytes;
    allocator_->Allocate(frame_size, &memory);
    LOG_D("Allocate: %p, %" PRIuS, memory, frame_size);
  }
  return memory;
}

/**
 * @brief Release memory.
 * @param[in] (memory) The memory to release.
 */
void PseudoImageSource::ReleaseMemory(senscord::Memory* memory) {
  if (buffer_num_ > 0) {
    senscord::osal::OSLockMutex(memory_queue_mutex_);
    memory_queue_.push_back(memory);
    senscord::osal::OSUnlockMutex(memory_queue_mutex_);
  } else {
    allocator_->Free(memory);
    LOG_D("Free: %p", memory);
  }
}

/**
 * @brief Start the stream source.
 * @return The status of function.
 */
senscord::Status PseudoImageSource::Start() {
  senscord::Status status;
  ClearMemory();
  status = CreateMemory();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  sleep_nsec_ = (1000000000ULL * framerate_.denom) / framerate_.num;
  SENSCORD_LOG_INFO("[pseudo] sleep_nsec_ = %" PRIu64, sleep_nsec_);
  senscord::osal::OSGetTime(&last_time_nsec_);

  return senscord::Status::OK();
}

/**
 * @brief Stop the stream source.
 * @return The status of function.
 */
senscord::Status PseudoImageSource::Stop() {
  last_time_nsec_ = 0;
  return senscord::Status::OK();
}

/**
 * @brief Pull up the new frames.
 * @param[out] (frames) The information about new frames.
 */
void PseudoImageSource::GetFrames(std::vector<senscord::FrameInfo>* frames) {
  uint64_t current_time = 0;
  senscord::osal::OSGetTime(&current_time);

  uint64_t seq_num = frame_seq_num_;
  frame_seq_num_ += 1;
  last_time_nsec_ += sleep_nsec_;
  if (last_time_nsec_ > current_time) {
    uint64_t wait_time = last_time_nsec_ - current_time;
    senscord::osal::OSSleep(wait_time);
  }

  senscord::FrameInfo frameinfo = {};
  frameinfo.sequence_number = seq_num;

  for (uint32_t index = 0; index < kChannelMax; ++index) {
    senscord::Memory* memory = GetMemory();
    if (memory == NULL) {
      // No Buffer available
      LOG_W("[pseudo] drop (seq=%" PRIu64 "): no buffer left", seq_num);
      util_->SendEventFrameDropped(seq_num);
      ReleaseFrame(frameinfo, NULL);
      return;
    }

    senscord::osal::OSMemset(
        reinterpret_cast<uint8_t*>(memory->GetAddress()),
        static_cast<uint8_t>(seq_num & 0xFF),
        memory->GetSize());

    senscord::ChannelRawData channel = {};
    channel.channel_id = senscord::kChannelIdImage(index);
    channel.data_type = senscord::kRawDataTypeImage;
    channel.data_memory = memory;
    channel.data_size = memory->GetSize();
    channel.data_offset = 0;
    channel.captured_timestamp = last_time_nsec_;
    frameinfo.channels.push_back(channel);
  }

  frames->push_back(frameinfo);
}

/**
 * @brief Release the used frame.
 * @param[in] (frameinfo) The information about used frame.
 * @param[in] (referenced_channel_ids) List of referenced channel IDs.
 *                                     (NULL is the same as empty)
 * @return The status of function.
 */
senscord::Status PseudoImageSource::ReleaseFrame(
    const senscord::FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids) {
  for (std::vector<senscord::ChannelRawData>::const_iterator
      itr = frameinfo.channels.begin(), end = frameinfo.channels.end();
      itr != end; ++itr) {
    if (itr->data_memory != NULL) {
      ReleaseMemory(itr->data_memory);
    }
  }
  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status PseudoImageSource::Get(
    const std::string& key, senscord::ChannelInfoProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidArgument, "Null pointer");
  }
  for (uint32_t index = 0; index < kChannelMax; ++index) {
    senscord::ChannelInfo info = {};
    info.raw_data_type = senscord::kRawDataTypeImage;
    info.description = "Sample image raw data";

    property->channels.insert(
        std::make_pair(senscord::kChannelIdImage(index), info));
  }
  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status PseudoImageSource::Get(
    const std::string& key, senscord::FrameRateProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidArgument, "Null pointer");
  }
  *property = framerate_;
  return senscord::Status::OK();
}

/**
 * @brief Set the new stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status PseudoImageSource::Set(
    const std::string& key, const senscord::FrameRateProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidArgument, "Null pointer");
  }
  if (property->denom == 0 || property->num == 0) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidArgument, "0 value");
  }

  uint64_t new_sleep_nsec =
      ((1000ULL * 1000 * 1000) * property->denom) / property->num;

  if (sleep_nsec_ != new_sleep_nsec) {
    framerate_ = *property;
    sleep_nsec_ = new_sleep_nsec;

    LOG_I("change framerate to %" PRId32 " / %" PRId32,
          property->num, property->denom);

    // notify to streams
    util_->SendEventPropertyUpdated(key);
  }
  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status PseudoImageSource::Get(
    const std::string& key, senscord::ImageProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidArgument, "Null pointer");
  }
  *property = image_property_;
  return senscord::Status::OK();
}

/**
 * @brief Set the new stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status PseudoImageSource::Set(
    const std::string& key, const senscord::ImageProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidArgument, "Null pointer");
  }

  if (image_property_.pixel_format != property->pixel_format) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidArgument,
        "Changing pixel format is not supported");
  }
  if (last_time_nsec_ > 0) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidOperation, "already started");
  }

  if (IsDifferent(image_property_, *property)) {
    image_property_ = *property;
    image_property_.stride_bytes = RoundUp(image_property_.width, 16);

    // notify to channel
    for (uint32_t index = 0; index < kChannelMax; ++index) {
      util_->UpdateChannelProperty(senscord::kChannelIdImage(index),
          senscord::kImagePropertyKey, &image_property_);
    }

    // notify to streams
    util_->SendEventPropertyUpdated(key);
  }
  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status PseudoImageSource::Get(
    const std::string& key,
    senscord::ImageSensorFunctionSupportedProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidArgument, "Null pointer");
  }
  property->auto_exposure_supported = false;
  property->auto_white_balance_supported = false;
  property->brightness_supported = false;
  property->iso_sensitivity_supported = false;
  property->exposure_time_supported = false;
  property->exposure_metering_supported = false;
  property->gamma_value_supported = false;
  property->gain_value_supported = false;
  property->hue_supported = false;
  property->saturation_supported = false;
  property->sharpness_supported = false;
  property->white_balance_supported = false;
  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status PseudoImageSource::Get(
    const std::string& key, PseudoImageProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidArgument, "Null pointer");
  }
  *property = pseudo_image_;
  return senscord::Status::OK();
}

/**
 * @brief Set the new stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status PseudoImageSource::Set(
    const std::string& key, const PseudoImageProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidArgument, "Null pointer");
  }

  if (IsDifferent(pseudo_image_, *property)) {
    pseudo_image_ = *property;

    // notify to streams
    util_->SendEventPropertyUpdated(key);
  }
  return senscord::Status::OK();
}
