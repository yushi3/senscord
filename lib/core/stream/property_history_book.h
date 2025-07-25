/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_STREAM_PROPERTY_HISTORY_BOOK_H_
#define LIB_CORE_STREAM_PROPERTY_HISTORY_BOOK_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "senscord/status.h"
#include "senscord/stream.h"
#include "util/mutex.h"
#ifndef SENSCORD_SERIALIZE
#include "senscord/develop/property_factory.h"
#endif  // SENSCORD_SERIALIZE

namespace senscord {

/**
 * @brief Frame property history book for each stream.
 */
class PropertyHistoryBook {
 public:
#ifdef SENSCORD_SERIALIZE
  /**
   * @brief Update or create the serialized property.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Serialized property.
   * @param[in] (size) Serialized property size.
   * @return Status object.
   */
  Status SetProperty(
    uint32_t channel_id,
    const std::string& key,
    const void* property,
    size_t size);

  /**
   * @brief Get the serialized property for frame channel with history ID.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (history_id) ID of property history.
   * @param[out] (property) Serialized property.
   * @param[out] (size) Serialized property size.
   * @return Status object.
   */
  Status GetProperty(
    uint32_t channel_id,
    const std::string& key,
    uint32_t history_id,
    void** property,
    size_t* size);
#else
  /**
   * @brief Update or create the property.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Property.
   * @param[in] (factory) Property factory.
   * @return Status object.
   */
  Status SetProperty(
      uint32_t channel_id,
      const std::string& key,
      const void* property,
      const PropertyFactoryBase& factory);

  /**
   * @brief Get the property for frame channel with history ID.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (history_id) ID of property history.
   * @param[out] (property) Property.
   * @return Status object.
   */
  Status GetProperty(
      uint32_t channel_id,
      const std::string& key,
      uint32_t history_id,
      void* property);
#endif  // SENSCORD_SERIALIZE

  /**
   * @brief Get the current property keys and history IDs.
   * @param[in] (channel_id) Target channel ID.
   * @param[out] (current_properties) Current property informations.
   */
  void ReferenceCurrentProperties(
    uint32_t channel_id,
    std::map<std::string, uint32_t>* current_properties);

  /**
   * @brief Release the property keys and history IDs from reference.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (properties) Property keys and history IDs to release.
   */
  void ReleaseProperties(
    uint32_t channel_id,
    const std::map<std::string, uint32_t>& properties);

  /**
   * @brief Get the list of updated property keys.
   * @param[in] (stream) Referenced stream.
   * @param[in] (channel_id) Referenced channel ID.
   * @param[in] (properties) Property keys and history IDs.
   * @param[out] (updated_list) Updated property key list.
   * @return Status object.
   */
  Status GetUpdatedPropertyList(
      const Stream* stream,
      uint32_t channel_id,
      const std::map<std::string, uint32_t>& properties,
      std::vector<std::string>* updated_list);

  /**
   * @brief Remove the updated property list associated with Stream.
   * @param[in] (stream) Stream to remove.
   */
  void RemoveUpdatedPropertyList(const Stream* stream);

  /**
   * @brief Constructor
   */
  PropertyHistoryBook();

  /**
   * @brief Destructor
   */
  ~PropertyHistoryBook();

 private:
  /**
   * @brief Property history.
   */
  struct History {
    /** Reference count. */
    uint32_t referenced;

    /** Address */
    void* data;

#ifdef SENSCORD_SERIALIZE
    /** Serialized size */
    size_t size;
#else
    const PropertyFactoryBase* factory;  // for create & delete.
#endif  // SENSCORD_SERIALIZE
  };

  /**
   * @brief Property histories of each key.
   */
  struct PropertyHistories {
    /** Current active history ID */
    uint32_t current_history_id;

    /** Histories on each history IDs */
    std::map<uint32_t, History> histories;

    /** History ID that Stream last accessed (for UpdatedPropertyList) */
    std::map<const Stream*, uint32_t> last_access_id;
  };

  /**
   * @brief Channel property histories.
   */
  struct ChannelProperty {
    /** Histories of each key. */
    std::map<std::string, PropertyHistories*> properties;
  };

  /**
   * @brief Add the latest property history.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to add.
   * @param[in] (history) Property history.
   */
  void AddHistory(
      uint32_t channel_id,
      const std::string& key,
      const History& history);

  /**
   * @brief Get the property history.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to get.
   * @param[in] (history_id) Property history ID.
   * @return Pointer of property history.
   */
  const History* GetHistory(
      uint32_t channel_id,
      const std::string& key,
      uint32_t history_id) const;

  /**
   * @brief Get histories of key.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key.
   * @return Histories of key.
   */
  PropertyHistories* GetKeyHistories(
    uint32_t channel_id,
    const std::string& key) const;

  /**
   * @brief Release the property history.
   *
   * Decrease the reference count and release the history
   * when the count reaches zero.
   * @param[in,out] (histories) Histories of key.
   * @param[in] (key) Property key.
   * @param[in] (history_id) ID of property history to release.
   */
  void ReleaseHistory(
    PropertyHistories* histories,
    const std::string& key,
    uint32_t history_id);

  /**
   * @brief Clear history of key.
   * @param[in] (history) History to clear.
   */
  void ClearHistory(History* history);

  /**
   * @brief Clear all histories.
   */
  void ClearAll();

  // histories of all channels on each channel ID.
  typedef std::map<uint32_t, ChannelProperty*> ChannelProperties;
  ChannelProperties channels_;
  mutable util::Mutex mutex_;
};

}   // namespace senscord
#endif  // LIB_CORE_STREAM_PROPERTY_HISTORY_BOOK_H_
