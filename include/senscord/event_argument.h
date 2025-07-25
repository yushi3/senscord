/*
 * SPDX-FileCopyrightText: 2020-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_EVENT_ARGUMENT_H_
#define SENSCORD_EVENT_ARGUMENT_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

#include "senscord/config.h"
#include "senscord/status.h"
#include "senscord/serialize.h"

namespace senscord {

#ifdef SENSCORD_STREAM_EVENT_ARGUMENT

/**
 * @brief Class that represents the argument of the event.
 *
 * Arguments consist of an associative array of keys and values.
 * Supported type of the value is as follows:
 *  - int8_t, int16_t, int32_t, int64_t
 *  - uint8_t, uint16_t, uint32_t, uint64_t
 *  - float, double
 *  - std::string
 *  - std::vector<uint8_t>
 */
class EventArgument {
 public:
  /**
   * @brief Constructor.
   */
  EventArgument();

  /**
   * @brief Destructor.
   */
  ~EventArgument();

  /**
   * @brief Sets the value for the specified key.
   * @param[in] key  Argument key.
   * @param[in] value  Value to set.
   * @return Status object.
   */
  Status Set(const std::string& key, const uint8_t& value);
  Status Set(const std::string& key, const uint16_t& value);
  Status Set(const std::string& key, const uint32_t& value);
  Status Set(const std::string& key, const uint64_t& value);
  Status Set(const std::string& key, const int8_t& value);
  Status Set(const std::string& key, const int16_t& value);
  Status Set(const std::string& key, const int32_t& value);
  Status Set(const std::string& key, const int64_t& value);
  Status Set(const std::string& key, const float& value);
  Status Set(const std::string& key, const double& value);
  Status Set(const std::string& key, const std::string& value);
  Status Set(const std::string& key, const std::vector<uint8_t>& value);

  /**
   * @brief Gets the value for the specified key.
   * @param[in] key  Argument key.
   * @param[out] value  Location to store the value.
   * @return Status object.
   */
  Status Get(const std::string& key, uint8_t* value) const;
  Status Get(const std::string& key, uint16_t* value) const;
  Status Get(const std::string& key, uint32_t* value) const;
  Status Get(const std::string& key, uint64_t* value) const;
  Status Get(const std::string& key, int8_t* value) const;
  Status Get(const std::string& key, int16_t* value) const;
  Status Get(const std::string& key, int32_t* value) const;
  Status Get(const std::string& key, int64_t* value) const;
  Status Get(const std::string& key, float* value) const;
  Status Get(const std::string& key, double* value) const;
  Status Get(const std::string& key, std::string* value) const;
  Status Get(const std::string& key, std::vector<uint8_t>* value) const;

  /**
   * @brief Gets the serialized binary for the specified key.
   * @param[in] key  Argument key.
   * @return Serialized binary. Returns NULL if key is not found.
   */
  const std::vector<uint8_t>* GetSerializedBinary(
      const std::string& key) const;

  /**
   * @brief Remove the value for the specified key.
   * @param[in] key  Argument key.
   * @return Status object.
   */
  Status Remove(const std::string& key);

  /**
   * @brief Returns true if the argument list is empty.
   */
  bool Empty() const;

  /**
   * @brief Returns true if the argument list contains the specified key.
   * @param[in] key  Argument key.
   */
  bool Contains(const std::string& key) const;

  /**
   * @brief Returns the size of the argument list.
   */
  size_t GetSize() const;

  /**
   * @brief Get a list of argument keys.
   */
  std::vector<std::string> GetKeys() const;

  /**
   * @brief Gets the argument key at the specified index.
   * @param[in] index  Index of argument list. (0 to elements-1)
   * @return Argument key. Returns an empty string if invalid.
   */
  const std::string& GetKey(size_t index) const;

 public:
  SENSCORD_SERIALIZE_DEFINE(args_)

 private:
  std::map<std::string, std::vector<uint8_t> > args_;
};

#else

class EventArgument {
 public:
  template<typename T>
  Status Set(const std::string& /*key*/, const T& /*value*/) {
    return Status::OK();
  }

  template<typename T>
  Status Get(const std::string& /*key*/, T* /*value*/) const {
    return SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
        senscord::Status::kCauseNotFound, "The specified key was not found.");
  }

  const std::vector<uint8_t>* GetSerializedBinary(
      const std::string& /*key*/) const {
    return NULL;
  }

  Status Remove(const std::string& /*key*/) {
    return SENSCORD_STATUS_FAIL(senscord::kStatusBlockCore,
        senscord::Status::kCauseNotFound, "The specified key was not found.");
  }

  bool Empty() const { return true; }
  bool Contains(const std::string& /*key*/) const { return false; }
  size_t GetSize() const { return 0; }

  std::vector<std::string> GetKeys() const {
    return std::vector<std::string>();
  }

  const std::string& GetKey(size_t /*index*/) const {
    static std::string empty_string;
    return empty_string;
  }
};

#endif  // SENSCORD_STREAM_EVENT_ARGUMENT

}  // namespace senscord

#endif  // SENSCORD_EVENT_ARGUMENT_H_
