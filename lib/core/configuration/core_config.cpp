/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "configuration/core_config.h"

namespace senscord {

/**
 * @brief Searches stream config by stream key.
 * @param[in] (stream_list) Stream config list.
 * @param[in] (stream_key) Stream key.
 * @return Stream config.
 */
StreamSetting* GetStreamConfig(
    std::vector<StreamSetting>* stream_list,
    const std::string& stream_key) {
  for (std::vector<StreamSetting>::iterator
      itr = stream_list->begin(), end = stream_list->end();
      itr != end; ++itr) {
    if (itr->stream_key == stream_key) {
      return &(*itr);
    }
  }
  return NULL;
}

/**
 * @brief Searches stream config by stream key. (Backward match)
 * @param[in] (stream_list) Stream config list.
 * @param[in] (stream_key) Stream key.
 * @return Stream config.
 */
const StreamSetting* GetStreamConfigBackwardMatch(
    const std::vector<StreamSetting>* stream_list,
    const std::string& stream_key) {
  const StreamSetting* result = NULL;
  for (std::vector<StreamSetting>::const_iterator
      itr = stream_list->begin(), end = stream_list->end();
      itr != end; ++itr) {
    const std::string& target = itr->stream_key;
    // Exact match.
    if (target == stream_key) {
      result = &(*itr);
      break;
    }
    // Backward match.
    if (result == NULL) {
      size_t target_size = target.size();
      size_t suffix_size = stream_key.size();
      if (target_size >= suffix_size && target.find(
          stream_key, target_size - suffix_size) != std::string::npos) {
        result = &(*itr);
      }
    }
  }
  return result;
}

/**
 * @brief Searches component config by instance name.
 * @param[in] (instance_list) Instance config list.
 * @param[in] (instance_name) Instance name.
 * @return Component config.
 */
ComponentInstanceConfig* GetComponentConfig(
    std::vector<ComponentInstanceConfig>* instance_list,
    const std::string& instance_name) {
  for (std::vector<ComponentInstanceConfig>::iterator
      itr = instance_list->begin(), end = instance_list->end();
      itr != end; ++itr) {
    if (itr->instance_name == instance_name) {
      return &(*itr);
    }
  }
  return NULL;
}

/**
 * @brief Searches component config by instance name.
 * @param[in] (instance_list) Instance config list.
 * @param[in] (instance_name) Instance name.
 * @return Component config.
 */
const ComponentInstanceConfig* GetComponentConfig(
    const std::vector<ComponentInstanceConfig>* instance_list,
    const std::string& instance_name) {
  for (std::vector<ComponentInstanceConfig>::const_iterator
      itr = instance_list->begin(), end = instance_list->end();
      itr != end; ++itr) {
    if (itr->instance_name == instance_name) {
      return &(*itr);
    }
  }
  return NULL;
}

/**
 * @brief Searches allocator config by allocator key.
 * @param[in] (allocator_list) Allocator config list.
 * @param[in] (allocator_key) Allocator key.
 * @return Allocator config.
 */
AllocatorConfig* GetAllocatorConfig(
    std::vector<AllocatorConfig>* allocator_list,
    const std::string& allocator_key) {
  for (std::vector<AllocatorConfig>::iterator
      itr = allocator_list->begin(), end = allocator_list->end();
      itr != end; ++itr) {
    if (itr->key == allocator_key) {
      return &(*itr);
    }
  }
  return NULL;
}

/**
 * @brief Searches allocator config by allocator key.
 * @param[in] (allocator_list) Allocator config list.
 * @param[in] (allocator_key) Allocator key.
 * @return Allocator config.
 */
const AllocatorConfig* GetAllocatorConfig(
    const std::vector<AllocatorConfig>* allocator_list,
    const std::string& allocator_key) {
  for (std::vector<AllocatorConfig>::const_iterator
      itr = allocator_list->begin(), end = allocator_list->end();
      itr != end; ++itr) {
    if (itr->key == allocator_key) {
      return &(*itr);
    }
  }
  return NULL;
}

}  // namespace senscord
