/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stream/property_history_book.h"

#include <inttypes.h>
#include <utility>  // make_pair
#include <algorithm>  // copy

#include "util/autolock.h"
#include "logger/logger.h"
#include "senscord/osal_inttypes.h"

namespace senscord {

/**
 * @brief Constructor
 */
PropertyHistoryBook::PropertyHistoryBook() {
}

/**
 * @brief Destructor
 */
PropertyHistoryBook::~PropertyHistoryBook() {
  ClearAll();
}

#ifdef SENSCORD_SERIALIZE
/**
 * @brief Set the serialized property for frame channel.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (property) Serialized property.
 * @param[in] (size) Serialized property size.
 * @return Status object.
 */
Status PropertyHistoryBook::SetProperty(
    uint32_t channel_id,
    const std::string& key,
    const void* property,
    size_t size) {
  uint8_t* copied_property = NULL;
  if (size > 0) {
    copied_property = new uint8_t[size];
    osal::OSMemcpy(copied_property, size, property, size);
  }

  History history = {};
  history.data = copied_property;
  history.size = size;
  AddHistory(channel_id, key, history);
  return Status::OK();
}

/**
 * @brief Get the serialized property for frame channel with history ID.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (history_id) ID of property history.
 * @param[out] (property) Serialized property.
 * @param[out] (size) Serialized property size.
 * @return Status object.
 */
Status PropertyHistoryBook::GetProperty(
    uint32_t channel_id,
    const std::string& key,
    uint32_t history_id,
    void** property,
    size_t* size) {
  if ((property == NULL) || (size == NULL)) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  util::AutoLock lock(&mutex_);
  const History* history = GetHistory(channel_id, key, history_id);
  if (history == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "unknown history: ch=%" PRIu32 ", key=%s, id=%" PRIu32,
        channel_id, key.c_str(), history_id);
  }

  *property = history->data;
  *size = history->size;
  return Status::OK();
}
#else
/**
 * @brief Update or create the property.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (property) Property.
 * @param[in] (factory) Property factory.
 * @return Status object.
 */
Status PropertyHistoryBook::SetProperty(
    uint32_t channel_id,
    const std::string& key,
    const void* property,
    const PropertyFactoryBase& factory) {
  const PropertyFactoryBase* cloned_factory = factory.CloneFactory();
  void* cloned_property = cloned_factory->Create();
  if (property != NULL) {
    cloned_factory->Copy(property, cloned_property);
  }

  History history = {};
  history.data = cloned_property;
  history.factory = cloned_factory;
  AddHistory(channel_id, key, history);
  return Status::OK();
}

/**
 * @brief Get the property for frame channel with history ID.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (history_id) ID of property history.
 * @param[out] (property) Property.
 * @return Status object.
 */
Status PropertyHistoryBook::GetProperty(
    uint32_t channel_id,
    const std::string& key,
    uint32_t history_id,
    void* property) {
  if (property == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  util::AutoLock lock(&mutex_);
  const History* history = GetHistory(channel_id, key, history_id);
  if (history == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "unknown history: ch=%" PRIu32 ", key=%s, id=%" PRIu32,
        channel_id, key.c_str(), history_id);
  }

  history->factory->Copy(history->data, property);
  return Status::OK();
}
#endif  // SENSCORD_SERIALIZE

/**
 * @brief Add the latest property history.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to updated.
 * @param[in] (history) Property history.
 */
void PropertyHistoryBook::AddHistory(
    uint32_t channel_id,
    const std::string& key,
    const History& history) {
  util::AutoLock lock(&mutex_);

  std::pair<ChannelProperties::iterator, bool> ret =
      channels_.insert(ChannelProperties::value_type(channel_id, NULL));
  if (ret.second) {
    // Add new channel histories.
    ret.first->second = new ChannelProperty();
  }
  ChannelProperty* channel = ret.first->second;

  PropertyHistories* histories = NULL;
  std::pair<std::map<std::string, PropertyHistories*>::iterator, bool> ret2 =
      channel->properties.insert(
          std::map<std::string, PropertyHistories*>::value_type(key, NULL));
  if (ret2.second) {
    // Add new key's history.
    ret2.first->second = new PropertyHistories();
    histories = ret2.first->second;
    histories->current_history_id = 1;
  } else {
    // Update new history.
    histories = ret2.first->second;
    // from latest history to old history: reference count '-1'
    ReleaseHistory(histories, key, histories->current_history_id);
    ++histories->current_history_id;
  }

  History& new_history = histories->histories[histories->current_history_id];
  new_history = history;
  // latest history: reference count '+1'
  new_history.referenced = 1;

  SENSCORD_LOG_DEBUG(
      "reference(add): %" PRIu32 ", key=%s, history_id=%" PRIu32,
      new_history.referenced, key.c_str(), histories->current_history_id);
}

/**
 * @brief Get the current property keys and history IDs.
 * @param[in] (channel_id) Target channel ID.
 * @param[out] (current_properties) Current property informations.
 */
void PropertyHistoryBook::ReferenceCurrentProperties(
    uint32_t channel_id,
    std::map<std::string, uint32_t>* current_properties) {
  util::AutoLock lock(&mutex_);
  ChannelProperties::const_iterator channel_itr = channels_.find(channel_id);
  if (channel_itr != channels_.end()) {
    // get the all pairs of property key and history ID.
    current_properties->clear();

    const ChannelProperty* channel = channel_itr->second;
    for (std::map<std::string, PropertyHistories*>::const_iterator
        itr = channel->properties.begin(), end = channel->properties.end();
        itr != end; ++itr) {
      PropertyHistories* histories = itr->second;

      // add return map and increase reference count.
      current_properties->insert(
          std::make_pair(itr->first, histories->current_history_id));
      History& history = histories->histories[histories->current_history_id];
      ++history.referenced;

      SENSCORD_LOG_DEBUG(
          "reference(++): %" PRIu32 ", key=%s, history_id=%" PRIu32,
          history.referenced,
          itr->first.c_str(), histories->current_history_id);
    }
  }
}

/**
 * @brief Release the property keys and history IDs from reference.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (properties) Property keys and history IDs to release.
 */
void PropertyHistoryBook::ReleaseProperties(
    uint32_t channel_id,
    const std::map<std::string, uint32_t>& properties) {
  for (std::map<std::string, uint32_t>::const_iterator
      itr = properties.begin(), end = properties.end(); itr != end; ++itr) {
    PropertyHistories* histories = GetKeyHistories(channel_id, itr->first);
    if (histories != NULL) {
      ReleaseHistory(histories, itr->first, itr->second);
    }
  }
}

/**
 * @brief Release the property history.
 *
 * Decrease the reference count and release the history
 * when the count reaches zero.
 * @param[in,out] (histories) Histories of key.
 * @param[in] (key) Property key.
 * @param[in] (history_id) ID of property history to release.
 */
void PropertyHistoryBook::ReleaseHistory(
    PropertyHistories* histories,
    const std::string& key, uint32_t history_id) {
  util::AutoLock lock(&mutex_);
  std::map<uint32_t, History>::iterator history_itr =
      histories->histories.find(history_id);
  if (history_itr != histories->histories.end()) {
    History& history = history_itr->second;
    --history.referenced;
    SENSCORD_LOG_DEBUG(
        "reference(%s): %" PRIu32 ", key=%s, history_id=%" PRIu32,
        (history.referenced == 0) ? "del" : "--",
        history.referenced, key.c_str(), history_id);
    if (history.referenced == 0) {
      ClearHistory(&history);
      histories->histories.erase(history_itr);
    }
  }
}

/**
 * @brief Get the property history.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key to get.
 * @param[in] (history_id) Property history ID.
 * @return Pointer of property history.
 */
const PropertyHistoryBook::History* PropertyHistoryBook::GetHistory(
    uint32_t channel_id, const std::string& key, uint32_t history_id) const {
  const History* history = NULL;
  util::AutoLock lock(&mutex_);
  PropertyHistories* histories = GetKeyHistories(channel_id, key);
  if (histories != NULL) {
    std::map<uint32_t, History>::const_iterator history_itr =
        histories->histories.find(history_id);
    if (history_itr != histories->histories.end()) {
      history = &history_itr->second;
    }
  }
  return history;
}

/**
 * @brief Get histories of key.
 * @param[in] (channel_id) Target channel ID.
 * @param[in] (key) Property key.
 * @return Histories of key.
 */
PropertyHistoryBook::PropertyHistories* PropertyHistoryBook::GetKeyHistories(
    uint32_t channel_id, const std::string& key) const {
  PropertyHistories* histories = NULL;
  util::AutoLock lock(&mutex_);
  ChannelProperties::const_iterator channel_itr = channels_.find(channel_id);
  if (channel_itr != channels_.end()) {
    const ChannelProperty* channel = channel_itr->second;
    std::map<std::string, PropertyHistories*>::const_iterator property_itr =
        channel->properties.find(key);
    if (property_itr != channel->properties.end()) {
      histories = property_itr->second;
    }
  }
  return histories;
}

/**
 * @brief Clear history of key.
 * @param[in] (history) History to clear.
 */
void PropertyHistoryBook::ClearHistory(History* history) {
  if (history != NULL) {
#ifdef SENSCORD_SERIALIZE
    if (history->data != NULL) {
      delete [] reinterpret_cast<uint8_t*>(history->data);
    }
    history->data = NULL;
    history->size = 0;
    history->referenced = 0;
#else
    if (history->factory != NULL) {
      history->factory->Delete(history->data);
      history->data = NULL;
      delete history->factory;
      history->factory = NULL;
    }
    history->referenced = 0;
#endif  // SENSCORD_SERIALIZE
  }
}

/**
 * @brief Clear all histories.
 */
void PropertyHistoryBook::ClearAll() {
  util::AutoLock lock(&mutex_);
  ChannelProperties::const_iterator channel_itr = channels_.begin();
  ChannelProperties::const_iterator channel_end = channels_.end();
  for (; channel_itr != channel_end; ++channel_itr) {
    ChannelProperty* channel = channel_itr->second;
    std::map<std::string, PropertyHistories*>::const_iterator property_itr =
        channel->properties.begin();
    std::map<std::string, PropertyHistories*>::const_iterator property_end =
        channel->properties.end();
    for (; property_itr != property_end; ++property_itr) {
      PropertyHistories* histories = property_itr->second;
      std::map<uint32_t, History>::iterator history_itr =
          histories->histories.begin();
      std::map<uint32_t, History>::iterator history_end =
          histories->histories.end();
      for (; history_itr != history_end; ++history_itr) {
        ClearHistory(&history_itr->second);
      }
      histories->histories.clear();
      delete histories;
    }
    channel->properties.clear();
    delete channel;
  }
  channels_.clear();
}

/**
 * @brief Get the list of updated property keys.
 * @param[in] (stream) Referenced stream.
 * @param[in] (channel_id) Referenced channel ID.
 * @param[in] (properties) Property keys and history IDs.
 * @param[out] (updated_list) Updated property key list.
 * @return Status object.
 */
Status PropertyHistoryBook::GetUpdatedPropertyList(
    const Stream* stream,
    uint32_t channel_id,
    const std::map<std::string, uint32_t>& properties,
    std::vector<std::string>* updated_list) {
  if ((stream == NULL) || (updated_list == NULL)) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  updated_list->clear();

  util::AutoLock lock(&mutex_);
  ChannelProperties::const_iterator ch_itr = channels_.find(channel_id);
  if (ch_itr != channels_.end()) {
    ChannelProperty* channel = ch_itr->second;
    for (std::map<std::string, PropertyHistories*>::const_iterator
        itr = channel->properties.begin(), end = channel->properties.end();
        itr != end; ++itr) {
      PropertyHistories* histories = itr->second;
      std::pair<std::map<const Stream*, uint32_t>::iterator, bool> ret =
          histories->last_access_id.insert(std::make_pair(
              stream, histories->current_history_id));
      if (ret.second) {
        // first access (insert success)
        updated_list->push_back(itr->first);
      } else if (ret.first->second < histories->current_history_id) {
        // last access is not latest
        updated_list->push_back(itr->first);
        ret.first->second = histories->current_history_id;
      }
    }
  }
  return Status::OK();
}

/**
 * @brief Remove the updated property list associated with Stream.
 * @param[in] (stream) Stream to remove.
 */
void PropertyHistoryBook::RemoveUpdatedPropertyList(const Stream* stream) {
  util::AutoLock lock(&mutex_);
  for (ChannelProperties::const_iterator ch_itr = channels_.begin(),
      ch_end = channels_.end(); ch_itr != ch_end; ++ch_itr) {
    ChannelProperty* channel = ch_itr->second;
    for (std::map<std::string, PropertyHistories*>::const_iterator
        itr = channel->properties.begin(), end = channel->properties.end();
        itr != end; ++itr) {
      PropertyHistories* histories = itr->second;
      histories->last_access_id.erase(stream);
    }
  }
}

}   // namespace senscord
