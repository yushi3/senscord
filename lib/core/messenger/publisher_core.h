/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_MESSENGER_PUBLISHER_CORE_H_
#define LIB_CORE_MESSENGER_PUBLISHER_CORE_H_

#include <vector>
#include <string>
#include <map>

#include "senscord/senscord.h"
#include "senscord/messenger.h"
#include "core/internal_types.h"
#include "messenger/messenger_topic.h"
#include "util/mutex.h"

namespace senscord {

class MessengerTopic;

/**
 * @brief Publisher core class.
 */
class PublisherCore : public Publisher {
 public:
  /**
   * @brief Constructor
   * @param[in] (topic) Parent messenger topic.
   */
  explicit PublisherCore(MessengerTopic* topic);

  /**
   * @brief Destructor
   */
  virtual ~PublisherCore();

  /**
   * @brief Open the publisher.
   * @param[in] (key) Publisher key.
   * @param[in] (callback) Release frame callback.
   * @param[in] (allocator_keys) Memory allocator keys.
   * @return Status object.
   */
  Status Open(
      const std::string& key,
      Core::OnReleaseFrameCallback callback,
      const std::map<std::string, std::string>& allocator_keys);

  /**
   * @brief Close the publisher.
   * @return Status object.
   */
  Status Close();

  /**
   * @brief Publish frames to connected stream.
   * @param[in] (frames) The publish frames.
   * @return Status object.
   */
  virtual Status PublishFrames(const std::vector<FrameInfo>& frames);

  /**
   * @brief Release the used frame.
   * @param[in] (frameinfo) The information about used frame.
   * @return The status of function.
   */
  void ReleaseFrame(const FrameInfo& frameinfo);

  /**
   * @brief Set the user data for release frame callback.
   * @param[in] (user_data) The user data.
   * @return Status object.
   */
  virtual Status SetCallbackUserData(uintptr_t user_data);

  /**
   * @brief Get topic.
   * @return The topic instance.
   */
  MessengerTopic* GetTopic() const { return topic_; }

  /**
   * @brief Get the memory allocator by the name.
   * @param[in] (name) The memory allocator name.
   * @param[out] (allocator) The accessor of memory allocator.
   * @return Status object.
   */
  Status GetAllocator(const std::string& name, MemoryAllocator** allocator);

  /**
   * @brief Get the key when publisher is opened.
   * @return The pulbisher key.
   */
  std::string GetKey() const { return key_; }

  /**
   * @brief State of publisher.
   */
  enum PublisherState {
    kPublisherStateInit = 0,
    kPublisherStateOpen,
    kPublisherStateClose,
  };

  /**
   * @brief Get the publisher state.
   * @return Publisher state.
   */
  PublisherState GetState() const;

  /**
   * @brief Set the publisher state.
   * @param[in] (state) Pulbisher state.
   */
  void SetState(PublisherState state);

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
      const void* property, size_t property_size);
#else
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
      const void* property, const PropertyFactoryBase& factory);
#endif

 private:
  /**
   * @brief Set allocators by keys.
   * @param[in] (allocator_keys) Memory allocator keys.
   * @return Status object.
   */
  Status SetAllocators(
      const std::map<std::string, std::string>& allocator_keys);

  Core::OnReleaseFrameCallback callback_;
  uintptr_t callback_user_data_;
  std::string key_;
  MessengerTopic* topic_;
  std::map<std::string, MemoryAllocator*> allocators_;
  PublisherState state_;
  util::Mutex* state_mutex_;
};

}   // namespace senscord
#endif  // LIB_CORE_MESSENGER_PUBLISHER_CORE_H_
