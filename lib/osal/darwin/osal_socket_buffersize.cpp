/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <ifaddrs.h>
#include <errno.h>
#include <inttypes.h>

#include <algorithm>
#include <vector>
#include <limits>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "common/osal_logger.h"
#include "darwin/osal_darwinerror.h"
#include "linux/socket_info_manager.h"

namespace senscord {
namespace osal {

/**
 * @brief Convert OSSocket to socket fd.
 */
static int32_t GetSocketFd(OSSocket* socket) {
  intptr_t temp = reinterpret_cast<intptr_t>(socket);
  int32_t socket_fd = static_cast<int32_t>(temp);
  return socket_fd;
}

/**
 * @brief Get the send buffer size.
 * @param[in]  socket      Socket object.
 * @param[out] buffer_size Pointer to the variable that receives the buffer
 *                         size in bytes.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetSocketSendBufferSize(OSSocket* socket, uint32_t* buffer_size) {
  static const OSFunctionId kFuncId = kIdOSGetSocketSendBufferSize;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (buffer_size == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);
  socklen_t length = sizeof(uint32_t);
  int32_t ret = getsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF,
                           buffer_size, &length);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

/**
 * @brief Get the receive buffer size.
 * @param[in]  socket      Socket object.
 * @param[out] buffer_size Pointer to the variable that receives the buffer
 *                         size in bytes.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetSocketRecvBufferSize(OSSocket* socket, uint32_t* buffer_size) {
  static const OSFunctionId kFuncId = kIdOSGetSocketRecvBufferSize;
  if (socket == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  if (buffer_size == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t socket_fd = GetSocketFd(socket);
  socklen_t length = sizeof(uint32_t);
  int32_t ret = getsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF,
                           buffer_size, &length);
  if (ret != 0) {
    OSErrorCause cause = GetErrorCauseFromErrno(errno);
    return OSMakeErrorCode(kFuncId, cause);
  }
  return 0;
}

}  // namespace osal
}  // namespace senscord
