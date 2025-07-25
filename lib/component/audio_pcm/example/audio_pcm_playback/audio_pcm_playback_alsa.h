/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SAMPLE_CPP_AUDIO_PCM_PLAYBACK_AUDIO_PCM_PLAYBACK_ALSA_H_
#define SAMPLE_CPP_AUDIO_PCM_PLAYBACK_AUDIO_PCM_PLAYBACK_ALSA_H_

#include <string>

#include "audio_pcm_playback.h"

/**
 * @brief Audio playback using ALSA.
 */
class AudioPcmPlaybackAlsa : public AudioPcmPlayback {
 public:
  /**
   * @brief Constructor.
   */
  AudioPcmPlaybackAlsa();

  /**
   * @brief Destructor.
   */
  virtual ~AudioPcmPlaybackAlsa();

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
   * @brief Write the PCM frame data.
   * @param[in] (frame) Frame to write.
   * @return Status object.
   */
  virtual senscord::Status Write(senscord::Frame* frame);

 private:
  struct Impl;
  Impl* pimpl_;
};

#endif  // SAMPLE_CPP_AUDIO_PCM_PLAYBACK_AUDIO_PCM_PLAYBACK_ALSA_H_
