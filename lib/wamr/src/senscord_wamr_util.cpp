/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/senscord_wamr_util.h"

#include "c_api/c_common.h"
#include "stream/stream_core.h"
#include "frame/frame_core.h"

#include "wasm_export.h"

namespace c_api = senscord::c_api;

/**
 * @brief WasmThreadEnv constructor.
 */
WasmThreadEnv::WasmThreadEnv() : thread_env_inited_() {
  if (!wasm_runtime_thread_env_inited()) {
    thread_env_inited_ = wasm_runtime_init_thread_env();
  }
}

/**
 * @brief WasmThreadEnv destructor.
 */
WasmThreadEnv::~WasmThreadEnv() {
  if (thread_env_inited_) {
    wasm_runtime_destroy_thread_env();
  }
}

/**
 * @brief Get stream key.
 * @param[in] stream  Stream handle.
 * @return Stream key.
 */
const char* senscord_stream_get_key(senscord_stream_t stream) {
  const char* stream_key = NULL;
  senscord::StreamCore* stream_ptr =
      c_api::ToPointer<senscord::StreamCore*>(stream);
  if (stream_ptr != NULL) {
    stream_key = stream_ptr->GetKey().c_str();
  }
  return stream_key;
}

/**
 * @brief Get parent stream handle.
 * @param[in] frame  Frame handle.
 * @return Stream handle.
 */
senscord_stream_t senscord_frame_get_parent_stream(senscord_frame_t frame) {
  senscord_stream_t stream_handle = 0;
  senscord::FrameCore* frame_ptr =
      c_api::ToPointer<senscord::FrameCore*>(frame);
  if (frame_ptr != NULL) {
    stream_handle = c_api::ToHandle(frame_ptr->GetParentStream());
  }
  return stream_handle;
}
