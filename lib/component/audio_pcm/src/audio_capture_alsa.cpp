/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/audio_capture_alsa.h"

#include <inttypes.h>
#include <alsa/asoundlib.h>

#include <string>
#include <vector>

#include "senscord/osal.h"
#include "senscord/logger.h"

namespace {

const char kBlockName[] = "audio";

const uint32_t kDefaultChannels = 2;
const bool kDefaultInterleaved = true;
const senscord::AudioPcm::Format kDefaultFormat =
    senscord::AudioPcm::kFormatS16LE;
const uint32_t kDefaultSampleRate = 44100;

struct FormatInfo {
  senscord::AudioPcm::Format format;
  snd_pcm_format_t alsa_format;
};

const FormatInfo kFormatTable[] = {
    // 8bit
    { senscord::AudioPcm::kFormatS8, SND_PCM_FORMAT_S8 },
    { senscord::AudioPcm::kFormatU8, SND_PCM_FORMAT_U8 },
    // 16bit
    { senscord::AudioPcm::kFormatS16LE, SND_PCM_FORMAT_S16_LE },
    { senscord::AudioPcm::kFormatS16BE, SND_PCM_FORMAT_S16_BE },
    { senscord::AudioPcm::kFormatU16LE, SND_PCM_FORMAT_U16_LE },
    { senscord::AudioPcm::kFormatU16BE, SND_PCM_FORMAT_U16_BE },
    // 24bit
    { senscord::AudioPcm::kFormatS24LE3, SND_PCM_FORMAT_S24_3LE },
    { senscord::AudioPcm::kFormatS24BE3, SND_PCM_FORMAT_S24_3BE },
    { senscord::AudioPcm::kFormatU24LE3, SND_PCM_FORMAT_U24_3LE },
    { senscord::AudioPcm::kFormatU24BE3, SND_PCM_FORMAT_U24_3BE },
    // 24bit (using lower 3 bytes out of 4 bytes)
    { senscord::AudioPcm::kFormatS24LE, SND_PCM_FORMAT_S24_LE },
    { senscord::AudioPcm::kFormatS24BE, SND_PCM_FORMAT_S24_BE },
    { senscord::AudioPcm::kFormatU24LE, SND_PCM_FORMAT_U24_LE },
    { senscord::AudioPcm::kFormatU24BE, SND_PCM_FORMAT_U24_BE },
    // 32bit
    { senscord::AudioPcm::kFormatS32LE, SND_PCM_FORMAT_S32_LE },
    { senscord::AudioPcm::kFormatS32BE, SND_PCM_FORMAT_S32_BE },
    { senscord::AudioPcm::kFormatU32LE, SND_PCM_FORMAT_U32_LE },
    { senscord::AudioPcm::kFormatU32BE, SND_PCM_FORMAT_U32_BE },
    // 32bit float
    { senscord::AudioPcm::kFormatFloat32LE, SND_PCM_FORMAT_FLOAT_LE },
    { senscord::AudioPcm::kFormatFloat32BE, SND_PCM_FORMAT_FLOAT_BE },
    // 64bit float
    { senscord::AudioPcm::kFormatFloat64LE, SND_PCM_FORMAT_FLOAT64_LE },
    { senscord::AudioPcm::kFormatFloat64BE, SND_PCM_FORMAT_FLOAT64_BE },
};

/**
 * @brief Get ALSA format from `AudioPcm::Format`.
 */
snd_pcm_format_t GetAlsaFormat(senscord::AudioPcm::Format format) {
  size_t count = sizeof(kFormatTable) / sizeof(kFormatTable[0]);
  for (size_t i = 0; i < count; ++i) {
    if (kFormatTable[i].format == format) {
      return kFormatTable[i].alsa_format;
    }
  }
  return SND_PCM_FORMAT_UNKNOWN;
}

}  // namespace

struct AudioCaptureAlsa::Impl {
  snd_pcm_t* capture_handle;
  senscord::AudioPcmProperty params;
  bool running;
};

/**
 * @brief Constructor.
 */
AudioCaptureAlsa::AudioCaptureAlsa() : pimpl_(new Impl), mutex_state_(NULL) {
  pimpl_->params.channels = kDefaultChannels;
  pimpl_->params.interleaved = kDefaultInterleaved;
  pimpl_->params.format = kDefaultFormat;
  pimpl_->params.samples_per_second = kDefaultSampleRate;
  pimpl_->params.samples_per_frame = 0;
  pimpl_->running = false;

  senscord::osal::OSCreateMutex(&mutex_state_);
}

/**
 * @brief Destructor.
 */
AudioCaptureAlsa::~AudioCaptureAlsa() {
  delete pimpl_;
  senscord::osal::OSDestroyMutex(mutex_state_);
}

/**
 * @brief Open the device.
 * @param[in] (device_name) Device name.
 * @return Status object.
 */
senscord::Status AudioCaptureAlsa::Open(const std::string& device_name) {
  int32_t err = 0;
  err = snd_pcm_open(
      &pimpl_->capture_handle, device_name.c_str(), SND_PCM_STREAM_CAPTURE, 0);
  if (err < 0) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseHardwareError,
        "Failed to snd_pcm_open: device_name=%s (%s)",
        device_name.c_str(), snd_strerror(err));
  }
  return senscord::Status::OK();
}

/**
 * @brief Close the device.
 * @return Status object.
 */
senscord::Status AudioCaptureAlsa::Close() {
  if (pimpl_->capture_handle != NULL) {
    snd_pcm_close(pimpl_->capture_handle);
    pimpl_->capture_handle = NULL;
  }
  pimpl_->running = false;
  return senscord::Status::OK();
}

/**
 * @brief Set the parameters.
 * @param[in] (params) Parameters to set.
 * @return Status object.
 */
senscord::Status AudioCaptureAlsa::SetParams(
    const senscord::AudioPcmProperty& params) {
  senscord::Status status;
  snd_pcm_hw_params_t* hw_params = NULL;

  {
    int32_t err = snd_pcm_hw_params_malloc(&hw_params);
    if (err < 0) {
      status = SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseHardwareError,
          "Failed to snd_pcm_hw_params_malloc (%s)",
          snd_strerror(err));
    }
  }

  if (status.ok()) {
    int32_t err = snd_pcm_hw_params_any(pimpl_->capture_handle, hw_params);
    if (err < 0) {
      status = SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseHardwareError,
          "Failed to snd_pcm_hw_params_any (%s)",
          snd_strerror(err));
    }
  }

  // set access type (interleaved / non-interleaved)
  if (status.ok()) {
    snd_pcm_access_t accesss = params.interleaved ?
        SND_PCM_ACCESS_RW_INTERLEAVED : SND_PCM_ACCESS_RW_NONINTERLEAVED;
    int32_t err = snd_pcm_hw_params_set_access(
        pimpl_->capture_handle, hw_params, accesss);
    if (err < 0) {
      status = SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseHardwareError,
          "Failed to snd_pcm_hw_params_set_access (%s)",
          snd_strerror(err));
    }
  }

  // set sample format
  if (status.ok()) {
    snd_pcm_format_t format = GetAlsaFormat(params.format);
    int32_t err = snd_pcm_hw_params_set_format(
        pimpl_->capture_handle, hw_params, format);
    if (err < 0) {
      status = SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseHardwareError,
          "Failed to snd_pcm_hw_params_set_format: format=%d (%s)",
          format, snd_strerror(err));
    }
  }

  // set sample rate
  uint32_t rate = params.samples_per_second;
  if (status.ok()) {
    int32_t err = snd_pcm_hw_params_set_rate_near(
        pimpl_->capture_handle, hw_params, &rate, NULL);
    if (err < 0) {
      status = SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseHardwareError,
          "Failed to snd_pcm_hw_params_set_rate_near: rate=%" PRIu32 " (%s)",
          rate, snd_strerror(err));
    }
  }

  // set channel count
  if (status.ok()) {
    int32_t err = snd_pcm_hw_params_set_channels(
        pimpl_->capture_handle, hw_params, params.channels);
    if (err < 0) {
      status = SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseHardwareError,
          "Failed to snd_pcm_hw_params_set_channels: channels=%" PRIu8 " (%s)",
          params.channels, snd_strerror(err));
    }
  }

  // TODO: set period size, buffer size

  if (status.ok()) {
    int32_t err = snd_pcm_hw_params(pimpl_->capture_handle, hw_params);
    if (err < 0) {
      status = SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseHardwareError,
          "Failed to snd_pcm_hw_params (%s)",
          snd_strerror(err));
    }
  }

  if (hw_params != NULL) {
    snd_pcm_hw_params_free(hw_params);
  }

  if (status.ok()) {
    pimpl_->params = params;
    pimpl_->params.samples_per_second = rate;
  }

  return status;
}

/**
 * @brief Get the parameters.
 * @param[out] (params) Parameters.
 * @return Status object.
 */
senscord::Status AudioCaptureAlsa::GetParams(
    senscord::AudioPcmProperty* params) {
  *params = pimpl_->params;
  return senscord::Status::OK();
}

/**
 * @brief Start capturing.
 * @return Status object.
 */
senscord::Status AudioCaptureAlsa::Start() {
  senscord::osal::OSLockMutex(mutex_state_);
  if (!pimpl_->running) {
    int32_t err = snd_pcm_start(pimpl_->capture_handle);
    if (err < 0) {
      senscord::osal::OSUnlockMutex(mutex_state_);
      return SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseHardwareError,
          "Failed to snd_pcm_start (%s)",
          snd_strerror(err));
    }
    pimpl_->running = true;
  }
  senscord::osal::OSUnlockMutex(mutex_state_);
  return senscord::Status::OK();
}

/**
 * @brief Stop capturing.
 * @return Status object.
 */
senscord::Status AudioCaptureAlsa::Stop() {
  senscord::osal::OSLockMutex(mutex_state_);
  if (pimpl_->running) {
    int32_t err = snd_pcm_drop(pimpl_->capture_handle);
    if (err < 0) {
      senscord::osal::OSUnlockMutex(mutex_state_);
      return SENSCORD_STATUS_FAIL(
          kBlockName, senscord::Status::kCauseHardwareError,
          "Failed to snd_pcm_drain (%s)",
          snd_strerror(err));
    }
    pimpl_->running = false;
  }
  senscord::osal::OSUnlockMutex(mutex_state_);
  return senscord::Status::OK();
}

/**
 * @brief Read the capture data.
 * @param[in,out] (memory) Memory of storage destination.
 * @param[in] (sample_count) Number of samples to read.
 * @param[out] (timestamp) Timestamp.
 * @return Status object.
 */
senscord::Status AudioCaptureAlsa::Read(
    senscord::Memory* memory, uint32_t sample_count,
    uint64_t* timestamp) {
  senscord::osal::OSLockMutex(mutex_state_);
  if (!pimpl_->running) {
    senscord::osal::OSUnlockMutex(mutex_state_);
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidOperation,
        "Already stopped.");
  }

  // TODO: snd_pcm_status_get_htstamp
  senscord::osal::OSGetTime(timestamp);

  snd_pcm_sframes_t read_count = 0;
  uint8_t* ptr = reinterpret_cast<uint8_t*>(memory->GetAddress());
  if (pimpl_->params.interleaved) {
    read_count = snd_pcm_readi(pimpl_->capture_handle, ptr, sample_count);
  } else {
    const uint32_t channel_count = pimpl_->params.channels;
    const uint32_t channel_size =
        sample_count * senscord::AudioPcm::GetByteWidth(pimpl_->params.format);
    std::vector<void*> buf;
    for (uint32_t i = 0; i < channel_count; ++i) {
      buf.push_back(&ptr[channel_size * i]);
    }
    read_count = snd_pcm_readn(pimpl_->capture_handle, &buf[0], sample_count);
  }
  senscord::osal::OSUnlockMutex(mutex_state_);

  if (read_count != sample_count) {
    return SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseHardwareError,
        "Failed to snd_pcm_read: interleaved=%d, expected=%" PRIu32
        ", read=%ld",
        pimpl_->params.interleaved, sample_count, read_count);
  }
  return senscord::Status::OK();
}
