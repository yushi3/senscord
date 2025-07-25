/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_STREAM_SOURCE_H_
#define SENSCORD_DEVELOP_STREAM_SOURCE_H_

#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/senscord.h"
#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/property_types.h"
#include "senscord/develop/common_types.h"
#include "senscord/develop/stream_source_utility.h"

namespace senscord {

/**
 * @brief The abstruct class for the stream source.
 */
class StreamSource : private util::Noncopyable {
 public:
  /**
   * @brief Pull up the new frames.
   * @param[out] (frames) The information about new frames.
   */
  virtual void GetFrames(std::vector<FrameInfo>* frames) = 0;

  /**
   * @brief Release the used frame.
   * @param[in] (frameinfo) The information about used frame.
   * @param[in] (referenced_channel_ids) List of referenced channel IDs.
   *                                     (NULL is the same as empty)
   * @return The status of function.
   */
  virtual Status ReleaseFrame(
    const FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids) = 0;

  /**
   * @brief Open the stream source.
   * @param[in] (core) The core instance.
   * @param[in] (util) The utility accessor to core.
   * @return The status of function.
   */
  virtual Status Open(Core* core, StreamSourceUtility* util) {
    // Please overwrite this function.
    return Open(util);
  }

  /**
   * @deprecated
   * @brief Open the stream source.
   * @param[in] (util) The utility accessor to core.
   * @return The status of function.
   */
  virtual Status Open(StreamSourceUtility* util) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseNotSupported,
        "StreamSource::Open is not implemented.");
  }

  /**
   * @brief Close the stream source.
   * @return The status of function.
   */
  virtual Status Close() = 0;

  /**
   * @brief Start the stream source.
   * @return The status of function.
   */
  virtual Status Start() {
    return Status::OK();
  }

  /**
   * @brief Stop the stream source.
   * @return The status of function.
   */
  virtual Status Stop() {
    return Status::OK();
  }

  /**
   * @brief Hook for cacth the result of frame sending.
   * @param[in] (result) Status of SendFrame() result.
   */
  virtual void CatchErrorSendingFrame(const Status& result) {}

  /**
   * @brief Register the mandatory properties.
   * @param[in] (util) The utility accessor to core.
   */
  virtual void RegisterMandatoryProperties(StreamSourceUtility* util) {
    SENSCORD_REGISTER_PROPERTY(
        util, kChannelInfoPropertyKey, ChannelInfoProperty);
    SENSCORD_REGISTER_PROPERTY(
        util, kFrameRatePropertyKey, FrameRateProperty);
  }

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Get(
    const std::string& key, ChannelInfoProperty* property) = 0;

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Set(
      const std::string& key, const ChannelInfoProperty* property) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation,
        "not available to set %s", key.c_str());
  }

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Get(
    const std::string& key, FrameRateProperty* property) = 0;

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Set(
      const std::string& key, const FrameRateProperty* property) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented to set %s", key.c_str());
  }

  /**
   * @brief Destructor
   */
  virtual ~StreamSource() {}
};

/**
 * @brief The abstruct class for the imaage stream source.
 */
class ImageStreamSource : public StreamSource {
 public:
  /**
   * @brief Register the mandatory properties.
   * @param[in] (util) The utility accessor to core.
   */
  virtual void RegisterMandatoryProperties(StreamSourceUtility* util) {
    StreamSource::RegisterMandatoryProperties(util);
    SENSCORD_REGISTER_PROPERTY(
        util, kImagePropertyKey, ImageProperty);
  }

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Get(
    const std::string& key, ImageProperty* property) = 0;

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Set(
      const std::string& key, const ImageProperty* property) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented to set %s", key.c_str());
  }

  /**
   * @brief Destructor
   */
  virtual ~ImageStreamSource() {}
};

/**
 * @brief The abstruct class for the depth stream source.
 */
class DepthStreamSource : public StreamSource {
 public:
  /**
   * @brief Register the mandatory properties.
   * @param[in] (util) The utility accessor to core.
   */
  virtual void RegisterMandatoryProperties(StreamSourceUtility* util) {
    StreamSource::RegisterMandatoryProperties(util);
    SENSCORD_REGISTER_PROPERTY(
        util, kImagePropertyKey, ImageProperty);
    SENSCORD_REGISTER_PROPERTY(
        util, kDepthPropertyKey, DepthProperty);
    SENSCORD_REGISTER_PROPERTY(
        util, kConfidencePropertyKey, ConfidenceProperty);
  }

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Get(
    const std::string& key, ImageProperty* property) = 0;

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Set(
      const std::string& key, const ImageProperty* property) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented to set %s", key.c_str());
  }

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Get(
    const std::string& key, DepthProperty* property) = 0;

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Set(
      const std::string& key, const DepthProperty* property) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented to set %s", key.c_str());
  }

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Get(
    const std::string& key, ConfidenceProperty* property) = 0;

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Set(
      const std::string& key, const ConfidenceProperty* property) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented to set %s", key.c_str());
  }

  /**
   * @brief Destructor
   */
  virtual ~DepthStreamSource() {}
};

/**
 * @brief The abstruct class for the IMU stream source.
 */
class ImuStreamSource : public StreamSource {
 public:
  /**
   * @brief Register the mandatory properties.
   * @param[in] (util) The utility accessor to core.
   */
  virtual void RegisterMandatoryProperties(StreamSourceUtility* util) {
    StreamSource::RegisterMandatoryProperties(util);
    SENSCORD_REGISTER_PROPERTY(
        util, kImuDataUnitPropertyKey, ImuDataUnitProperty);
    SENSCORD_REGISTER_PROPERTY(
        util, kSamplingFrequencyPropertyKey, SamplingFrequencyProperty);
  }

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Get(
    const std::string& key, ImuDataUnitProperty* property) = 0;

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Set(
      const std::string& key, const ImuDataUnitProperty* property) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation,
        "not available to set %s", key.c_str());
  }

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Get(
    const std::string& key, SamplingFrequencyProperty* property) = 0;

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Set(
      const std::string& key, const SamplingFrequencyProperty* property) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented to set %s", key.c_str());
  }

  /**
   * @brief Destructor
   */
  virtual ~ImuStreamSource() {}
};

/**
 * @brief The abstruct class for the SLAM stream source.
 */
class SlamStreamSource : public StreamSource {
 public:
  /**
   * @brief Register the mandatory properties.
   * @param[in] (util) The utility accessor to core.
   */
  virtual void RegisterMandatoryProperties(StreamSourceUtility* util) {
    StreamSource::RegisterMandatoryProperties(util);
    SENSCORD_REGISTER_PROPERTY(
        util, kSlamDataSupportedPropertyKey, SlamDataSupportedProperty);
  }

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Get(
    const std::string& key, SlamDataSupportedProperty* property) = 0;

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Set(
      const std::string& key, const SlamDataSupportedProperty* property) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation,
        "not available to set %s", key.c_str());
  }

  /**
   * @brief Destructor
   */
  virtual ~SlamStreamSource() {}
};

/**
 * @brief The abstruct class for the object detection stream source.
 */
class ObjectDetectionStreamSource : public StreamSource {
 public:
  /**
   * @brief Destructor
   */
  virtual ~ObjectDetectionStreamSource() {}
};

/**
 * @brief The abstruct class for the key point stream source.
 */
class KeyPointStreamSource : public StreamSource {
 public:
  /**
   * @brief Destructor
   */
  virtual ~KeyPointStreamSource() {}
};

/**
 * @brief The abstruct class for the TemporalContrast stream source.
 */
class TemporalContrastStreamSource : public StreamSource {
 public:
  /**
   * @brief Register the mandatory properties.
   * @param[in] (util) The utility accessor to core.
   */
  virtual void RegisterMandatoryProperties(StreamSourceUtility* util) {
    StreamSource::RegisterMandatoryProperties(util);
    SENSCORD_REGISTER_PROPERTY(
        util, kTemporalContrastDataPropertyKey, TemporalContrastDataProperty);
  }

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Get(
    const std::string& key, TemporalContrastDataProperty* property) = 0;

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Set(
      const std::string& key, const TemporalContrastDataProperty* property) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation,
        "not available to set %s", key.c_str());
  }

  /**
   * @brief Destructor
   */
  virtual ~TemporalContrastStreamSource() {}
};

/**
 * @brief The abstruct class for the PixelPolarity stream source.
 * @deprecated will be replaced by TemporalContrastStreamSource
 */
typedef TemporalContrastStreamSource PixelPolarityStreamSource;

/**
 * @brief The abstruct class for the ObjectTracking stream source.
 */
class ObjectTrackingStreamSource : public StreamSource {
 public:
  /**
   * @brief Register the mandatory properties.
   * @param[in] (util) The utility accessor to core.
   */
  virtual void RegisterMandatoryProperties(StreamSourceUtility* util) {
    StreamSource::RegisterMandatoryProperties(util);
    SENSCORD_REGISTER_PROPERTY(
        util, kVelocityDataUnitPropertyKey, VelocityDataUnitProperty);
  }

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Get(
    const std::string& key, VelocityDataUnitProperty* property) = 0;

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Set(
      const std::string& key, const VelocityDataUnitProperty* property) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidOperation,
        "not available to set %s", key.c_str());
  }

  /**
   * @brief Destructor
   */
  virtual ~ObjectTrackingStreamSource() {}
};

/**
 * @brief The abstract class for the audio stream source.
 */
class AudioStreamSource : public StreamSource {
 public:
  /**
   * @brief Register the mandatory properties.
   * @param[in] (util) The utility accessor to core.
   */
  virtual void RegisterMandatoryProperties(StreamSourceUtility* util) {
    StreamSource::RegisterMandatoryProperties(util);
    SENSCORD_REGISTER_PROPERTY(
        util, kAudioPropertyKey, AudioProperty);
    SENSCORD_REGISTER_PROPERTY(
        util, kSamplingFrequencyPropertyKey, SamplingFrequencyProperty);
  }

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Get(
      const std::string& key, AudioProperty* property) = 0;

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual Status Set(
      const std::string& key, const AudioProperty* property) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidOperation,
        "not available to set %s", key.c_str());
  }

  /**
   * @brief Get the stream source property.
   * @param[in] (key) The key of property.
   * @param[out] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Get(
      const std::string& key, SamplingFrequencyProperty* property) = 0;

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Set(
      const std::string& key, const SamplingFrequencyProperty* property) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented to set %s", key.c_str());
  }
};

}   // namespace senscord
#endif    // SENSCORD_DEVELOP_STREAM_SOURCE_H_
