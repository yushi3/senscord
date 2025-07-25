/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_SERVER_CLIENTLESS_CORE_BEHAVIOR_H_
#define LIB_SERVER_CLIENTLESS_CORE_BEHAVIOR_H_

#include <stdint.h>
#include <string>

#include "senscord/senscord_types.h"
#include "senscord/status.h"
#include "senscord/stream.h"

// TODO: private header access
#include "core/core_behavior.h"

namespace senscord {
namespace server {

/**
 * @brief Behavior class to open stream without client.
 */
class ClientlessCoreBehavior : public DefaultCoreBehavior {
 public:
  /**
   * @brief Constructor.
   */
  ClientlessCoreBehavior();

  /**
   * @brief Destructor.
   */
  virtual ~ClientlessCoreBehavior();

  /**
   * @brief Create instance.
   *
   * Use 'delete' to release the created instance.
   * @return created instance.
   */
  virtual CoreBehavior* CreateInstance() const;

#ifdef SENSCORD_STREAM_VERSION
  /**
   * @brief Get the version of this core library.
   * @param[out] (version) The version of this core library.
   * @return Status object.
   */
  Status GetVersion(SensCordVersion* version);
#endif  // SENSCORD_STREAM_VERSION

  /**
   * @brief Open the new stream from key and specified setting.
   * @param[in] (key) The key of the stream to open.
   * @param[in] (setting) Setting to open stream.
   * @param[out] (stream) The new stream pointer.
   * @return Status object.
   */
  virtual Status OpenStream(
      const std::string& key,
      const OpenStreamSetting* setting,
      Stream** stream);

 protected:
#ifdef SENSCORD_STREAM_VERSION
  /**
   * @brief Read component config.
   * @return Status object.
   */
  virtual Status ReadComponentConfig();
#endif  // SENSCORD_STREAM_VERSION
};

}  // namespace server
}  // namespace senscord

#endif  // LIB_SERVER_CLIENTLESS_CORE_BEHAVIOR_H_
