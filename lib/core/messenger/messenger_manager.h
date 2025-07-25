/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_MESSENGER_MESSENGER_MANAGER_H_
#define LIB_CORE_MESSENGER_MESSENGER_MANAGER_H_

#include <map>
#include <string>
#include <vector>

#include "component/component_adapter.h"
#include "messenger/publisher_core.h"
#include "messenger/messenger_topic.h"

namespace senscord {

class PublisherCore;
class MessengerTopic;

/**
 * @brief Meesenger manager class.
 */
class MessengerManager : private util::Noncopyable {
 public:
  /**
   * @brief Get singleton instance.
   * @return Instance of Meesenger manager.
   */
  static MessengerManager* GetInstance();

  /**
   * @brief Initialize manager.
   * @return Status object.
   */
  Status Init();

  /**
   * @brief Deinitialize manager.
   * @return Status object.
   */
  Status Exit();

  /**
   * @brief Get publisher handle.
   * @param[in] (setting) Stream setting.
   * @param[in] (callback) Callback for release frame.
   * @param[out] (publisher) Created publisher handle.
   * @return Status object.
   */
  Status GetPublisher(
      const StreamSetting& setting,
      Core::OnReleaseFrameCallback callback,
      const CoreBehavior* core_behavior,
      PublisherCore** publisher);

  /**
   * @brief Release publisher handle.
   * @param[in] (publisher) Publisher handle.
   * @return Status object.
   */
  Status ReleasePublisher(PublisherCore* publisher);

  /**
   * @brief Get frame sender handle.
   * @param[in] (name) Topic name.
   * @param[out] (frame_sender) Created frame sender handle.
   * @return Status object.
   */
  Status GetFrameSender(
      const std::string& name, FrameSender** frame_sender);

  /**
   * @brief Release frame sender handle.
   * @param[in] (frame_sender) frame sender handle.
   * @return Status object.
   */
  Status ReleaseFrameSender(FrameSender* frame_sender);

  /**
   * @brief Monitor resource.
   */
  void Monitor();

  /**
   * @brief Notification of resource release.
   * @param[in] (topic) Target topic.
   */
  void ReleaseResources(MessengerTopic* topic);

 private:
  /**
   * @brief Constructor
   */
  MessengerManager();

  /**
   * @brief Destructor
   */
  ~MessengerManager();

  /**
   * @brief Get topic by name.
   * @param[in] (name) Topic name.
   * @param[out] (topic) Topic handle.
   * @return Status object.
   */
  Status GetTopic(const std::string& name, MessengerTopic** topic);

  /**
   * @brief Release unreferenced topic (garbage collection).
   * @return Status object.
   */
  void ReleaseUnreferencedTopic();

  /**
   * @brief Create publisher resource.
   * @param[in] (setting) Stream setting.
   * @param[in] (topic) Topic handle.
   * @param[in] (callback) Relelase frame callback.
   * @param[in] (allocator_keys) Allocator keys.
   * @param[out] (publisher) Created publisher.
   * @return Status object.
   */
  Status CreatePublisher(
      const StreamSetting& setting,
      MessengerTopic* topic,
      Core::OnReleaseFrameCallback callback,
      const std::map<std::string, std::string>& allocator_keys,
      PublisherCore** publisher);

  /**
   * @brief Create frame sender to server resource.
   * @param[in] (setting) Stream setting.
   * @param[in] (topic) Topic handle.
   * @param[in] (config_manager) ConfigManager.
   * @return Status object.
   */
  Status SetupServerFrameSender(
      const StreamSetting& setting,
      MessengerTopic* topic,
      ConfigManager* config_manager);

  /**
   * @brief Returns whether it is necessary to connect to the server.
   * @param[in] (setting) Stream setting.
   * @return True is connect to the server.
   */
  bool IsRequiredConnectServer(const StreamSetting& setting);

  /**
   * @brief Start the monitoring thread.
   * @return Status object.
   */
  Status StartMonitorThread();

  /**
   * @brief Stop the monitoring thread.
   */
  void StopMonitorThread();

  /**
   * @brief Release all topics.
   */
  void ReleaseAllTopics();

  // key is topic name
  typedef std::map<std::string, MessengerTopic*> TopicMap;
  TopicMap topics_;
  util::Mutex* topics_mutex_;

  // for resource monitor
  osal::OSThread* thread_;
  osal::OSCond* monitor_cond_;
  util::Mutex* monitor_mutex_;
  bool end_thread_;
  std::vector<MessengerTopic*> release_resource_topics_;
  int32_t reference_count_;
};

}   // namespace senscord
#endif  // LIB_CORE_MESSENGER_MESSENGER_MANAGER_H_
