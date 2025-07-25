/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_OSAL_WINDOWS_SOCKET_INFO_MANAGER_H_
#define LIB_OSAL_WINDOWS_SOCKET_INFO_MANAGER_H_

#include <stdint.h>
#include <Windows.h>
#include <map>

#include "senscord/osal.h"
#include "common/osal_error.h"

namespace senscord {
namespace osal {

/**
 * @brief Socket information.
 */
struct SocketInfo {
  bool writable;  /**< Writable flag : Used in select() function */
};

/**
 * @brief Socket information management class.
 */
class SocketInfoManager {
 public:
  /**
   * @brief Get singleton instance.
   */
  static SocketInfoManager* GetInstance();

  /**
   * @brief Insert socket information.
   * @param[in] socket  Socket object.
   * @param[in] info    Socket information.
   * @return OSAL error cause. On success, it returns kErrorNone.
   */
  OSErrorCause Insert(OSSocket* socket, const SocketInfo& info);

  /**
   * @brief Set socket information.
   * @param[in] socket  Socket object.
   * @param[in] info    Socket information.
   * @return OSAL error cause. On success, it returns kErrorNone.
   */
  OSErrorCause Set(OSSocket* socket, const SocketInfo& info);

  /**
   * @brief Get socket information.
   * @param[in]  socket  Socket object.
   * @param[out] info    Pointer to the variable that receives the socket
   *                     information.
   * @return OSAL error cause. On success, it returns kErrorNone.
   */
  OSErrorCause Get(OSSocket* socket, SocketInfo* info) const;

  /**
   * @brief Delete socket information.
   * @param[in]  socket  Socket object.
   * @param[out] info    Pointer to the variable that receives the socket
   *                     information. (optional)
   * @return OSAL error cause. On success, it returns kErrorNone.
   */
  OSErrorCause Delete(OSSocket* socket, SocketInfo* info = NULL);

 private:
  /**
   * @brief Constructor.
   */
  SocketInfoManager();

  /**
   * @brief Destructor.
   */
  virtual ~SocketInfoManager();

  SocketInfoManager(const SocketInfoManager&);  // = delete;
  SocketInfoManager& operator=(const SocketInfoManager&);  // = delete;

 private:
  typedef std::map<OSSocket*, SocketInfo*> List;
  List list_;
  mutable CRITICAL_SECTION critical_;
};

}  //  namespace osal
}  //  namespace senscord

#endif  // LIB_OSAL_WINDOWS_SOCKET_INFO_MANAGER_H_
