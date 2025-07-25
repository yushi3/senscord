/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "v4l2_image_stream_source.h"
#include <inttypes.h>
#include <stdint.h>
#include <vector>
#include <map>

#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"
#include "senscord/logger.h"

#define LOG_E(...) SENSCORD_LOG_ERROR_TAGGED(kBlockName, __VA_ARGS__)
#define LOG_W(...) SENSCORD_LOG_WARNING_TAGGED(kBlockName, __VA_ARGS__)
#define LOG_I(...) SENSCORD_LOG_INFO_TAGGED(kBlockName, __VA_ARGS__)
#define LOG_D(...) SENSCORD_LOG_DEBUG_TAGGED(kBlockName, __VA_ARGS__)

namespace {
const char kBlockName[] = "v4l2_image";
const char kUseAllocatorName[] = "image";

// default values
const char kDefaultDevice[]       = "/dev/video0";
const int32_t kDefaultBufferNum   = 6;
const int32_t kDefaultWidth       = 640;
const int32_t kDefaultHeight      = 480;
const int32_t kDefaultStrideBytes = 640;
const char*   kDefaultPixelFormat = senscord::kPixelFormatNV16;
const int32_t kDefaultFramerate   = 30;

// spare buffer number
const int32_t kSpareBufferNum     = 2;

/**
 * @brief Lock guard.
 */
class LockGuard {
 public:
  explicit LockGuard(senscord::osal::OSMutex* mutex) : mutex_(mutex) {
    senscord::osal::OSLockMutex(mutex_);
  }

  ~LockGuard() {
    senscord::osal::OSUnlockMutex(mutex_);
  }

 private:
  senscord::osal::OSMutex* mutex_;
};

/**
 * @brief Get nano sec timestamp.
 * @param[in] (time) Structure to convert nano sec.
 * @return nano sec timestamp.
 */
uint64_t GetNsecTimestamp(const struct timeval& time) {
  return ((static_cast<uint64_t>(time.tv_sec) * 1000000000ULL) +
      (time.tv_usec * 1000));
}
}  // namespace

/**
 * @brief Constructor.
 */
V4L2ImageStreamSource::V4L2ImageStreamSource()
    : util_(), settings_(), frame_seq_num_(0), allocator_(),
      image_property_(), framerate_property_(), is_started_(),
      is_yuyv_to_nv16_(true) {
  settings_.device       = kDefaultDevice;
  settings_.buffer_num   = kDefaultBufferNum + kSpareBufferNum;

  image_property_.width        = kDefaultWidth;
  image_property_.height       = kDefaultHeight;
  image_property_.stride_bytes = kDefaultStrideBytes;
  image_property_.pixel_format = kDefaultPixelFormat;

  framerate_property_.num = kDefaultFramerate;
  framerate_property_.denom = 1;

  senscord::osal::OSCreateMutex(&buffer_mutex_);
}

/**
 * @brief Destructor.
 */
V4L2ImageStreamSource::~V4L2ImageStreamSource() {
  FreeBuffer();
  senscord::osal::OSDestroyMutex(buffer_mutex_);
  buffer_mutex_ = NULL;
}

/**
 * @brief Open the stream source.
 * @param[in] (core) The core instance.
 * @param[in] (util) The utility accessor to core.
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::Open(
    senscord::Core* core,
    senscord::StreamSourceUtility* util) {
  util_ = util;

  // get allocator
  // if there is no specified, use default.
  senscord::Status status = util_->GetAllocator(
      kUseAllocatorName, &allocator_);
  SENSCORD_STATUS_TRACE(status);
  if (!status.ok()) {
      status = util_->GetAllocator(
          senscord::kAllocatorNameDefault, &allocator_);
      SENSCORD_STATUS_TRACE(status);
  }

  if (status.ok()) {
    // parse xml parameter
    ParseParameter();

    // open device
    status = device_.DevOpen(settings_.device);
    SENSCORD_STATUS_TRACE(status);
  }
  return status;
}

/**
 * @brief Close the stream source.
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::Close() {
  FreeBuffer();

  senscord::Status status = device_.DevClose();
  if (!status.ok()) {
    LOG_W("%s", status.ToString().c_str());
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Start the stream source.
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::Start() {
  // for restart, clear buffer
  FreeBuffer();

  // setting to device
  senscord::Status status = SetDeviceParameter();
  SENSCORD_STATUS_TRACE(status);

  if (status.ok()) {
    // allocate memory
    status = AllocateBuffer();
    SENSCORD_STATUS_TRACE(status);
  }

  if (status.ok()) {
    // queuing buffer
    for (size_t index = 0; index < buffer_list_.size(); ++index) {
      struct v4l2_buffer buffer = {};
      status = device_.QueryBuffer(index, &buffer);
      if (!status.ok()) {
        SENSCORD_STATUS_TRACE(status);
        break;
      }
      status = device_.QueueBuffer(&buffer);
      if (!status.ok()) {
        SENSCORD_STATUS_TRACE(status);
        break;
      }
    }
  }

  if (status.ok()) {
    // start streaming
    status = device_.DevStart();
    SENSCORD_STATUS_TRACE(status);
  }

  if (status.ok()) {
    // update frame property
    senscord::ImageProperty update = image_property_;
    if (update.pixel_format == senscord::kPixelFormatYUYV &&
        is_yuyv_to_nv16_) {
      update.pixel_format = senscord::kPixelFormatNV16;
    }
    util_->UpdateChannelProperty(
        0, senscord::kImagePropertyKey, &update);
    is_started_ = true;
  }

  if (!status.ok()) {
    FreeBuffer();
  }
  return status;
}

/**
 * @brief Stop the stream source.
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::Stop() {
  // streaming stop
  senscord::Status status = device_.DevStop();
  if (status.ok()) {
    is_started_ = false;
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Pull up the new frames.
 * @param[out] (frames) The information about new frames.
 */
void V4L2ImageStreamSource::GetFrames(
    std::vector<senscord::FrameInfo>* frames) {
  struct v4l2_buffer buffer = {};
  senscord::Status status = device_.DequeueBuffer(&buffer);
  if (!status.ok()) {
    LOG_D("dequeue error, next buffer");
    return;   // next buffer
  }
  // always have a buffer on the device.
  if (GetUsedBufferNum() >= (settings_.buffer_num - kSpareBufferNum)) {
    LOG_D("no buffer available: index=%" PRIu32, buffer.index);
    device_.QueueBuffer(&buffer);
    return;
  }

  BufferInfo* buffer_info = NULL;
  {
    LockGuard autolock(buffer_mutex_);
    buffer_info = &buffer_list_[buffer.index];
    buffer_info->used = true;
  }
  void* src_addr = buffer_info->addr;
  uint32_t src_size = buffer.bytesused;
  void* dest_addr =
      reinterpret_cast<void*>(buffer_info->memory->GetAddress());
  size_t dest_size = buffer_info->memory->GetSize();

  if (image_property_.pixel_format == senscord::kPixelFormatYUYV &&
      is_yuyv_to_nv16_) {
    /* yuyv to nv16 color format conversion */
    src_size = (image_property_.width) * (image_property_.height) * 2;
    std::vector<char> dest_nv16_address(src_size);
    char* byte_element = reinterpret_cast<char*>(src_addr);
    for (uint32_t i = 0; i < src_size; ++i) {
      if (i % 2 == 0) {
          dest_nv16_address[i / 2] = *byte_element;
      } else {
          dest_nv16_address[(src_size + i - 1) / 2] = *byte_element;
      }
      ++byte_element;
    }
    senscord::osal::OSMemcpy(
        dest_addr, dest_size, &dest_nv16_address[0], src_size);
  } else {
    // copy without conversion
    senscord::osal::OSMemcpy(dest_addr, dest_size, src_addr, src_size);
  }

  senscord::FrameInfo frame = {};
  frame.sequence_number = frame_seq_num_++;
  senscord::ChannelRawData channel = {};
  channel.channel_id = 0;
  channel.data_type = senscord::kRawDataTypeImage;
  channel.data_memory = buffer_info->memory;
  channel.data_size = src_size;
  channel.captured_timestamp = GetNsecTimestamp(buffer.timestamp);
  LOG_D("Send: seq_num=%" PRIu64 ", ts=%" PRIu64,
      frame.sequence_number, channel.captured_timestamp);
  frame.channels.push_back(channel);
  frames->push_back(frame);
}

/**
 * @brief Release the used frame.
 * @param[in] (frameinfo) The information about used frame.
 * @param[in] (referenced_channel_ids) List of referenced channel IDs.
 *                                     (NULL is the same as empty)
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::ReleaseFrame(
    const senscord::FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids) {
  struct v4l2_buffer buffer = {};
  const senscord::ChannelRawData& channel = frameinfo.channels[0];

  LockGuard autolock(buffer_mutex_);
  for (std::vector<BufferInfo>::iterator itr = buffer_list_.begin();
      itr != buffer_list_.end(); ++itr) {
    if (itr->memory == channel.data_memory) {
      LOG_D("Release: seq_num=%" PRIu64 ", ts=%" PRIu64,
          frameinfo.sequence_number, channel.captured_timestamp);
      itr->used = false;
      device_.QueryBuffer(itr->index, &buffer);
      device_.QueueBuffer(&buffer);
      break;
    }
  }
  return senscord::Status::OK();
}

/**
 * @brief Set parameter to device.
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::SetDeviceParameter() {
  senscord::Status status;

  // set format
  status = device_.SetDevFormat(image_property_);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    senscord::ImageProperty device_param = {};
    status = device_.GetDevFormat(&device_param);
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      LOG_I("device: width=%" PRIu32 ", height=%" PRIu32
          ", stride_bytes=%" PRIu32 ", pixel_format=%s",
          device_param.width, device_param.height,
          device_param.stride_bytes, device_param.pixel_format.c_str());
      if (device_param.pixel_format == senscord::kPixelFormatYUYV &&
          is_yuyv_to_nv16_) {
        device_param.stride_bytes = device_param.stride_bytes / 2;
      }
      // correct device parameters
      image_property_ = device_param;
    }
  }

  if (status.ok()) {
    // set framerate
    status = device_.SetFramerate(framerate_property_);
    SENSCORD_STATUS_TRACE(status);
  }
  if (status.ok()) {
    senscord::FrameRateProperty device_param = {};
    status = device_.GetFramerate(&device_param);
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      LOG_I("device: num=%" PRIu32 ", denom=%" PRIu32,
          device_param.num, device_param.denom);
      // correct device parameters
      framerate_property_ = device_param;
    }
  }
  return status;
}

/**
 * @brief Allocate device buffer.
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::AllocateBuffer() {
  senscord::Status status = device_.SetReqBuffer(settings_.buffer_num);
  SENSCORD_STATUS_TRACE(status);

  if (status.ok()) {
    for (uint32_t index = 0; index < settings_.buffer_num; ++index) {
      BufferInfo buffer = {};
      buffer.index = index;
      buffer.used = false;
      status = device_.Mmap(index, &buffer.addr, &buffer.length);
      SENSCORD_STATUS_TRACE(status);
      if (!status.ok()) {
        break;
      }
      senscord::Memory* memory = NULL;
      status = allocator_->Allocate(buffer.length, &memory);
      SENSCORD_STATUS_TRACE(status);
      if (!status.ok()) {
        device_.Munmap(buffer.addr, buffer.length);
        break;
      }
      buffer.memory = memory;
      LockGuard autolock(buffer_mutex_);
      buffer_list_.push_back(buffer);
    }
  }

  if (!status.ok()) {
    FreeBuffer();
  }
  return status;
}

/**
 * @brief Free device buffer.
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::FreeBuffer() {
  LockGuard autolock(buffer_mutex_);
  while (!buffer_list_.empty()) {
    BufferInfo buffer = buffer_list_.back();
    senscord::Status status = device_.Munmap(buffer.addr, buffer.length);
    if (!status.ok()) {
      LOG_W(status.ToString().c_str());
    }
    allocator_->Free(buffer.memory);
    buffer_list_.pop_back();
  }
  senscord::Status status = device_.FreeReqBuffer();
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Get property.
 * @param[in] (key) Property key.
 * @param[out] (property) Property.
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::Get(const std::string& key,
    senscord::ChannelInfoProperty* property) {
  senscord::ChannelInfo channel_info = {};
  channel_info.raw_data_type = senscord::kRawDataTypeImage;
  channel_info.description = "Image data from a V4L2 device.";
  property->channels[0] = channel_info;
  return senscord::Status::OK();
}

/**
 * @brief Get property.
 * @param[in] (key) Property key.
 * @param[out] (property) Property.
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::Get(const std::string& key,
    senscord::ImageSensorFunctionSupportedProperty* property) {
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
 * @brief Get property.
 * @param[in] (key) Property key.
 * @param[out] (property) Property.
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::Get(const std::string& key,
    senscord::FrameRateProperty* property) {
  *property = framerate_property_;
  return senscord::Status::OK();
}

/**
 * @brief Set property.
 * @param[in] (key) Property key.
 * @param[in] (property) Property.
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::Set(const std::string& key,
    const senscord::FrameRateProperty* property) {
  if (is_started_) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidOperation,
        "already streaming");
  }
  framerate_property_ = *property;
  return senscord::Status::OK();
}

/**
 * @brief Get property.
 * @param[in] (key) Property key.
 * @param[out] (property) Property.
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::Get(const std::string& key,
    senscord::ImageProperty* property) {
  *property = image_property_;
  // overwrite format when in NV16 conversion mode
  if (image_property_.pixel_format == senscord::kPixelFormatYUYV &&
      is_yuyv_to_nv16_) {
    property->pixel_format = senscord::kPixelFormatNV16;
  }
  return senscord::Status::OK();
}

/**
 * @brief Set property.
 * @param[in] (key) Property key.
 * @param[in] (property) Property.
 * @return Status object.
 */
senscord::Status V4L2ImageStreamSource::Set(const std::string& key,
    const senscord::ImageProperty* property) {
  if (is_started_) {
    return SENSCORD_STATUS_FAIL(kBlockName,
        senscord::Status::kCauseInvalidOperation,
        "already streaming");
  }
  image_property_ = *property;
  return senscord::Status::OK();
}

/**
 * @brief Parse parameter.
 */
void V4L2ImageStreamSource::ParseParameter() {
  senscord::Status status;

  {
    // device name
    std::string value;
    status = util_->GetInstanceArgument("device", &value);
    if (status.ok()) {
      settings_.device = value;
    }

    // pixel_format
    status = util_->GetInstanceArgument("pixel_format", &value);
    if (status.ok()) {
      image_property_.pixel_format = value;
    }

    // yuyv_to_nv16
    status = util_->GetInstanceArgument("yuyv_to_nv16", &value);
    if (status.ok()) {
      is_yuyv_to_nv16_ = (value == "true");
    }
  }

  {
    // buffer num
    uint64_t num = 0;
    status = util_->GetInstanceArgument("buffer_num", &num);
    if (status.ok()) {
      settings_.buffer_num = static_cast<uint32_t>(num);
    }

    // width
    status = util_->GetInstanceArgument("width", &num);
    if (status.ok()) {
      image_property_.width = static_cast<uint32_t>(num);
      image_property_.stride_bytes = static_cast<uint32_t>(num);
    }

    // height
    status = util_->GetInstanceArgument("height", &num);
    if (status.ok()) {
      image_property_.height = static_cast<uint32_t>(num);
    }

    // stride_bytes
    status = util_->GetInstanceArgument("stride_bytes", &num);
    if (status.ok()) {
      image_property_.stride_bytes = static_cast<uint32_t>(num);
    }

    // framerate
    status = util_->GetInstanceArgument("framerate", &num);
    if (status.ok()) {
      framerate_property_.num = static_cast<uint32_t>(num);
      framerate_property_.denom = 1;
    }
  }
}

/**
 * @brief Get used buffer number.
 * @return used buffer number.
 */
uint32_t V4L2ImageStreamSource::GetUsedBufferNum() const {
  uint32_t cnt = 0;
  LockGuard autolock(buffer_mutex_);
  for (std::vector<BufferInfo>::const_iterator itr = buffer_list_.begin();
      itr != buffer_list_.end(); ++itr) {
    if (itr->used) {
      ++cnt;
    }
  }
  return cnt;
}
