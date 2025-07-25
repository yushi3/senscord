/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_COMPONENT_STREAM_SOURCE_ADAPTER_H_
#define LIB_CORE_COMPONENT_STREAM_SOURCE_ADAPTER_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include "senscord/osal.h"
#include "senscord/status.h"
#include "senscord/develop/property_accessor.h"
#include "senscord/develop/stream_source.h"
#include "senscord/develop/stream_source_utility.h"
#include "senscord/develop/component_port.h"
#include "component/stream_source_function_lock_manager.h"
#include "util/mutex.h"

namespace senscord {

/**
 * @brief Adapter between stream source and component port.
 */
class StreamSourceAdapter : public StreamSourceUtility {
 public:
  /**
   * @brief Constructor
   * @param[in] (core) Core instance.
   * @param[in] (port) Component port.
   * @param[in] (args) Component instance arguments.
   */
  explicit StreamSourceAdapter(
    Core* core, ComponentPort* port, const ComponentArgument& args);

  /**
   * @brief Destructor
   */
  ~StreamSourceAdapter();

  /**
   * @brief Open the stream source.
   * @param[in] (args) Port argument.
   * @return Status object.
   */
  Status Open(const ComponentPortArgument& args);

  /**
   * @brief Close the stream source.
   * @return Status object.
   */
  Status Close();

  /**
   * @brief Start the stream source.
   * @return Status object.
   */
  Status Start();

  /**
   * @brief Stop the stream source.
   * @return Status object.
   */
  Status Stop();

  /**
   * @brief Release the finished frame.
   * @param[in] (frameinfo) Finished frame infomation.
   * @param[in] (referenced_channel_ids) List of referenced channel IDs.
   *                                     (NULL is the same as empty)
   * @return Status object.
   */
  Status ReleaseFrame(
    const FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids);

  /**
   * @brief Start the publishing thread.
   * @return Status object.
   */
  Status StartThreading();

  /**
   * @brief Notify to stop the publishing thread.
   */
  void StopThreadingNotify();

  /**
   * @brief Apply to stop the publishing thread.
   * @return Status object.
   */
  Status StopThreadingApply();

  /**
   * @brief Publishing the frames.
   */
  void Publishing();

  /**
   * @brief Thread for publishing frames.
   * @param[in] (arg) The porinter to this adapter.
   * @return always 0.
   */
  static osal::OSThreadResult Threading(void* arg);

  /**
   * @brief Get the type of stream.
   * @return The type of stream.
   */
  const std::string& GetType() const { return port_->GetPortType(); }

  /**
   * @brief Get the ID of stream.
   * @return The ID of stream.
   */
  int32_t GetId() const { return port_->GetPortId(); }

  /**
   * @brief Get the key of stream.
   * @return The key of stream.
   */
  virtual const std::string& GetStreamKey() const {
    return port_args_->stream_key;
  }

  /**
   * @brief Get the instance name of this component.
   * @return The name of instance.
   */
  virtual const std::string& GetInstanceName() const {
    return component_args_.instance_name;
  }

  /**
   * @brief Get the argument value of instance by the name.
   * @param[in] (name) The argument name.
   * @param[out] (value) The argument value.
   * @return Status object.
   */
  virtual Status GetInstanceArgument(
    const std::string& name, std::string* value) const;

  virtual Status GetInstanceArgument(
    const std::string& name, int64_t* value) const;

  virtual Status GetInstanceArgument(
    const std::string& name, uint64_t* value) const;

  /**
   * @brief Get the argument value of stream by the name.
   * @param[in] (name) The argument name.
   * @param[out] (value) The argument value.
   * @return Status object.
   */
  virtual Status GetStreamArgument(
    const std::string& name, std::string* value) const;

  virtual Status GetStreamArgument(
    const std::string& name, int64_t* value) const;

  virtual Status GetStreamArgument(
    const std::string& name, uint64_t* value) const;

  /**
   * @brief Get the memory allocator by the name.
   * @param[in] (name) The memory allocator name.
   * @param[out] (allocator) The accessor of memory allocator.
   * @return Status object.
   */
  virtual Status GetAllocator(
    const std::string& name, MemoryAllocator** allocator) const;

  /**
   * @brief Send the event to the connected stream.
   * @param[in] (event_type) Event type to send.
   * @param[in] (args) Event argument.
   * @return Status object.
   */
  virtual Status SendEvent(
      const std::string& event_type, const EventArgument& args);

  /**
   * @brief Send the event for kEventError or kEventFatal.
   * @param[in] (error_status) Error status to send.
   * @return Status object.
   */
  virtual Status SendEventError(const Status& error_status);

  /**
   * @brief Send the event for kEventFrameDropped.
   * @param[in] (sequence_number) The sequence number that was dropped.
   * @return Status object.
   */
  virtual Status SendEventFrameDropped(uint64_t sequence_number);

  /**
   * @brief Send the event for kEventPropertyUpdated.
   * @param[in] (property_key) The key of the updated property.
   * @return Status object.
   */
  virtual Status SendEventPropertyUpdated(const std::string& property_key);

  /**
   * @brief Set the pointer of stream source.
   * @param[in] (source) Stream source.
   */
  void SetSource(StreamSource* source) { source_ = source; }

  /**
   * @brief Get the pointer of stream source.
   * @return The pointer of stream source.
   */
  StreamSource* GetSource() const { return source_; }

  /**
   * @biref Reset the stream source informations.
   */
  void ResetSourceInformation();

 private:
  /**
   * @brief Register property accessor.
   * @param[in] (accessor) Property accessor.
   * @return Status object.
   */
  virtual Status RegisterPropertyAccessor(PropertyAccessor* accessor);

#ifdef SENSCORD_SERIALIZE
  /**
   * @brief Update the serialized property for frame channel.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Serialized property.
   * @param[in] (property_size) Serialized property size.
   * @return Status object.
   */
  virtual Status UpdateFrameSerializedProperty(uint32_t channel_id,
    const std::string& key, const void* property, size_t property_size);
#else
  /**
   * @brief Update frame channel property.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Property to updated.
   * @param[in] (factory) Property factory.
   * @return Status object.
   */
  virtual Status UpdateFramePropertyWithFactory(
      uint32_t channel_id, const std::string& key, const void* property,
      const PropertyFactoryBase& factory);
#endif  // SENSCORD_SERIALIZE

  /**
   * @brief Release the unused frames.
   * @param[in] (frames) Finished and unused frames.
   */
  void ReleaseFramesUnused(const std::vector<const FrameInfo*>& frames);

  /**
   * @brief Collect the all frames.
   * @param[in] (in) The base list of frames.
   * @param[out] (out) The collected list of frames.
   */
  void CollectAllFrames(
    const std::vector<FrameInfo>& in, std::vector<const FrameInfo*>* out) const;

  /**
   * @brief Remove all property accessors.
   */
  void RemovePropertyAll();

 private:
  Core* core_;
  ComponentPort* port_;
  const ComponentArgument& component_args_;  // entity is in ComponentAdapter.
  const ComponentPortArgument* port_args_;   // entity is in ComponentPortCore.
  StreamSource* source_;
  osal::OSThread* thread_;
  volatile bool is_started_;

  util::Mutex mutex_properties_;
  std::set<std::string> properties_;
  StreamSourceFunctionLockManager* lock_manager_;
};

}   // namespace senscord
#endif    // LIB_CORE_COMPONENT_STREAM_SOURCE_ADAPTER_H_
