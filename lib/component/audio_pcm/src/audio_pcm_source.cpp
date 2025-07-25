/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/audio_pcm_source.h"

#include <inttypes.h>

#include <string>
#include <vector>

#include "senscord/senscord.h"
#include "senscord/property_types.h"
#include "senscord/osal.h"
#include "senscord/logger.h"

#include "src/audio_capture_alsa.h"

namespace {

// stream source name.
const char kBlockName[] = "audio";
// Default device name.
const char kDefaultDeviceName[] = "default";
// Default frame rate (fps).
const uint32_t kDefaultFrameRate = 10;  // 10 fps.
// Default buffer period (unit: milliseconds).
const uint32_t kDefaultBufferPeriod = 2000;  // 2 second buffer.

/**
 * @brief Greatest Common Divisor (Euclidean Algorithm)
 */
template<typename T>
T CalcGcd(T a, T b) {
  if (a < b) {
    T tmp = a;
    a = b;
    b = tmp;
  }
  T remainder = a % b;
  while (remainder != 0) {
    a = b;
    b = remainder;
    remainder = a % b;
  }
  return b;
}

}  // namespace

/**
 * @brief Constructor
 */
AudioPcmSource::AudioPcmSource()
    : util_(), memory_pool_(),
      device_name_(kDefaultDeviceName),
      buffer_period_(kDefaultBufferPeriod),
      capture_(),
      frame_seq_num_(),
      running_(),
      audio_property_(),
      pcm_property_() {
  audio_property_.format = senscord::kAudioFormatLinearPcm;
}

/**
 * @brief Destructor
 */
AudioPcmSource::~AudioPcmSource() {
}

/**
 * @brief Open the stream source.
 * @param[in] (core) The core instance.
 * @param[in] (util) The utility accessor to core.
 * @return The status of function.
 */
senscord::Status AudioPcmSource::Open(
    senscord::Core* core,
    senscord::StreamSourceUtility* util) {
  senscord::Status status;
  util_ = util;

  // Create memory pool.
  senscord::MemoryAllocator* allocator = NULL;
  status = util_->GetAllocator(
      senscord::kAllocatorNameDefault, &allocator);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  memory_pool_ = new MemoryPool(allocator);

  // Register properties.
  SENSCORD_REGISTER_PROPERTY(
      util_, senscord::kAudioPcmPropertyKey, senscord::AudioPcmProperty);

  // Parse arguments.
  ParseArgument();

  // Open capture device.
  capture_ = new AudioCaptureAlsa;
  status = capture_->Open(device_name_);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  capture_->GetParams(&pcm_property_);
  pcm_property_.samples_per_frame =
      pcm_property_.samples_per_second / kDefaultFrameRate;

  frame_seq_num_ = 0;

  return status;
}

/**
 * @brief Close the stream source.
 * @return The status of function.
 */
senscord::Status AudioPcmSource::Close() {
  if (capture_ != NULL) {
    capture_->Close();
    delete capture_;
    capture_ = NULL;
  }

  delete memory_pool_;
  memory_pool_ = NULL;

  return senscord::Status::OK();
}

/**
 * @brief Start the stream source.
 * @return The status of function.
 */
senscord::Status AudioPcmSource::Start() {
  senscord::Status status;

  status = capture_->SetParams(pcm_property_);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  capture_->GetParams(&pcm_property_);

  UpdateChannelProperty();

  const double frame_rate =
      static_cast<double>(pcm_property_.samples_per_second) /
      pcm_property_.samples_per_frame;
  SENSCORD_LOG_INFO_TAGGED(kBlockName, "frame_rate=%f", frame_rate);
  const uint32_t buffer_num = ((frame_rate * buffer_period_) + 999) / 1000;
  const uint32_t frame_size =
      pcm_property_.samples_per_frame * pcm_property_.channels *
      senscord::AudioPcm::GetByteWidth(pcm_property_.format);

  if ((memory_pool_->GetBufferNum() != buffer_num) ||
      (memory_pool_->GetBufferSize() != frame_size)) {
    memory_pool_->Exit();
    status = memory_pool_->Init(buffer_num, frame_size);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
  }

  status = capture_->Start();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  running_ = true;

  return status;
}

/**
 * @brief Stop the stream source.
 * @return The status of function.
 */
senscord::Status AudioPcmSource::Stop() {
  senscord::Status status;

  status = capture_->Stop();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  running_ = false;

  return status;
}

/**
 * @brief Pull up the new frames.
 * @param[out] (frames) The information about new frames.
 */
void AudioPcmSource::GetFrames(std::vector<senscord::FrameInfo>* frames) {
  const uint64_t seq_num = frame_seq_num_;
  frame_seq_num_ += 1;

  senscord::Memory* memory = memory_pool_->GetMemory();
  if (memory == NULL) {
    util_->SendEventFrameDropped(seq_num);
    const double frame_rate =
        static_cast<double>(pcm_property_.samples_per_second) /
        pcm_property_.samples_per_frame;
    senscord::osal::OSSleep(static_cast<uint64_t>(1000000000ULL / frame_rate));
    return;
  }

  // Read PCM data.
  uint64_t current_time = 0;
  senscord::Status status = capture_->Read(
      memory, pcm_property_.samples_per_frame, &current_time);
  if (!status.ok()) {
    util_->SendEventFrameDropped(seq_num);
    memory_pool_->ReleaseMemory(memory);
    // TODO: reset capture device
    return;
  }

  const uint32_t channel_count = pcm_property_.interleaved ?
      1 : pcm_property_.channels;
  const size_t data_size = pcm_property_.interleaved ?
      memory->GetSize() : (memory->GetSize() / pcm_property_.channels);

  // Setup FrameInfo.
  senscord::FrameInfo frameinfo = {};
  frameinfo.sequence_number = seq_num;

  for (uint32_t id = 0; id < channel_count; ++id) {
    senscord::ChannelRawData channel = {};
    channel.channel_id = id;
    channel.data_type = senscord::kRawDataTypeAudio;
    channel.data_memory = memory;
    channel.data_size = data_size;
    channel.data_offset = data_size * id;
    channel.captured_timestamp = current_time;
    frameinfo.channels.push_back(channel);
    SENSCORD_LOG_DEBUG_TAGGED(
        kBlockName, "channel[%" PRIu32 "]: data_size=%" PRIuS
        ", data_offset=%" PRIuS,
        channel.channel_id , channel.data_size, channel.data_offset);
  }

  frames->push_back(frameinfo);

  SENSCORD_LOG_DEBUG_TAGGED(
      kBlockName, "seq_num=%" PRIu64 ", timestamp=%" PRIu64 ".%09" PRIu32,
      seq_num, current_time / 1000000000, current_time % 1000000000);
}

/**
 * @brief Release the used frame.
 * @param[in] (frameinfo) The information about used frame.
 * @param[in] (referenced_channel_ids) List of referenced channel IDs.
 *                                     (NULL is the same as empty)
 * @return The status of function.
 */
senscord::Status AudioPcmSource::ReleaseFrame(
    const senscord::FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids) {
  if (!frameinfo.channels.empty()) {
    memory_pool_->ReleaseMemory(frameinfo.channels[0].data_memory);
  }
  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status AudioPcmSource::Get(
    const std::string& key, senscord::ChannelInfoProperty* property) {
  property->channels.clear();
  if (pcm_property_.interleaved) {
    // interleaved
    senscord::ChannelInfo info = {};
    info.raw_data_type = senscord::kRawDataTypeAudio;
    info.description = "Audio data (interleaved)";
    property->channels[0] = info;
  } else {
    // non-interleaved
    senscord::ChannelInfo info = {};
    info.raw_data_type = senscord::kRawDataTypeAudio;
    info.description = "Audio data";
    const uint32_t channel_count = pcm_property_.channels;
    for (uint32_t id = 0; id < channel_count; ++id) {
      property->channels[id] = info;
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
senscord::Status AudioPcmSource::Get(
    const std::string& key, senscord::FrameRateProperty* property) {
  const uint32_t gcd = CalcGcd(
      pcm_property_.samples_per_second, pcm_property_.samples_per_frame);
  property->num = pcm_property_.samples_per_second / gcd;
  property->denom = pcm_property_.samples_per_frame / gcd;
  SENSCORD_LOG_INFO_TAGGED(
      kBlockName, "num=%" PRIu32 ", denom=%" PRIu32 " (%f)",
      property->num, property->denom,
      static_cast<float>(property->num) / property->denom);
  return senscord::Status::OK();
}

/**
 * @brief Set the new stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status AudioPcmSource::Set(
    const std::string& key, const senscord::FrameRateProperty* property) {
  return SENSCORD_STATUS_FAIL(
      kBlockName, senscord::Status::kCauseNotSupported,
      "not supported (use AudioPcmProperty)");
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status AudioPcmSource::Get(
    const std::string& key, senscord::AudioProperty* property) {
  *property = audio_property_;
  return senscord::Status::OK();
}

/**
 * @brief Set the new stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status AudioPcmSource::Set(
    const std::string& key, const senscord::AudioProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidArgument,
        "Null pointer");
  }
  if (IsRunning()) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidOperation,
        "Cannot set because stream is running.");
  }
  if (property->format != senscord::kAudioFormatLinearPcm) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseNotSupported,
        "Unsupported audio format.");
  }
  return senscord::Status::OK();
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status AudioPcmSource::Get(
    const std::string& key, senscord::SamplingFrequencyProperty* property) {
  property->value = static_cast<float>(pcm_property_.samples_per_second);
  return senscord::Status::OK();
}

/**
 * @brief Set the new stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status AudioPcmSource::Set(
    const std::string& key,
    const senscord::SamplingFrequencyProperty* property) {
  return SENSCORD_STATUS_FAIL(
      kBlockName, senscord::Status::kCauseNotSupported,
      "not supported (use AudioPcmProperty.samples_per_second)");
}

/**
 * @brief Get the stream source property.
 * @param[in] (key) The key of property.
 * @param[out] (property) The location of property.
 * @return The status of function.
 */
senscord::Status AudioPcmSource::Get(
    const std::string& key, senscord::AudioPcmProperty* property) {
  *property = pcm_property_;
  return senscord::Status::OK();
}

/**
 * @brief Set the new stream source property.
 * @param[in] (key) The key of property.
 * @param[in] (property) The location of property.
 * @return The status of function.
 */
senscord::Status AudioPcmSource::Set(
    const std::string& key, const senscord::AudioPcmProperty* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidArgument,
        "Null pointer");
  }
  if (IsRunning()) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidOperation,
        "Cannot set because stream is running.");
  }
  senscord::Status status = capture_->SetParams(*property);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  capture_->GetParams(&pcm_property_);
  return senscord::Status::OK();
}

/**
 * @brief Parse parameter.
 */
void AudioPcmSource::ParseArgument() {
  senscord::Status status;

  // "device_name"
  {
    std::string value;
    status = util_->GetInstanceArgument("device_name", &value);
    if (status.ok()) {
      device_name_ = value;
    }
  }
  // "buffer_period"
  {
    uint64_t value = 0;
    status = util_->GetInstanceArgument("buffer_period", &value);
    if (status.ok()) {
      buffer_period_ = static_cast<uint32_t>(value);
    }
  }
}

/**
 * @brief Returns true if the stream is running.
 */
bool AudioPcmSource::IsRunning() const {
  return running_;
}

/**
 * @brief Updates the channel property.
 */
void AudioPcmSource::UpdateChannelProperty() {
  const uint32_t channel_count =
      pcm_property_.interleaved ? 1 : pcm_property_.channels;
  for (uint32_t id = 0; id < channel_count; ++id) {
    // AudioProperty
    util_->UpdateChannelProperty(
        id, senscord::kAudioPropertyKey, &audio_property_);
    // AudioPcmProperty
    util_->UpdateChannelProperty(
        id, senscord::kAudioPcmPropertyKey, &pcm_property_);
  }
}
