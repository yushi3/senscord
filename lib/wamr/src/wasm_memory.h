/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_WAMR_SRC_WASM_MEMORY_H_
#define LIB_WAMR_SRC_WASM_MEMORY_H_

#include <stdint.h>

#include "senscord/memory.h"
#include "senscord/memory_allocator.h"

namespace senscord {

/**
 * WASM Memory.
 */
class WasmMemory : public Memory {
 public:
  /**
   * @brief Constructor.
   * @param[in] (native_address) Native address.
   * @param[in] (wasm_address) WASM address.
   * @param[in] (size) Memory block size.
   * @param[in] (allocator) Depend allocator.
   */
  explicit WasmMemory(
      uintptr_t native_address, uint32_t wasm_address, uint32_t size,
      MemoryAllocator* allocator)
      : native_address_(native_address), wasm_address_(wasm_address),
        size_(size), allocator_(allocator) {
  }

  /**
   * @brief Destructor.
   */
  virtual ~WasmMemory() {}

  /**
   * @brief Returns native address.
   * @return native address.
   */
  uintptr_t GetAddress() const {
    return native_address_;
  }

  /**
   * @brief Set native address.
   * @param[in] (address) Native address.
   */
  void SetAddress(uintptr_t address) {
    native_address_ = address;
  }

  /**
   * @brief Returns WASM address.
   * @return WASM address.
   */
  uint32_t GetWasmAddress() const {
    return wasm_address_;
  }

  /**
   * @brief Set WASM address.
   * @param[in] (address) WASM address.
   */
  void SetWasmAddress(uint32_t address) {
    wasm_address_ = address;
  }

  /**
   * @brief Returns memory block size.
   * @return Memory block size.
   */
  size_t GetSize() const {
    return size_;
  }

  /**
   * @brief Invalidate a memory block.
   * @return Status object.
   */
  Status Invalidate() {
    Status status;
    if (allocator_ != NULL) {
      status = allocator_->InvalidateCache(native_address_, size_);
    }
    return SENSCORD_STATUS_TRACE(status);
  }

  /**
   * @brief Get depend allocator instance.
   * @return Allocator instance.
   */
  MemoryAllocator* GetAllocator() const {
    return allocator_;
  }

 private:
  uintptr_t native_address_;
  uint32_t wasm_address_;
  size_t size_;
  MemoryAllocator* allocator_;
};

}  // namespace senscord

#endif  // LIB_WAMR_SRC_WASM_MEMORY_H_
