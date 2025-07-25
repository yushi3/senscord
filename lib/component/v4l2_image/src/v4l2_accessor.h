/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_V4L2_IMAGE_SRC_V4L2_ACCESSOR_H_
#define LIB_COMPONENT_V4L2_IMAGE_SRC_V4L2_ACCESSOR_H_

#include <stdint.h>
#include <linux/videodev2.h>    /* v4l2 */
#include <string>
#include "senscord/senscord.h"

/**
 * @brief V4L2 accessor.
 */
class V4L2Accessor {
 public:
  /**
   * @brief Constructor.
   */
  V4L2Accessor();

  /**
   * @brief Destructor.
   */
  ~V4L2Accessor();

  /**
   * @brief Device open.
   * @param[in] (path) device path.
   * @return Status object.
   */
  senscord::Status DevOpen(const std::string& path);

  /**
   * @brief Device close.
   * @return Status object.
   */
  senscord::Status DevClose();

  /**
   * @brief Get format to device.
   * @param[out] (property) image property.
   * @return Status object.
   */
  senscord::Status GetDevFormat(senscord::ImageProperty* property);

  /**
   * @brief Set format to device.
   * @param[in] (property) image property.
   * @return Status object.
   */
  senscord::Status SetDevFormat(const senscord::ImageProperty& property);

  /**
   * @brief Get framerate to device.
   * @param[out] (property) framerate property.
   * @return Status object.
   */
  senscord::Status GetFramerate(senscord::FrameRateProperty* property);

  /**
   * @brief Set framerate to device.
   * @param[in] (property) framerate property.
   * @return Status object.
   */
  senscord::Status SetFramerate(const senscord::FrameRateProperty& property);

  /**
   * @brief Set request buffer to device.
   * @param[in] (num_req) request number of buffer.
   * @return Status object.
   */
  senscord::Status SetReqBuffer(uint32_t num_req);

  /**
   * @brief Free buffer to device.
   * @return Status object.
   */
  senscord::Status FreeReqBuffer();

  /**
   * @brief Query buffer.
   * @param[in] (index) Inquiry buffer index.
   * @param[out] (buffer) Buffer of inquiry result.
   * @return Status object.
   */
  senscord::Status QueryBuffer(
      const uint32_t index, struct v4l2_buffer* buffer);

  /**
   * @brief Queue buffer.
   * @param[in] (buffer) Target queue buffer.
   * @return Status object.
   */
  senscord::Status QueueBuffer(v4l2_buffer* buffer);

  /**
   * @brief Dequeue buffer.
   * @param[out] (buffer) Buffer of dequeue result.
   * @return Status object.
   */
  senscord::Status DequeueBuffer(v4l2_buffer* buffer);

  /**
   * @brief Mmap to device.
   * @param[in] (index) Buffer index.
   * @param[out] (addr) Mmap address.
   * @param[out] (size) Mmap size
   * @return Status object.
   */
  senscord::Status Mmap(uint32_t index, void** addr, uint32_t* size);

  /**
   * @brief Munmap to device.
   * @param[in] (addr) Mmap address.
   * @param[in] (size) Mmap size.
   * @return Status object.
   */
  senscord::Status Munmap(void* addr, uint32_t size);

  /**
   * @brief Set start stream to device.
   * @return Status object.
   */
  senscord::Status DevStart();

  /**
   * @brief Set stop stream to device.
   * @return Status object.
   */
  senscord::Status DevStop();

 private:
  /**
   * @brief Get v4l2 pixelformat from senscord pixel format.
   * @param[in] (format) SensCord pixel format.
   * @param[out] (converted) V4L2 pixel format.
   * @return Status object.
   */
  senscord::Status GetV4L2PixelFormat(
      const std::string& format, uint32_t* converted) const;

  /**
   * @brief Get senscord pixelformat from v4l2 pixel format.
   * @param[in] (format) V4L2 pixel format.
   * @param[out] (converted) SensCord pixel format.
   * @return Status object.
   */
  senscord::Status GetSensCordPixelFormat(
      const uint32_t format, std::string* converted) const;

  int32_t fd_;
};

#endif  // LIB_COMPONENT_V4L2_IMAGE_SRC_V4L2_ACCESSOR_H_
