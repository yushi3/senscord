/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef COMPONENT_AUDIO_SRC_AUDIO_CAPTURE_H_
#define COMPONENT_AUDIO_SRC_AUDIO_CAPTURE_H_

#include <stdint.h>

#include <string>

#include "senscord/status.h"
#include "senscord/memory.h"
#include "senscord/property_types.h"

/**
 * @brief Audio capture interface.
 */
class AudioCapture {
 public:
  virtual ~AudioCapture() {}

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
   * @brief Read the capture data.
   * @param[in,out] (memory) Memory of storage destination.
   * @param[in] (sample_count) Number of samples to read.
   * @param[out] (timestamp) Timestamp.
   * @return Status object.
   */
  virtual senscord::Status Read(
      senscord::Memory* memory, uint32_t sample_count,
      uint64_t* timestamp) = 0;
};

#endif  // COMPONENT_AUDIO_SRC_AUDIO_CAPTURE_H_
