/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "messenger/messenger_manager.h"

#include <utility>

#include "messenger/publisher_core.h"
#include "util/singleton.h"

#ifdef SENSCORD_SERVER
#include "messenger/server_frame_sender.h"
#endif  // SENSCORD_SERVER

namespace {
static const char kLocalHost[] = "localhost";

/**
 * @brief Working thread for monitoring.
 * @param[in] (arg) The instance of messenger manager.
 * @return Always returns normal.
 */
senscord::osal::OSThreadResult MonitorThreadEntry(void* arg) {
  if (arg) {
    senscord::MessengerManager* manager =
        reinterpret_cast<senscord::MessengerManager*>(arg);
    manager->Monitor();
  }
  return static_cast<senscord::osal::OSThreadResult>(0);
}
}  // namespace

namespace senscord {

/**
 * @brief Get singleton instance.
 * @return Instance of messenger manager.
 */
MessengerManager* MessengerManager::GetInstance() {
  // for private constructor / destructor
  struct InnerMessengerManager : public MessengerManager {};
  return util::Singleton<InnerMessengerManager>::GetInstance();
}

/**
 * @brief Constructor
 */
MessengerManager::MessengerManager()
    : thread_(), end_thread_(), reference_count_() {
  topics_mutex_ = new util::Mutex();
  monitor_mutex_ = new util::Mutex();
  osal::OSCreateCond(&monitor_cond_);
}

/**
 * @brief Destructor
 */
MessengerManager::~MessengerManager() {
  StopMonitorThread();
  ReleaseAllTopics();

  osal::OSDestroyCond(monitor_cond_);
  monitor_cond_ = NULL;
  delete topics_mutex_;
  topics_mutex_ = NULL;
  delete monitor_mutex_;
  monitor_mutex_ = NULL;
}

/**
 * @brief Initialize manager.
 * @return Status object.
 */
Status MessengerManager::Init() {
  Status status;
  util::AutoLock lock(topics_mutex_);
  if (reference_count_ == 0) {
    status = StartMonitorThread();
    SENSCORD_STATUS_TRACE(status);
  }
  if (status.ok()) {
    ++reference_count_;
  }
  return status;
}

/**
 * @brief Deinitialize manager.
 * @return Status object.
 */
Status MessengerManager::Exit() {
  util::AutoLock lock(topics_mutex_);
  if (reference_count_ > 0) {
    --reference_count_;
  }
  if (reference_count_ == 0) {
    StopMonitorThread();
    ReleaseAllTopics();
  }
  return Status::OK();
}

/**
 * @brief Monitor resource.
 */
void MessengerManager::Monitor() {
  util::AutoLock lock(monitor_mutex_);
  while (!end_thread_) {
    if (!end_thread_ && release_resource_topics_.empty()) {
      osal::OSWaitCond(monitor_cond_, monitor_mutex_->GetObject());
    }
    while (!release_resource_topics_.empty()) {
      MessengerTopic* topic = release_resource_topics_.back();
      topic->ReleaseUnreferencedResource();
      release_resource_topics_.pop_back();
    }
  }
}

/**
 * @brief Notification of resource release.
 * @param[in] (topic) Target topic.
 */
void MessengerManager::ReleaseResources(MessengerTopic* topic) {
  util::AutoLock lock(monitor_mutex_);
  release_resource_topics_.push_back(topic);
  osal::OSSignalCond(monitor_cond_);
}

/**
 * @brief Get publisher handle.
 * @param[in] (setting) Stream setting.
 * @param[in] (callback) Callback for release frame.
 * @param[out] (publisher) Created publisher handle.
 * @return Status object.
 */
Status MessengerManager::GetPublisher(
    const StreamSetting& setting,
    Core::OnReleaseFrameCallback callback,
    const CoreBehavior* core_behavior,
    PublisherCore** publisher) {
  // get instance config
  ConfigManager* config_manager = core_behavior->GetConfigManager();
  const ComponentInstanceConfig* instance_config =
      config_manager->GetComponentConfigByInstanceName(
        setting.radical_address.instance_name);
  if (instance_config == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseNotFound,
        "not found instance config");
  }
  if (instance_config->component_name != kComponentNamePublisher) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "component name is not the publisher");
  }

  // get topic
  MessengerTopic* topic = NULL;
  Status status = GetTopic(instance_config->instance_name, &topic);

  util::AutoLock lock(monitor_mutex_);
  {
    // new publisher
    if (status.ok()) {
      *publisher = topic->GetPublisher(IsRequiredConnectServer(setting));
      status = (*publisher)->Open(
          setting.stream_key, callback, instance_config->allocator_key_list);
      SENSCORD_STATUS_TRACE(status);
    }

    if (status.ok() && IsRequiredConnectServer(setting)) {
      status = SetupServerFrameSender(setting, topic, config_manager);
      SENSCORD_STATUS_TRACE(status);
    }
  }

  if (!status.ok()) {
    (*publisher)->Close();
    *publisher = NULL;
    ReleaseUnreferencedTopic();
  }
  return status;
}

/**
 * @brief Returns whether it is necessary to connect to the server.
 * @param[in] (setting) Stream setting.
 * @return True is connect to the server.
 */
bool MessengerManager::IsRequiredConnectServer(const StreamSetting& setting) {
#ifdef SENSCORD_SERVER
  return (!setting.client_instance_name.empty() &&
      setting.client_instance_name != kLocalHost);
#else
  return false;
#endif  // SENSCORD_SERVER
}

/**
 * @brief Create frame sender to server resource.
 * @param[in] (setting) Stream setting.
 * @param[in] (topic) Topic handle.
 * @param[in] (config_manager) ConfigManager.
 * @return Status object.
 */
Status MessengerManager::SetupServerFrameSender(
    const StreamSetting& setting,
    MessengerTopic* topic,
    ConfigManager* config_manager) {
  Status status;
#ifdef SENSCORD_SERVER
  ServerFrameSender* sender =
      static_cast<ServerFrameSender*>(topic->GetFrameSender(true));
  const ComponentInstanceConfig* client_config =
      config_manager->GetComponentConfigByInstanceName(
          setting.client_instance_name);
  if (client_config == NULL) {
    return SENSCORD_STATUS_FAIL(
      kStatusBlockCore, Status::kCauseNotFound,
      "not found client instance config: %s",
      setting.client_instance_name.c_str());
  }
  status = sender->Open(setting.stream_key, client_config->arguments);
  SENSCORD_STATUS_TRACE(status);
#endif  // SENSCORD_SERVER
  return status;
}

/**
 * @brief Release publisher handle.
 * @param[in] (publisher) Publisher handle.
 * @return Status object.
 */
Status MessengerManager::ReleasePublisher(PublisherCore* publisher) {
  Status status;
  {
    util::AutoLock lock(monitor_mutex_);
    status = publisher->Close();
    SENSCORD_STATUS_TRACE(status);
    if (status.ok()) {
      MessengerTopic* topic = publisher->GetTopic();
      topic->ReleasePublisher(publisher);
    }
  }
  if (status.ok()) {
    ReleaseUnreferencedTopic();
  }
  return status;
}

/**
 * @brief Get frame sender handle.
 * @param[in] (name) Topic name.
 * @param[out] (frame_sender) Created frame sender handle.
 * @return Status object.
 */
Status MessengerManager::GetFrameSender(
    const std::string& name, FrameSender** frame_sender) {
  MessengerTopic* topic = NULL;
  Status status = GetTopic(name, &topic);
  SENSCORD_STATUS_TRACE(status);
  if (status.ok()) {
    util::AutoLock lock(monitor_mutex_);
    *frame_sender = topic->GetFrameSender(false);  // fix inner sender
  }
  if (status.ok()) {
    status = (*frame_sender)->Open();
    SENSCORD_STATUS_TRACE(status);
  }
  if (!status.ok()) {
    if (topic) {
      topic->ReleaseFrameSender(*frame_sender);
    }
    ReleaseUnreferencedTopic();
  }
  return status;
}

/**
 * @brief Release frame sender handle.
 * @param[in] (frame_sender) frame sender handle.
 * @return Status object.
 */
Status MessengerManager::ReleaseFrameSender(FrameSender* frame_sender) {
  MessengerTopic* topic = frame_sender->GetTopic();
  {
    util::AutoLock lock(monitor_mutex_);
    topic->ReleaseFrameSender(frame_sender);
  }
  ReleaseUnreferencedTopic();
  return Status::OK();
}

/**
 * @brief Get topic by name.
 * @param[in] (name) Topic name.
 * @param[out] (topic) Topic handle.
 * @return Status object.
 */
Status MessengerManager::GetTopic(
    const std::string& name, MessengerTopic** topic) {
  util::AutoLock lock(topics_mutex_);
  TopicMap::iterator found = topics_.find(name);
  if (found != topics_.end()) {
    *topic = found->second;
  } else {
    MessengerTopic* tmp = new MessengerTopic(name);
    topics_.insert(std::make_pair(name, tmp));
    *topic = tmp;
  }
  return Status::OK();
}

/**
 * @brief Release unreferenced topic (garbage collection).
 * @return Status object.
 */
void MessengerManager::ReleaseUnreferencedTopic() {
  util::AutoLock lock(topics_mutex_);
  for (TopicMap::iterator itr = topics_.begin(), end = topics_.end();
      itr != end;) {
    MessengerTopic* topic = itr->second;
    if (!topic->IsReferenced()) {
      topics_.erase(itr++);
      delete topic;
    } else {
      ++itr;
    }
  }
}

/**
 * @brief Start the monitoring thread.
 * @return Status object.
 */
Status MessengerManager::StartMonitorThread() {
  if (!thread_) {
    int32_t ret = osal::OSCreateThread(
        &thread_, MonitorThreadEntry, this, NULL);
    if (ret < 0) {
      return SENSCORD_STATUS_FAIL(
          kStatusBlockCore, Status::kCauseAborted,
          "CreateThread failed: 0x%" PRIx32, ret);
    }
  }
  return Status::OK();
}

/**
 * @brief Stop the monitoring thread.
 */
void MessengerManager::StopMonitorThread() {
  if (thread_) {
    {
      util::AutoLock lock(monitor_mutex_);
      end_thread_ = true;
      osal::OSSignalCond(monitor_cond_);
    }
    osal::OSJoinThread(thread_, NULL);
    thread_ = NULL;
  }
}

/**
 * @brief Release all topics.
 */
void MessengerManager::ReleaseAllTopics() {
  util::AutoLock lock(topics_mutex_);
  while (!topics_.empty()) {
    TopicMap::iterator itr = topics_.begin();
    MessengerTopic* topic = itr->second;
    topics_.erase(itr);
    delete topic;
  }
}

}   // namespace senscord
