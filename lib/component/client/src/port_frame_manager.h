/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_CLIENT_PORT_FRAME_MANAGER_H_
#define LIB_COMPONENT_CLIENT_PORT_FRAME_MANAGER_H_

#include <stdint.h>
#include <map>
#include "senscord/osal.h"
#include "senscord/status.h"
#include "senscord/noncopyable.h"
#include "senscord/develop/common_types.h"

namespace client {

struct PortParameter;

/**
 * @brief Frame event listener interface.
 */
class PortFrameEventListener {
 public:
  /**
   * @brief Destructor.
   */
  virtual ~PortFrameEventListener() {}

  /**
   * @brief Release the all frames.
   *
   * This function is called when the following conditions:
   * - When stream stop is called when there is no frame being acquired.
   * - When all frames are released after stream stop.
   *
   * @param[in] (port_id) Port ID.
   */
  virtual void OnReleaseAllFrames(int32_t port_id) = 0;
};

/**
 * @brief Frame management class for each port.
 */
class PortFrameManager : private senscord::util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   * @param[in] (listener) Frame event listener.
   */
  explicit PortFrameManager(PortFrameEventListener* listener);

  /**
   * @brief Destructor.
   */
  ~PortFrameManager();

  /**
   * @brief Set the specified port to the stream start state.
   * @param[in] (port_id) Port ID.
   * @return Status object.
   */
  senscord::Status Start(int32_t port_id);

  /**
   * @brief Set the specified port to the stream stop state.
   * @param[in] (port_id) Port ID.
   * @return Status object.
   */
  senscord::Status Stop(int32_t port_id);

  /**
   * @brief Add the frame to the management target.
   * @param[in] (port_id) Port ID.
   * @param[in] (sequence_number) Frame sequence number.
   * @return Status object.
   */
  senscord::Status AddFrame(int32_t port_id, uint64_t sequence_number);

  /**
   * @brief Remove the frame from the management target.
   * @param[in] (port_id) Port ID.
   * @param[in] (sequence_number) Frame sequence number.
   * @return Status object.
   */
  senscord::Status RemoveFrame(int32_t port_id, uint64_t sequence_number);

 private:
  PortFrameEventListener* listener_;
  std::map<int32_t, PortParameter*> list_;
  senscord::osal::OSMutex* mutex_;
};

}  // namespace client

#endif  // LIB_COMPONENT_CLIENT_PORT_FRAME_MANAGER_H_
