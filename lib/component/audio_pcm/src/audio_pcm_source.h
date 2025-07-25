/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef COMPONENT_AUDIO_PCM_SRC_AUDIO_PCM_SOURCE_H_
#define COMPONENT_AUDIO_PCM_SRC_AUDIO_PCM_SOURCE_H_

#include <string>
#include <vector>

#include "senscord/develop/stream_source.h"

#include "src/audio_capture.h"
#include "src/memory_pool.h"

/**
 * @brief The audio stream source.
 */
class AudioPcmSource : public senscord::AudioStreamSource {
 public:
  /**
   * @brief Constructor
   */
  AudioPcmSource();

  /**
   * @brief Destructor
   */
  ~AudioPcmSource();

  /**
   * @brief Open the stream source.
   * @param[in] (core) The core instance.
   * @param[in] (util) The utility accessor to core.
   * @return The status of function.
   */
  virtual senscord::Status Open(
      senscord::Core* core,
      senscord::StreamSourceUtility* util);

  /**
   * @brief Close the stream source.
   * @return The status of function.
   */
  virtual senscord::Status Close();

  /**
   * @brief Start the stream source.
   * @return The status of function.
   */
  virtual senscord::Status Start();

  /**
   * @brief Stop the stream source.
   * @return The status of function.
   */
  virtual senscord::Status Stop();

  /**
   * @brief Pull up the new frames.
   * @param[out] (frames) The information about new frames.
   */
  virtual void GetFrames(std::vector<senscord::FrameInfo>* frames);

  /**
   * @brief Release the used frame.
   * @param[in] (frameinfo) The information about used frame.
   * @param[in] (referenced_channel_ids) List of referenced channel IDs.
   *                                     (NULL is the same as empty)
   * @return The status of function.
   */
  virtual senscord::Status ReleaseFrame(
      const senscord::FrameInfo& frameinfo,
      const std::vector<uint32_t>* referenced_channel_ids);

  /// Mandatory properties.

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Get(
      const std::string& key, senscord::ChannelInfoProperty* property);

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Get(
      const std::string& key, senscord::FrameRateProperty* property);

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Set(
      const std::string& key, const senscord::FrameRateProperty* property);

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Get(
      const std::string& key, senscord::AudioProperty* property);

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Set(
      const std::string& key, const senscord::AudioProperty* property);

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Get(
      const std::string& key, senscord::SamplingFrequencyProperty* property);

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Set(
      const std::string& key,
      const senscord::SamplingFrequencyProperty* property);

  /// Unique properties.

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  senscord::Status Get(
      const std::string& key, senscord::AudioPcmProperty* property);

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  senscord::Status Set(
      const std::string& key, const senscord::AudioPcmProperty* property);

 private:
  /**
   * @brief Parse parameter.
   */
  void ParseArgument();

  /**
   * @brief Returns true if the stream is running.
   */
  bool IsRunning() const;

  /**
   * @brief Updates the channel property.
   */
  void UpdateChannelProperty();

 private:
  senscord::StreamSourceUtility* util_;
  MemoryPool* memory_pool_;

  // instance argument
  /** Capture device name */
  std::string device_name_;
  /** Buffer period (unit: milliseconds) */
  uint32_t buffer_period_;

  AudioCapture* capture_;

  uint64_t frame_seq_num_;
  bool running_;

  senscord::AudioProperty audio_property_;
  senscord::AudioPcmProperty pcm_property_;
};

#endif  // COMPONENT_AUDIO_PCM_SRC_AUDIO_PCM_SOURCE_H_
