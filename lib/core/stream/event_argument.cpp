/*
 * SPDX-FileCopyrightText: 2020-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/config.h"

#ifdef SENSCORD_STREAM_EVENT_ARGUMENT

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

#include "senscord/event_argument.h"
#include "senscord/serialize.h"

namespace {

/**
 * @brief Serialize and set the value.
 * @param[in] key  Argument key.
 * @param[in] value  Value to set.
 * @param[out] args  Location to store the key and value.
 * @return Status object.
 */
template<typename T>
senscord::Status Serialize(
    const std::string& key, const T& value,
    std::map<std::string, std::vector<uint8_t> >* args) {
  if (key.empty()) {
    return SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
        senscord::Status::kCauseInvalidArgument,
        "empty argument key is invalid.");
  }
  senscord::serialize::SerializedBuffer buffer;
  senscord::serialize::Encoder encoder(&buffer);
  senscord::Status status = encoder.Push(value);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  const uint8_t* ptr = buffer.data();
  (*args)[key] = std::vector<uint8_t>(ptr, ptr + buffer.size());
  return senscord::Status::OK();
}

/**
 * @brief Deserialize and get the value.
 * @param[in] args  List of keys and values.
 * @param[in] key  Argument key.
 * @param[out] value  Location to store the value.
 * @return Status object.
 */
template<typename T>
senscord::Status Deserialize(
    const std::map<std::string, std::vector<uint8_t> >& args,
    const std::string& key, T* value) {
  if (value == NULL) {
    return SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
        senscord::Status::kCauseInvalidArgument, "value is NULL.");
  }
  std::map<std::string, std::vector<uint8_t> >::const_iterator itr =
      args.find(key);
  if (itr == args.end()) {
    return SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
        senscord::Status::kCauseNotFound, "The specified key was not found.");
  }
  const std::vector<uint8_t>& buffer = itr->second;
  senscord::serialize::Decoder decoder(buffer.data(), buffer.size());
  senscord::Status status = decoder.Pop(*value);
  return SENSCORD_STATUS_TRACE(status);
}

}  // namespace

namespace senscord {

/**
 * @brief Constructor.
 */
EventArgument::EventArgument() : args_() {
}

/**
 * @brief Destructor.
 */
EventArgument::~EventArgument() {
}

/**
 * @brief Sets the value for the specified key. (uint8_t)
 */
Status EventArgument::Set(const std::string& key, const uint8_t& value) {
  return Serialize(key, value, &args_);
}

/**
 * @brief Gets the value for the specified key. (uint8_t)
 */
Status EventArgument::Get(const std::string& key, uint8_t* value) const {
  return Deserialize(args_, key, value);
}

/**
 * @brief Sets the value for the specified key. (uint16_t)
 */
Status EventArgument::Set(const std::string& key, const uint16_t& value) {
  return Serialize(key, value, &args_);
}

/**
 * @brief Gets the value for the specified key. (uint16_t)
 */
Status EventArgument::Get(const std::string& key, uint16_t* value) const {
  return Deserialize(args_, key, value);
}

/**
 * @brief Sets the value for the specified key. (uint32_t)
 */
Status EventArgument::Set(const std::string& key, const uint32_t& value) {
  return Serialize(key, value, &args_);
}

/**
 * @brief Gets the value for the specified key. (uint32_t)
 */
Status EventArgument::Get(const std::string& key, uint32_t* value) const {
  return Deserialize(args_, key, value);
}

/**
 * @brief Sets the value for the specified key. (uint64_t)
 */
Status EventArgument::Set(const std::string& key, const uint64_t& value) {
  return Serialize(key, value, &args_);
}

/**
 * @brief Gets the value for the specified key. (uint64_t)
 */
Status EventArgument::Get(const std::string& key, uint64_t* value) const {
  return Deserialize(args_, key, value);
}

/**
 * @brief Sets the value for the specified key. (int8_t)
 */
Status EventArgument::Set(const std::string& key, const int8_t& value) {
  return Serialize(key, value, &args_);
}

/**
 * @brief Gets the value for the specified key. (int8_t)
 */
Status EventArgument::Get(const std::string& key, int8_t* value) const {
  return Deserialize(args_, key, value);
}

/**
 * @brief Sets the value for the specified key. (int16_t)
 */
Status EventArgument::Set(const std::string& key, const int16_t& value) {
  return Serialize(key, value, &args_);
}

/**
 * @brief Gets the value for the specified key. (int16_t)
 */
Status EventArgument::Get(const std::string& key, int16_t* value) const {
  return Deserialize(args_, key, value);
}

/**
 * @brief Sets the value for the specified key. (int32_t)
 */
Status EventArgument::Set(const std::string& key, const int32_t& value) {
  return Serialize(key, value, &args_);
}

/**
 * @brief Gets the value for the specified key. (int32_t)
 */
Status EventArgument::Get(const std::string& key, int32_t* value) const {
  return Deserialize(args_, key, value);
}

/**
 * @brief Sets the value for the specified key. (int64_t)
 */
Status EventArgument::Set(const std::string& key, const int64_t& value) {
  return Serialize(key, value, &args_);
}

/**
 * @brief Gets the value for the specified key. (int64_t)
 */
Status EventArgument::Get(const std::string& key, int64_t* value) const {
  return Deserialize(args_, key, value);
}

/**
 * @brief Sets the value for the specified key. (float)
 */
Status EventArgument::Set(const std::string& key, const float& value) {
  return Serialize(key, value, &args_);
}

/**
 * @brief Gets the value for the specified key. (float)
 */
Status EventArgument::Get(const std::string& key, float* value) const {
  return Deserialize(args_, key, value);
}

/**
 * @brief Sets the value for the specified key. (double)
 */
Status EventArgument::Set(const std::string& key, const double& value) {
  return Serialize(key, value, &args_);
}

/**
 * @brief Gets the value for the specified key. (double)
 */
Status EventArgument::Get(const std::string& key, double* value) const {
  return Deserialize(args_, key, value);
}

/**
 * @brief Sets the value for the specified key. (string)
 */
Status EventArgument::Set(const std::string& key, const std::string& value) {
  return Serialize(key, value, &args_);
}

/**
 * @brief Gets the value for the specified key. (string)
 */
Status EventArgument::Get(const std::string& key, std::string* value) const {
  return Deserialize(args_, key, value);
}

/**
 * @brief Sets the value for the specified key. (vector<uint8_t>)
 */
Status EventArgument::Set(
    const std::string& key, const std::vector<uint8_t>& value) {
  return Serialize(key, value, &args_);
}

/**
 * @brief Gets the value for the specified key. (vector<uint8_t>)
 */
Status EventArgument::Get(
    const std::string& key, std::vector<uint8_t>* value) const {
  return Deserialize(args_, key, value);
}


/**
 * @brief Gets the serialized binary for the specified key.
 * @param[in] key  Argument key.
 * @return Serialized binary. Returns NULL if key is not found.
 */
const std::vector<uint8_t>* EventArgument::GetSerializedBinary(
    const std::string& key) const {
  std::map<std::string, std::vector<uint8_t> >::const_iterator itr =
      args_.find(key);
  if (itr == args_.end()) {
    return NULL;
  }
  return &itr->second;
}

/**
 * @brief Remove the value for the specified key.
 * @param[in] key  Argument key.
 * @return Status object.
 */
Status EventArgument::Remove(const std::string& key) {
  std::map<std::string, std::vector<uint8_t> >::iterator itr = args_.find(key);
  if (itr == args_.end()) {
    return SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
        senscord::Status::kCauseNotFound, "The specified key was not found.");
  }
  args_.erase(itr);
  return Status::OK();
}

/**
 * @brief Returns true if the argument list is empty.
 */
bool EventArgument::Empty() const {
  return args_.empty();
}

/**
 * @brief Returns true if the argument list contains the specified key.
 * @param[in] key  Argument key.
 */
bool EventArgument::Contains(const std::string& key) const {
  return args_.find(key) != args_.end();
}

/**
 * @brief Returns the size of the argument list.
 */
size_t EventArgument::GetSize() const {
  return args_.size();
}

/**
 * @brief Get a list of argument keys.
 */
std::vector<std::string> EventArgument::GetKeys() const {
  std::vector<std::string> value;
  for (std::map<std::string, std::vector<uint8_t> >::const_iterator
      itr = args_.begin(), end = args_.end(); itr != end; ++itr) {
    value.push_back(itr->first);
  }
  return value;
}

/**
 * @brief Gets the argument key at the specified index.
 * @param[in] index  Index of argument list. (0 to elements-1)
 * @return Argument key. Returns an empty string if invalid.
 */
const std::string& EventArgument::GetKey(size_t index) const {
  if (index >= args_.size()) {
    static std::string empty_string;
    return empty_string;
  }
  std::map<std::string, std::vector<uint8_t> >::const_iterator itr =
      args_.begin();
  std::advance(itr, index);
  return itr->first;
}

}  // namespace senscord

#endif  // SENSCORD_STREAM_EVENT_ARGUMENT
