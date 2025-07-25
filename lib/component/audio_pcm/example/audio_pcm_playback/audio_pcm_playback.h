/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SAMPLE_CPP_AUDIO_PCM_PLAYBACK_AUDIO_PCM_PLAYBACK_H_
#define SAMPLE_CPP_AUDIO_PCM_PLAYBACK_AUDIO_PCM_PLAYBACK_H_

#include <stdint.h>

#include <string>

#include "senscord/senscord.h"
#include "senscord/property_types.h"

/**
 * @brief Get format name from `AudioPcm::Format`.
 */
const char* GetFormatName(senscord::AudioPcm::Format format);

/**
 * @brief Get `AudioPcm::Format` from format name.
 */
senscord::AudioPcm::Format GetFormat(const std::string& name);

/**
 * @brief Get format name list.
 */
std::string GetFormatNameList();

/**
 * @brief Audio playback interface.
 */
class AudioPcmPlayback {
 public:
  virtual ~AudioPcmPlayback() {}

  /**
   * @brief Open the device.
   * @param[in] (device_name) Device name.
   * @return Status object.
   */
  virtual senscord::Status Open(const std::string& device_name) = 0;

  /**
   * @brief Close the device.
   * @return Status object.
   */
  virtual senscord::Status Close() = 0;

  /**
   * @brief Set the parameters.
   * @param[in] (params) Parameters to set.
   * @return Status object.
   */
  virtual senscord::Status SetParams(
      const senscord::AudioPcmProperty& params) = 0;

  /**
   * @brief Get the parameters.
   * @param[out] (params) Parameters.
   * @return Status object.
   */
  virtual senscord::Status GetParams(
      senscord::AudioPcmProperty* params) = 0;

  /**
   * @brief Start capturing.
   * @return Status object.
   */
  virtual senscord::Status Start() = 0;

  /**
   * @brief Stop capturing.
   * @return Status object.
   */
  virtual senscord::Status Stop() = 0;

  /**
   * @brief Write the PCM frame data.
   * @param[in] (frame) Frame to write.
   * @return Status object.
   */
  virtual senscord::Status Write(senscord::Frame* frame) = 0;
};

#endif  // SAMPLE_CPP_AUDIO_PCM_PLAYBACK_AUDIO_PCM_PLAYBACK_H_
