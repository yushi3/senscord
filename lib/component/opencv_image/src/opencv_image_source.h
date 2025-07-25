/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_OPENCV_IMAGE_SRC_OPENCV_IMAGE_SOURCE_H_
#define LIB_COMPONENT_OPENCV_IMAGE_SRC_OPENCV_IMAGE_SOURCE_H_

#include <string>
#include <vector>
#include <queue>

#include "senscord/develop/stream_source.h"
#include "senscord/osal.h"

#include <opencv2/videoio.hpp>

/**
 * @brief The stream source of opencv image.
 */
class OpenCvImageSource : public senscord::ImageStreamSource {
 public:
  /**
   * @brief Constructor
   */
  OpenCvImageSource();

  /**
   * @brief Destructor
   */
  ~OpenCvImageSource();

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
      const std::string& key, senscord::ImageProperty* property);

  /**
   * @brief Set the new stream source property.
   * @param[in] (key) The key of property.
   * @param[in] (property) The location of property.
   * @return The status of function.
   */
  virtual senscord::Status Set(
      const std::string& key, const senscord::ImageProperty* property);

 private:
  senscord::Status CreateMemoryList();
  void ClearMemoryList();
  senscord::Memory* GetMemory();
  void ReleaseMemory(senscord::Memory* memory);

 private:
  senscord::StreamSourceUtility* util_;
  senscord::MemoryAllocator* allocator_;

  // Memory list
  std::vector<senscord::Memory*> memory_list_;
  std::queue<senscord::Memory*> memory_queue_;
  senscord::osal::OSMutex* memory_queue_mutex_;

  // instance argument
  /** video device id. */
  int32_t device_id_;
  /** Number of buffers. (If 0, allocate each time) */
  uint32_t buffer_num_;
  /** video or image file */
  std::string filename_;

  uint64_t frame_seq_num_;
  bool running_;

  senscord::ImageProperty image_property_;

  // OpenCV
  cv::VideoCapture video_;
};

#endif  // LIB_COMPONENT_OPENCV_IMAGE_SRC_OPENCV_IMAGE_SOURCE_H_
