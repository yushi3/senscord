/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_MESSENGER_H_
#define SENSCORD_MESSENGER_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/senscord_types.h"
#include "senscord/status.h"
#include "senscord/noncopyable.h"
#include "senscord/develop/common_types.h"
#include "senscord/develop/property_accessor.h"

namespace senscord {

/**
 * @brief Publisher class.
 */
class Publisher : private util::Noncopyable {
 public:
  /**
   * @brief Publish frames to connected stream.
   * @param[in] (frames) The publish frames.
   * @return Status object.
   */
  virtual Status PublishFrames(const std::vector< FrameInfo>& frames) = 0;

  /**
   * @brief Get the memory allocator by the name.
   * @param[in] (name) The memory allocator name.
   * @param[out] (allocator) The accessor of memory allocator.
   * @return Status object.
   */
  virtual Status GetAllocator(
      const std::string& name, MemoryAllocator** allocator) = 0;

  /**
   * @brief Get the key when publisher is opened.
   * @return The pulbisher key.
   */
  virtual std::string GetKey() const = 0;

  /**
   * @brief Set the user data for release frame callback.
   * @param[in] (user_data) The user data.
   * @return Status object.
   */
  virtual Status SetCallbackUserData(uintptr_t user_data) = 0;

  /**
   * @brief Update frame channel property.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Property to updated.
   * @return Status object.
   */
  template <typename T>
  Status UpdateChannelProperty(
      uint32_t channel_id, const std::string& key, const T* property);

  /**
   * @Destructor
   */
  virtual ~Publisher() {}

 protected:
#ifdef SENSCORD_SERIALIZE
  /**
   * @brief Set property to PropertyHistoryBook.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Property to updated.
   * @param[in] (size) Size of property.
   * @return Status object.
   */
  virtual Status SetUpdateChannelProperty(
      uint32_t channel_id, const std::string& key,
      const void* property, size_t property_size) = 0;
#else  // SENSCORD_SERIALIZE
  /**
   * @brief Set property to PropertyHistoryBook.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Property to updated.
   * @param[in] (factory) Property factory.
   * @return Status object.
   */
  virtual Status SetUpdateChannelProperty(
      uint32_t channel_id, const std::string& key,
      const void* property, const PropertyFactoryBase& factory) = 0;
#endif  // SENSCORD_SERIALIZE
};

class PublisherParam : private util::Noncopyable {
 public:
  /**
   * @brief Get the memory allocator by the name.
   * @param[in] (name) The memory allocator name.
   * @param[out] (allocator) The accessor of memory allocator.
   * @return Status object.
   */
  Status GetAllocator(
      const std::string& name, MemoryAllocator** allocator) const;

  /**
   * @brief Get the key when publisher is opened.
   * @return The key when publisher is opened.
   */
  std::string GetKey() const;

  /**
   * @brief Get the user data.
   * @return The user data.
   */
  uintptr_t GetUserData() const;

  /**
   * @brief Constructor.
   */
  explicit PublisherParam(Publisher* publisher, uintptr_t private_data);

  /**
   * @brief Destructor.
   */
  ~PublisherParam();

 private:
  Publisher* publisher_;
  uintptr_t user_data_;
};

}    // namespace senscord

// implementations of template methods.
#include "senscord/messenger_private.h"

#endif  // SENSCORD_MESSENGER_H_
