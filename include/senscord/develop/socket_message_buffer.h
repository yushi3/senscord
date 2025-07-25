/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_SOCKET_MESSAGE_BUFFER_H_
#define SENSCORD_DEVELOP_SOCKET_MESSAGE_BUFFER_H_

#include "senscord/config.h"

#ifdef SENSCORD_SERIALIZE

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include "senscord/status.h"
#include "senscord/serialize_buffer.h"
#include "senscord/noncopyable.h"
#include "senscord/osal.h"

namespace senscord {
namespace serialize {

/**
 * @brief Buffer class for creating socket messages.
 */
class SocketMessageBuffer : public Buffer, private util::Noncopyable {
 public:
  /**
   * @brief SocketMessageBuffer constructor.
   */
  SocketMessageBuffer();

  /**
   * @brief SocketMessageBuffer constructor.
   * @param[in] chunk_size            The size of one chunk.
   * @param[in] write_size_threshold  The threshold value of write size.
   */
  explicit SocketMessageBuffer(
      uint32_t chunk_size, uint32_t write_size_threshold);

  /**
   * @brief SocketMessageBuffer destructor.
   */
  virtual ~SocketMessageBuffer();

  /**
   * @brief Write to the buffer.
   *
   * Do not change the function name and argument of write(),
   * as it matches the std::basic_ostream::write() function.
   *
   * @param[in] buffer  Pointer to the buffer to write.
   * @param[in] size    Number of bytes to write.
   */
  virtual Status write(const void* buffer, size_t size);

  /**
   * @brief Clear the buffer.
   */
  virtual Status clear();

  /**
   * @brief Get the buffer size.
   */
  virtual size_t size() const { return total_size_; }

  /**
   * @brief Get the buffer pointer.
   */
  virtual const uint8_t* data() const { return NULL; }

  /**
   * @brief Get the size of one chunk.
   */
  uint32_t GetChunkSize() const { return chunk_size_; }

  /**
   * @brief Get the threshold value of write size.
   */
  uint32_t GetWriteSizeThreshold() const { return write_size_threshold_; }

  /**
   * @brief Get a list of socket messages.
   */
  const std::vector<osal::OSSocketMessage>& GetList() const {
    return message_list_;
  }

 private:
  struct Chunk {
    uint8_t* buffer;
  };

 private:
  const uint32_t chunk_size_;
  const uint32_t write_size_threshold_;
  std::vector<osal::OSSocketMessage> message_list_;
  std::vector<Chunk> chunk_list_;
  size_t total_size_;
  size_t chunk_offset_;
  bool chunk_continuous_writing_;  // Whether continuous writing to chunk area.
};

}  // namespace serialize
}  // namespace senscord

#endif  // SENSCORD_SERIALIZE
#endif  // SENSCORD_DEVELOP_SOCKET_MESSAGE_BUFFER_H_
