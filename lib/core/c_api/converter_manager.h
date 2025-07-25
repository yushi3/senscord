/*
 * SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_C_API_CONVERTER_MANAGER_H_
#define LIB_CORE_C_API_CONVERTER_MANAGER_H_

#include <map>
#include <string>
#include <vector>

#include "senscord/noncopyable.h"
#include "senscord/develop/converter.h"
#include "c_api/c_config.h"
#include "c_api/converter_dynamic_loader.h"
#include "util/mutex.h"
#ifndef SENSCORD_SERIALIZE
#include "senscord/stream.h"
#include "senscord/frame.h"
#endif  // SENSCORD_SERIALIZE

namespace senscord {

/**
 * @brief Converter type.
 */
enum ConverterType {
  kConverterTypeProperty,
  kConverterTypeRawData,
};

/**
 * @brief Converter manager.
 */
class ConverterManager : private util::Noncopyable {
 public:
  /**
   * @brief Get singleton instance.
   * @return Instance of converter manager.
   */
  static ConverterManager* GetInstance();

  /**
   * @brief Initialize the converter.
   * @param[in] converters  Converter config.
   */
  void Init(const std::vector<c_api::ConverterConfig>& converters);

  /**
   * @brief Terminates the converter.
   */
  void Exit();

#ifdef SENSCORD_SERIALIZE
  /**
   * @brief Serialize the data.
   * @param[in]  type         Converter type.
   * @param[in]  key          Search key.
   * @param[in]  input_data   Pointer to the input data.
   * @param[in]  input_size   Size of the input data.
   * @param[out] output_data  Output destination container.
   * @return Status object.
   */
  Status Serialize(
      ConverterType type, const std::string& key,
      const void* input_data, size_t input_size,
      std::vector<uint8_t>* output_data);

  /**
   * @brief Deserialize the data.
   * @param[in]  type         Converter type.
   * @param[in]  key          Search key.
   * @param[in]  input_data   Pointer to the input data.
   * @param[in]  input_size   Size of the input data.
   * @param[out] output_data  Pointer to the output data.
   * @param[in]  output_size  Size of the output data.
   * @return Status object.
   */
  Status Deserialize(
      ConverterType type, const std::string& key,
      const void* input_data, size_t input_size,
      void* output_data, size_t output_size);
#else
  /**
   * @brief Gets the stream property.
   * @param[in]      stream  Stream.
   * @param[in]      key     Search key.
   * @param[in,out]  value   Pointer to the C property.
   * @param[in]      size    Size of the C property.
   * @return Status object.
   */
  Status GetStreamProperty(
      Stream* stream, const std::string& key,
      void* value, size_t size);

  /**
   * @brief Sets the stream property.
   * @param[in]  stream  Stream.
   * @param[in]  key     Search key.
   * @param[in]  value   Pointer to the C property.
   * @param[in]  size    Size of the C property.
   * @return Status object.
   */
  Status SetStreamProperty(
      Stream* stream, const std::string& key,
      const void* value, size_t size);

  /**
   * @brief Gets the channel property.
   * @param[in]   channel  Channel.
   * @param[in]   key      Search key.
   * @param[out]  value    Pointer to the C property.
   * @param[in]   size     Size of the C property.
   * @return Status object.
   */
  Status GetChannelProperty(
      Channel* channel, const std::string& key,
      void* value, size_t size);
#endif  // SENSCORD_SERIALIZE

 private:
  ConverterManager();
  ~ConverterManager();

  struct LibraryInstance {
    ConverterLibrary* library;
    std::string library_name;
  };

  /**
   * @brief Create a library instance.
   * @param[in]  config    Converter configuration.
   * @param[out] instance  Created library instance.
   * @return Status object.
   */
  Status CreateLibrary(
      const c_api::ConverterConfig& config, LibraryInstance* instance);

  /**
   * @brief Delete a library instance.
   * @param[in] instance  library instance.
   */
  void DeleteLibrary(const LibraryInstance& instance);

  /**
   * @brief Delete the library instances.
   */
  void DeleteLibraries();

  /**
   * @brief Get a converter.
   * @param[in] type  Converter type.
   * @param[in] key   Search key.
   */
  ConverterBase* GetConverter(
      ConverterType type, const std::string& key) const;

 private:
  util::Mutex mutex_;
  int32_t ref_count_;
  ConverterDynamicLoader loader_;

  // list of library instances.
  std::vector<LibraryInstance> libraries_;
  // Key: property key, Value: converter.
  std::map<std::string, ConverterBase*> property_list_;
#ifdef SENSCORD_SERIALIZE
  // Key: rawdata type, Value: converter.
  std::map<std::string, ConverterBase*> rawdata_list_;
#endif  // SENSCORD_SERIALIZE
};

}  // namespace senscord

#endif  // LIB_CORE_C_API_CONVERTER_MANAGER_H_
