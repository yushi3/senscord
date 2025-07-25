/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APPLICATION_MULTI_SERVER_SRC_RESOURCE_ADAPTER_H_
#define APPLICATION_MULTI_SERVER_SRC_RESOURCE_ADAPTER_H_

#include "senscord/noncopyable.h"
#include "senscord/connection_types.h"
#include "senscord/connection.h"
#include "senscord/senscord.h"

namespace senscord {
namespace server {

/**
 * @brief The adapter class to access to the resource.
 */
class ResourceAdapter : private util::Noncopyable {
 public:
  enum MonitorType {
    kMonitorStandard = 0,
    kMonitorLockUnlock,
  };

  /**
   * @brief Close to the resource.
   * @param[in] (core) Core object.
   * @return Status object.
   */
  virtual Status Close(Core* core) = 0;

  /**
   * @brief Start to access to the resource.
   * @return Status object.
   */
  virtual Status StartMonitoring() = 0;

  /**
   * @brief Stop to access to the resource.
   * @return Status object.
   */
  virtual Status StopMonitoring() = 0;

  /**
   * @brief The method to monitor new messages.
   */
  virtual void Monitoring(MonitorType type) = 0;

  /**
   * @brief Push new message for this resource.
   * @param[in] (msg) New incoming message instance.
   */
  virtual void PushMessage(const Message* msg) = 0;

  /**
   * @brief Get the resource id.
   * @return The id of this resource.
   */
  uint64_t GetResourceId() const {
    return resource_id_;
  }

  /**
   * @brief Constructor.
   */
  ResourceAdapter() : resource_id_() {}

  /**
   * @brief Destructor.
   */
  virtual ~ResourceAdapter() {}

 protected:
  uint64_t resource_id_;
};

}  // namespace server
}  // namespace senscord

#endif  // APPLICATION_MULTI_SERVER_SRC_RESOURCE_ADAPTER_H_
