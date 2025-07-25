/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include "senscord/develop/socket_message_buffer.h"

namespace {

const uint32_t kMinChunkSize = 0x400;     // 1024 bytes.
const uint32_t kMaxChunkSize = 0x100000;  // 1048576 bytes.
const uint32_t kMinWriteSizeThreshold = 32;
const uint32_t kMaxWriteSizeThreshold = 65536;

/**
 * @brief Calculates the write size threshold.
 */
uint32_t CalcWriteSizeThreshold(uint32_t write_size_threshold) {
  uint32_t result = write_size_threshold;
  if (result < kMinWriteSizeThreshold) {
    result = kMinWriteSizeThreshold;
  } else if (result > kMaxWriteSizeThreshold) {
    result = kMaxWriteSizeThreshold;
  }
  return result;
}

/**
 * @brief Calculates the chunk size.
 */
uint32_t CalcChunkSize(uint32_t chunk_size, uint32_t write_size_threshold) {
  write_size_threshold = CalcWriteSizeThreshold(write_size_threshold);
  uint32_t result = chunk_size;
  if (result < write_size_threshold * 4) {
    result = write_size_threshold * 4;
  }
  if (result < kMinChunkSize) {
    result = kMinChunkSize;
  } else if (result > kMaxChunkSize) {
    result = kMaxChunkSize;
  }
  return result;
}

}  // namespace

namespace senscord {
namespace serialize {

/**
 * @brief SocketMessageBuffer constructor.
 */
SocketMessageBuffer::SocketMessageBuffer()
    : chunk_size_(kMinChunkSize)
    , write_size_threshold_(kMinWriteSizeThreshold)
    , message_list_(), chunk_list_(), total_size_(), chunk_offset_()
    , chunk_continuous_writing_() {
}

/**
 * @brief SocketMessageBuffer constructor.
 * @param[in] chunk_size            The size of one chunk.
 * @param[in] write_size_threshold  The threshold value of write size.
 */
SocketMessageBuffer::SocketMessageBuffer(
    uint32_t chunk_size, uint32_t write_size_threshold)
    : chunk_size_(CalcChunkSize(chunk_size, write_size_threshold))
    , write_size_threshold_(CalcWriteSizeThreshold(write_size_threshold))
    , message_list_(), chunk_list_(), total_size_(), chunk_offset_()
    , chunk_continuous_writing_() {
}

/**
 * @brief SocketMessageBuffer destructor.
 */
SocketMessageBuffer::~SocketMessageBuffer() {
  std::vector<Chunk>::const_iterator itr = chunk_list_.begin();
  std::vector<Chunk>::const_iterator end = chunk_list_.end();
  for (; itr != end; ++itr) {
    delete[] itr->buffer;
  }
}

/**
 * @brief Write to the buffer.
 * @param[in] buffer  Pointer to the buffer to write.
 * @param[in] size    Number of bytes to write.
 */
Status SocketMessageBuffer::write(const void* buffer, size_t size) {
  if (size == 0) {
    return Status::OK();
  }
  if (buffer == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "buffer is NULL");
  }
  if (size < write_size_threshold_) {
    // copy to chunk.
    uint8_t* chunk_ptr = NULL;
    if (!chunk_list_.empty() && (chunk_size_ - chunk_offset_ >= size)) {
      // copy to the end of the chunk area.
      const Chunk& chunk = *chunk_list_.rbegin();
      chunk_ptr = chunk.buffer + chunk_offset_;
      int32_t ret = osal::OSMemcpy(
          chunk_ptr, chunk_size_ - chunk_offset_, buffer, size);
      if (ret != 0) {
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseAborted, "memory copy failed.");
      }
      chunk_offset_ += size;
    } else {
      // copy to the new chunk area.
      chunk_ptr = new uint8_t[chunk_size_];
      int32_t ret = osal::OSMemcpy(
          chunk_ptr, chunk_size_, buffer, size);
      if (ret != 0) {
        delete[] chunk_ptr;
        return SENSCORD_STATUS_FAIL(kStatusBlockCore,
            Status::kCauseAborted, "memory copy failed.");
      }
      Chunk chunk = {};
      chunk.buffer = chunk_ptr;
      chunk_list_.push_back(chunk);
      chunk_offset_ = size;
      chunk_continuous_writing_ = false;
    }

    if (chunk_continuous_writing_) {
      // since writing to chunk is continuous, increase the message size.
      osal::OSSocketMessage& msg = *message_list_.rbegin();
      msg.buffer_size += size;
    } else {
      // since writing to chunk is discontinuous, add a new message.
      osal::OSSocketMessage msg = {};
      msg.buffer = chunk_ptr;
      msg.buffer_size = size;
      message_list_.push_back(msg);
    }

    chunk_continuous_writing_ = true;
  } else {
    // add to the message list.
    osal::OSSocketMessage msg = {};
    msg.buffer = const_cast<void*>(buffer);
    msg.buffer_size = size;
    message_list_.push_back(msg);

    chunk_continuous_writing_ = false;
  }
  total_size_ += size;
  return Status::OK();
}

/**
 * @brief Clear the buffer.
 */
Status SocketMessageBuffer::clear() {
  message_list_.clear();
  std::vector<Chunk>::const_iterator itr = chunk_list_.begin();
  std::vector<Chunk>::const_iterator end = chunk_list_.end();
  for (; itr != end; ++itr) {
    delete[] itr->buffer;
  }
  chunk_list_.clear();
  total_size_ = 0;
  chunk_offset_ = 0;
  chunk_continuous_writing_ = false;
  return Status::OK();
}

}  // namespace serialize
}  // namespace senscord
