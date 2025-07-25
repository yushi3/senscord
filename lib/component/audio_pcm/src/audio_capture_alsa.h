/*
 * SPDX-FileCopyrightText: 2023-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef COMPONENT_AUDIO_SRC_AUDIO_CAPTURE_ALSA_H_
#define COMPONENT_AUDIO_SRC_AUDIO_CAPTURE_ALSA_H_

#include <string>

#include "src/audio_capture.h"

#include "senscord/osal.h"

/**
 * @brief Audio capture using ALSA.
 */
class AudioCaptureAlsa : public AudioCapture {
 public:
  /**
   * @brief Constructor.
   */
  AudioCaptureAlsa();

  /**
   * @brief Destructor.
   */
  virtual ~AudioCaptureAlsa();

  /**
   * @brief Open the device.
   * @param[in] (device_name) Device name.
   * @return Status object.
   */
  virtual senscord::Status Open(const std::string& device_name);

  /**
   * @brief Close the device.
   * @return Status object.
   */
  virtual senscord::Status Close();

  /**
   * @brief Set the parameters.
   * @param[in] (params) Parameters to set.
   * @return Status object.
   */
  virtual senscord::Status SetParams(
      const senscord::AudioPcmProperty& params);

  /**
   * @brief Get the parameters.
   * @param[out] (params) Parameters.
   * @return Status object.
   */
  virtual senscord::Status GetParams(
      senscord::AudioPcmProperty* params);

  /**
   * @brief Start capturing.
   * @return Status object.
   */
  virtual senscord::Status Start();

  /**
   * @brief Stop capturing.
   * @return Status object.
   */
  virtual senscord::Status Stop();

  /**
   * @brief Read the capture data.
   * @param[in,out] (memory) Memory of storage destination.
   * @param[in] (sample_count) Number of samples to read.
   * @param[out] (timestamp) Timestamp.
   * @return Status object.
   */
  virtual senscord::Status Read(
      senscord::Memory* memory, uint32_t sample_count,
      uint64_t* timestamp);

 private:
  struct Impl;
  Impl* pimpl_;

  senscord::osal::OSMutex* mutex_state_;
};

#endif  // COMPONENT_AUDIO_SRC_AUDIO_CAPTURE_ALSA_H_
