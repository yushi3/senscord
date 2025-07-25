/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_V4L2_IMAGE_SRC_V4L2_IMAGE_STREAM_SOURCE_H_
#define LIB_COMPONENT_V4L2_IMAGE_SRC_V4L2_IMAGE_STREAM_SOURCE_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "senscord/develop/stream_source.h"
#include "senscord/osal.h"
#include "v4l2_accessor.h"

/**
 * @brief V4L2 Image Stream Source.
 */
class V4L2ImageStreamSource : public senscord::ImageStreamSource {
 public:
  /**
   * @brief Constructor.
   */
  V4L2ImageStreamSource();

  /**
   * @brief Destructor.
   */
  ~V4L2ImageStreamSource();

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

  // properties
  senscord::Status Get(
      const std::string& key,
      senscord::ChannelInfoProperty* property);

  senscord::Status Get(
      const std::string& key,
      senscord::FrameRateProperty* property);

  senscord::Status Set(
      const std::string& key,
      const senscord::FrameRateProperty* property);

  senscord::Status Get(
      const std::string& key,
      senscord::ImageProperty* property);

  senscord::Status Set(
      const std::string& key,
      const senscord::ImageProperty* property);

  senscord::Status Get(
      const std::string& key,
      senscord::ImageSensorFunctionSupportedProperty* property);

 protected:
  senscord::StreamSourceUtility* util_;

 private:
  /**
   * @brief Parse parameter.
   */
  void ParseParameter();

  /**
   * @brief Get used buffer number.
   * @return used buffer number.
   */
  uint32_t GetUsedBufferNum() const;

  /**
   * @brief Allocate device buffer.
   * @return Status object.
   */
  senscord::Status AllocateBuffer();

  /**
   * @brief Free device buffer.
   * @return Status object.
   */
  senscord::Status FreeBuffer();

  /**
   * @brief Set parameter to device.
   * @return Status object.
   */
  senscord::Status SetDeviceParameter();

  /**
  * @brief Buffer structure.
  */
  struct BufferInfo {
    uint32_t index;               /**< Buffer index. */
    uint32_t length;              /**< Mmap length. */
    void* addr;                   /**< Mmap address. */
    bool used;                    /**< Is used buffer. */
    senscord::Memory* memory;     /**< Memory. */
  };
  std::vector<BufferInfo> buffer_list_;
  senscord::osal::OSMutex* buffer_mutex_;

  /**
  * @brief Device settings.
  */
  struct DeviceSettings {
    std::string device;           /**< Device path. */
    uint32_t buffer_num;          /**< Buffer number. */
  };
  DeviceSettings settings_;

  V4L2Accessor device_;

  uint64_t frame_seq_num_;
  senscord::MemoryAllocator* allocator_;

  senscord::ImageProperty image_property_;
  senscord::FrameRateProperty framerate_property_;

  bool is_started_;
  bool is_yuyv_to_nv16_;
};

#endif  // LIB_COMPONENT_V4L2_IMAGE_SRC_V4L2_IMAGE_STREAM_SOURCE_H_
