/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/opencv_image_source.h"

#include <string>
#include <vector>
#include <utility>

#include "senscord/senscord.h"
#include "senscord/osal.h"
#include "senscord/logger.h"

#include <opencv2/imgproc.hpp>

namespace {

// blockname for status
const char kBlockName[] = "OpenCvImage";
const char kUseAllocatorName[] = "image";

// Default values
const uint32_t kDefaultDeviceId  = 0;
const uint32_t kDefaultBufferNum = 4;  // Default buffer num.

}  // namespace

/**
 * @brief Constructor
 */
OpenCvImageSource::OpenCvImageSource()
    : util_(), allocator_() {
  device_id_ = kDefaultDeviceId;
  buffer_num_ = kDefaultBufferNum;
  frame_seq_num_ = 0;
  running_ = false;
  senscord::osal::OSCreateMutex(&memory_queue_mutex_);
}

/**
 * @brief Destructor
 */
OpenCvImageSource::~OpenCvImageSource() {
  senscord::osal::OSDestroyMutex(memory_queue_mutex_);
  memory_queue_mutex_ = NULL;
}

/**
 * @brief Open the stream source.
 * @param[in] (core) The core instance.
 * @param[in] (util) The utility accessor to core.
 * @return The status of function.
 */
senscord::Status OpenCvImageSource::Open(
    senscord::Core* core,
    senscord::StreamSourceUtility* util) {
  SENSCORD_LOG_DEBUG("[opencv] Open");
  util_ = util;

  // get allocator
  // if there is no specified, use default.
  senscord::Status status = util_->GetAllocator(
      kUseAllocatorName, &allocator_);
  if (!status.ok()) {
    status = util_->GetAllocator(
        senscord::kAllocatorNameDefault, &allocator_);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }

  // parse arguments
  {
    // device_id
    int64_t value = 0;
    status = util_->GetInstanceArgument("device_id", &value);
    if (status.ok()) {
      device_id_ = static_cast<int32_t>(value);
    }
  }
  {
    // buffer_num
    uint64_t value = 0;
    status = util_->GetInstanceArgument("buffer_num", &value);
    if (status.ok()) {
      buffer_num_ = static_cast<uint32_t>(value);
    }
  }
  {
    // filename
    std::string value;
    status = util_->GetInstanceArgument("filename", &value);
    if (status.ok()) {
      filename_ = value;
    }
  }

  if (filename_.empty()) {
    // open camera device
    SENSCORD_LOG_INFO("[opencv] device_id = %" PRId32, device_id_);
    SENSCORD_LOG_INFO("[opencv] buffer_num = %" PRIu32, buffer_num_);
    bool ret = video_.open(device_id_);
    if (!ret) {
      return SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseInvalidArgument,
          "Unable to open camera(%" PRId32 ")", device_id_);
    }
  } else {
    // open video or image file
    SENSCORD_LOG_INFO("[opencv] filename = %s", filename_.c_str());
    SENSCORD_LOG_INFO("[opencv] buffer_num = %" PRIu32, buffer_num_);
    bool ret = video_.open(filename_);
    if (!ret) {
      return SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseInvalidArgument,
          "Unable to open file(%s)", filename_.c_str());
    }
  }

  // get property
  Get(senscord::kImagePropertyKey, &image_property_);

  // ChannelProperty
  util_->UpdateChannelProperty(senscord::kChannelIdImage(0),
      senscord::kImagePropertyKey, &image_property_);

  return senscord::Status::OK();
}

/**
 * @brief Close the stream source.
 * @return The status of function.
 */
senscord::Status OpenCvImageSource::Close() {
  SENSCORD_LOG_DEBUG("[opencv] Close");
  video_.release();
  ClearMemoryList();
  return senscord::Status::OK();
}

/**
 * @brief Create memory list.
 */
senscord::Status OpenCvImageSource::CreateMemoryList() {
  if (buffer_num_ > 0) {
    size_t frame_size = image_property_.width * image_property_.height * 2;
    for (uint32_t i = 0; i < buffer_num_; ++i) {
      senscord::Memory* memory = NULL;
      senscord::Status status = allocator_->Allocate(frame_size, &memory);
      if (!status.ok()) {
        return SENSCORD_STATUS_TRACE(status);
      }
      memory_list_.push_back(memory);
      senscord::osal::OSLockMutex(memory_queue_mutex_);
      memory_queue_.push(memory);
      senscord::osal::OSUnlockMutex(memory_queue_mutex_);
    }
  }
  return senscord::Status::OK();
}

/**
 * @brief Clear memory list.
 */
void OpenCvImageSource::ClearMemoryList() {
  if (buffer_num_ > 0) {
    for (std::vector<senscord::Memory*>::const_iterator
        itr = memory_list_.begin(), end = memory_list_.end();
        itr != end; ++itr) {
      allocator_->Free(*itr);
    }
    memory_list_.clear();
    senscord::osal::OSLockMutex(memory_queue_mutex_);
    std::queue<senscord::Memory*> empty;
    std::swap(memory_queue_, empty);
    senscord::osal::OSUnlockMutex(memory_queue_mutex_);
  }
}

/**
 * @brief Get memory.
 */
senscord::Memory* OpenCvImageSource::GetMemory() {
  senscord::Memory* memory = NULL;
  if (buffer_num_ > 0) {
    senscord::osal::OSLockMutex(memory_queue_mutex_);
    if (!memory_queue_.empty()) {
      memory = memory_queue_.front();
      memory_queue_.pop();
    }
    senscord::osal::OSUnlockMutex(memory_queue_mutex_);
  } else {
    size_t frame_size = image_property_.width * image_property_.height * 2;
    allocator_->Allocate(frame_size, &memory);
  }
  return memory;
}

/**
 * @brief Release memory.
 */
void OpenCvImageSource::ReleaseMemory(senscord::Memory* memory) {
  if (memory == NULL) {
    return;
  }
  if (buffer_num_ > 0) {
    senscord::osal::OSLockMutex(memory_queue_mutex_);
    memory_queue_.push(memory);
    senscord::osal::OSUnlockMutex(memory_queue_mutex_);
  } else {
    allocator_->Free(memory);
  }
}

/**
 * @brief Start the stream source.
 * @return The status of function.
 */
senscord::Status OpenCvImageSource::Start() {
  SENSCORD_LOG_DEBUG("[opencv] Start");
  ClearMemoryList();
  senscord::Status status = CreateMemoryList();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  running_ = true;
  return senscord::Status::OK();
}

/**
 * @brief Stop the stream source.
 * @return The status of function.
 */
senscord::Status OpenCvImageSource::Stop() {
  SENSCORD_LOG_DEBUG("[opencv] Stop");
  running_ = false;
  return senscord::Status::OK();
}

/**
 * @brief Pull up the new frames.
 * @param[out] (frames) The information about new frames.
 */
void OpenCvImageSource::GetFrames(std::vector<senscord::FrameInfo>* frames) {
  bool ret = video_.grab();
  if (!ret) {
    if (!filename_.empty()) {
      video_.set(cv::CAP_PROP_POS_FRAMES, 0);  // position reset (movie loop)
    }
    senscord::FrameRateProperty framerate = {};
    Get(senscord::kFrameRatePropertyKey, &framerate);
    uint64_t wait = 1000000000UL * framerate.denom / framerate.num;
    senscord::osal::OSSleep(wait);
    return;
  }

  uint64_t seq_num = frame_seq_num_;
  frame_seq_num_ += 1;

  // read BGR image
  cv::Mat image_bgr;
  ret = video_.retrieve(image_bgr);
  if (!ret) {
    SENSCORD_LOG_INFO(
        "[opencv] failed to read video capture. seq_num=%" PRIu64, seq_num);
    util_->SendEventFrameDropped(seq_num);
    return;
  }

  uint64_t timestamp = 0;
  senscord::osal::OSGetTime(&timestamp);

  senscord::Memory* memory = GetMemory();
  if (memory == NULL) {
    // No Buffer available
    util_->SendEventFrameDropped(seq_num);
    return;
  }

  // convert BGR to YUV (Mat CV_8UC3: Packed Y-Cb-Cr 24bit)
  cv::Mat image_yuv;
  cv::cvtColor(image_bgr, image_yuv, cv::COLOR_BGR2YUV);

  // convert YUV to NV16 (Semi-Planar YUV 4:2:2)
  size_t size = image_yuv.total();
  uint8_t* yuv_addr = image_yuv.ptr();
  uint8_t* nv16_addr = reinterpret_cast<uint8_t*>(memory->GetAddress());
  for (size_t i = 0; i < size; ++i) {
    // Y
    nv16_addr[i] = *yuv_addr;
    // Cb or Cr
    nv16_addr[size + i] = (i % 2 == 0) ? *(yuv_addr + 1) : *(yuv_addr + 2);
    yuv_addr += 3;
  }

  // setup FrameInfo
  senscord::ChannelRawData channel = {};
  channel.channel_id = senscord::kChannelIdImage(0);
  channel.data_type = senscord::kRawDataTypeImage;
  channel.data_memory = memory;
  channel.data_size = memory->GetSize();
  channel.data_offset = 0;
  channel.captured_timestamp = timestamp;

  senscord::FrameInfo frameinfo = {};
  frameinfo.sequence_number = seq_num;
  frameinfo.channels.push_back(channel);

  frames->push_back(frameinfo);
}

/**
 * @brief Release the used frame.
 * @param[in] (frameinfo) The information about used frame.
 * @param[in] (referenced_channel_ids) List of referenced channel IDs.
 *                                     (NULL is the same as empty)
 * @return The status of function.
 */
senscord::Status OpenCvImageSource::ReleaseFrame(
    const senscord::FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids) {
  ReleaseMemory(frameinfo.channels[0].data_memory);
  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status OpenCvImageSource::Get(
    const std::string& key, senscord::ChannelInfoProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidArgument, "Null pointer");
  }
  senscord::ChannelInfo info = {};
  info.raw_data_type = senscord::kRawDataTypeImage;
  info.description = "Image data NV16";
  property->channels[0] = info;
  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status OpenCvImageSource::Get(
    const std::string& key, senscord::FrameRateProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidArgument, "Null pointer");
  }
  double fps = video_.get(cv::CAP_PROP_FPS);
  if (fps > 0.0) {
    // fraction(fps, &property->num, &property->denom);
    property->num = static_cast<uint32_t>(fps);
    property->denom = 1;
  } else {
    // TODO: Not working with some backends.
    property->num = 30;
    property->denom = 1;
  }
  return senscord::Status::OK();
}

/**
 * @brief Set the new stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status OpenCvImageSource::Set(
    const std::string& key, const senscord::FrameRateProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidArgument, "Null pointer");
  }
  if (property->denom == 0 || property->num == 0) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidArgument, "0 value");
  }

  // set
  double fps = static_cast<double>(property->num) / property->denom;
  video_.set(cv::CAP_PROP_FPS, fps);

  // get for confirmation
  senscord::FrameRateProperty framerate = {};
  Get(senscord::kFrameRatePropertyKey, &framerate);

  if (property->num != framerate.num ||
      property->denom != framerate.denom) {
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
senscord::Status OpenCvImageSource::Get(
    const std::string& key, senscord::ImageProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidArgument, "Null pointer");
  }

  uint32_t width = static_cast<uint32_t>(
      video_.get(cv::CAP_PROP_FRAME_WIDTH));
  uint32_t height = static_cast<uint32_t>(
      video_.get(cv::CAP_PROP_FRAME_HEIGHT));

  SENSCORD_LOG_INFO("[opencv] width  = %" PRIu32, width);
  SENSCORD_LOG_INFO("[opencv] height = %" PRIu32, height);

  property->width = width;
  property->height = height;
  // The format is always NV16 (BGR -> NV16)
  property->stride_bytes = width;
  property->pixel_format = senscord::kPixelFormatNV16;

  return senscord::Status::OK();
}

/**
 * @brief Set the new stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status OpenCvImageSource::Set(
    const std::string& key, const senscord::ImageProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidArgument, "Null pointer");
  }
  if (running_) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidOperation,
        "Cannot be set during streaming.");
  }

  // set
  video_.set(cv::CAP_PROP_FRAME_WIDTH, property->width);
  video_.set(cv::CAP_PROP_FRAME_HEIGHT, property->height);

  // get for confirmation
  senscord::ImageProperty image = {};
  Get(senscord::kImagePropertyKey, &image);

  if (image_property_.width != image.width ||
      image_property_.height != image.height) {
    image_property_ = image;
    // ChannelProperty
    util_->UpdateChannelProperty(senscord::kChannelIdImage(0),
        senscord::kImagePropertyKey, &image_property_);
    // notify to streams
    util_->SendEventPropertyUpdated(key);
  }

  return senscord::Status::OK();
}
