/*
 * SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "c_api/converter_manager.h"

#include <map>
#include <string>
#include <utility>

#include "logger/logger.h"
#include "core/internal_types.h"
#include "util/autolock.h"
#include "util/senscord_utils.h"
#include "util/singleton.h"
#include "c_api/c_config_reader.h"

namespace {

/**
 * @brief Implementation of converter collector.
 */
class ConverterCollectorImpl : public senscord::ConverterCollector {
 public:
  /**
   * @brief Add converter.
   * @param[in] key        Search key.
   * @param[in] converter  Converter.
   */
  virtual void Add(
      const std::string& key, senscord::ConverterBase* converter) {
    if (list_.find(key) == list_.end()) {
      list_.insert(std::make_pair(key, converter));
      SENSCORD_LOG_DEBUG(
          "Collector.Add: key=%s, ptr=%p", key.c_str(), converter);
    } else {
      SENSCORD_LOG_WARNING(
          "Collector.Add: Already registered: key=%s", key.c_str());
    }
  }

  /**
   * @brief Get the list.
   */
  const std::map<std::string, senscord::ConverterBase*>& GetList() const {
    return list_;
  }

 private:
  std::map<std::string, senscord::ConverterBase*> list_;
};

}  // namespace

namespace senscord {

/**
 * @brief Get singleton instance.
 * @return Instance of converter manager.
 */
ConverterManager* ConverterManager::GetInstance() {
  // for private constructor / destructor
  struct InnerConverterManager : public ConverterManager {
  };
  return util::Singleton<InnerConverterManager>::GetInstance();
}

/**
 * @brief Constructor.
 */
ConverterManager::ConverterManager() : ref_count_() {
}

/**
 * @brief Destructor.
 */
ConverterManager::~ConverterManager() {
  {
    util::AutoLock autolock(&mutex_);
    DeleteLibraries();
  }
}

/**
 * @brief Initialize the converter.
 * @param[in] converters  Converter config.
 */
void ConverterManager::Init(
    const std::vector<c_api::ConverterConfig>& converters) {
  util::AutoLock autolock(&mutex_);
  if (ref_count_ == 0) {
    Status status;

    for (std::vector<c_api::ConverterConfig>::const_iterator
        itr = converters.begin(), end = converters.end(); itr != end; ++itr) {
      LibraryInstance instance = {};
      status = CreateLibrary(*itr, &instance);
      if (!status.ok()) {
        SENSCORD_LOG_WARNING(
            "CreateLibrary(%s): status=%s",
            itr->library_name.c_str(), status.ToString().c_str());
        continue;
      }

      ConverterCollectorImpl collector;
      status = instance.library->Init(&collector);
      if (!status.ok()) {
        DeleteLibrary(instance);
        SENSCORD_LOG_WARNING(
            "Library.Init(%s): status=%s",
            itr->library_name.c_str(), status.ToString().c_str());
        continue;
      }

      const std::map<std::string, ConverterBase*>& list = collector.GetList();
      uint32_t count = 0;
      for (std::map<std::string, ConverterBase*>::const_iterator
          itr2 = list.begin(), end2 = list.end(); itr2 != end2; ++itr2) {
        // property
        if (itr->enable_property) {
          if (property_list_.find(itr2->first) == property_list_.end()) {
            property_list_.insert(std::make_pair(itr2->first, itr2->second));
            ++count;
          } else {
            SENSCORD_LOG_WARNING(
                "Manager.Init: Property already registered: name=%s, key=%s",
                itr->library_name.c_str(), itr2->first.c_str());
          }
        }
        // rawdata
        if (itr->enable_rawdata) {
#ifdef SENSCORD_SERIALIZE
          if (rawdata_list_.find(itr2->first) == rawdata_list_.end()) {
            rawdata_list_.insert(std::make_pair(itr2->first, itr2->second));
            ++count;
          } else {
            SENSCORD_LOG_WARNING(
                "Manager.Init: RawData already registered: name=%s, key=%s",
                itr->library_name.c_str(), itr2->first.c_str());
          }
#endif  // SENSCORD_SERIALIZE
        }
      }
      if (count != 0) {
        libraries_.push_back(instance);
      } else {
        DeleteLibrary(instance);
      }
    }
  }

  ++ref_count_;
}

/**
 * @brief Terminates the converter.
 */
void ConverterManager::Exit() {
  util::AutoLock autolock(&mutex_);
  if (ref_count_ > 0) {
    --ref_count_;
  }
  if (ref_count_ == 0) {
    DeleteLibraries();
  }
}

/**
 * @brief Create a library instance.
 * @param[in]  config    Converter configuration.
 * @param[out] instance  Created library instance.
 * @return Status object.
 */
Status ConverterManager::CreateLibrary(
    const c_api::ConverterConfig& config, LibraryInstance* instance) {
  instance->library_name = config.library_name;
  Status status = loader_.Create(config.library_name, &instance->library);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  return Status::OK();
}

/**
 * @brief Delete a library instance.
 * @param[in] instance  library instance.
 */
void ConverterManager::DeleteLibrary(const LibraryInstance& instance) {
  Status status = loader_.Destroy(
      instance.library_name, instance.library);
  if (!status.ok()) {
    SENSCORD_LOG_ERROR("failed to destroy converter : ret=%s",
                       status.ToString().c_str());
  }
}

/**
 * @brief Delete the library instances.
 */
void ConverterManager::DeleteLibraries() {
  while (!libraries_.empty()) {
    const LibraryInstance& library = libraries_.back();
    DeleteLibrary(library);
    libraries_.pop_back();
  }
  property_list_.clear();
#ifdef SENSCORD_SERIALIZE
  rawdata_list_.clear();
#endif  // SENSCORD_SERIALIZE
}

/**
 * @brief Get a converter.
 * @param[in] type  Converter type.
 * @param[in] key   Search key.
 */
ConverterBase* ConverterManager::GetConverter(
    ConverterType type, const std::string& key) const {
#ifdef SENSCORD_SERIALIZE
  const std::map<std::string, ConverterBase*>& list =
      (type == kConverterTypeRawData) ? rawdata_list_ : property_list_;
#else
  const std::map<std::string, ConverterBase*>& list = property_list_;
#endif  // SENSCORD_SERIALIZE
  std::map<std::string, ConverterBase*>::const_iterator itr =
      list.find(PropertyUtils::GetKey(key));
  if (itr != list.end()) {
    return itr->second;
  }
  return NULL;
}

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
Status ConverterManager::Serialize(
    ConverterType type, const std::string& key,
    const void* input_data, size_t input_size,
    std::vector<uint8_t>* output_data) {
  util::AutoLock autolock(&mutex_);
  ConverterBase* converter = GetConverter(type, key);
  if (converter == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "unmanaged key=%s", key.c_str());
  }
  Status status = converter->Serialize(input_data, input_size, output_data);
  return SENSCORD_STATUS_TRACE(status);
}

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
Status ConverterManager::Deserialize(
    ConverterType type, const std::string& key,
    const void* input_data, size_t input_size,
    void* output_data, size_t output_size) {
  util::AutoLock autolock(&mutex_);
  ConverterBase* converter = GetConverter(type, key);
  if (converter == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "unmanaged key=%s", key.c_str());
  }
  Status status = converter->Deserialize(
      input_data, input_size, output_data, output_size);
  return SENSCORD_STATUS_TRACE(status);
}
#else
/**
 * @brief Gets the stream property.
 * @param[in]      stream  Stream.
 * @param[in]      key     Search key.
 * @param[in,out]  value   Pointer to the C property.
 * @param[in]      size    Size of the C property.
 * @return Status object.
 */
Status ConverterManager::GetStreamProperty(
    Stream* stream, const std::string& key,
    void* value, size_t size) {
  util::AutoLock autolock(&mutex_);
  ConverterBase* converter = GetConverter(kConverterTypeProperty, key);
  if (converter == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "unmanaged key=%s", key.c_str());
  }

  void* cxx_property = NULL;
  Status status = converter->CreateCxxProperty(value, size, &cxx_property);
  SENSCORD_STATUS_TRACE(status);

  if (status.ok()) {
    status = stream->GetProperty(key, cxx_property);
    SENSCORD_STATUS_TRACE(status);
  }

  if (status.ok()) {
    status = converter->ConvertProperty(cxx_property, value, size);
    SENSCORD_STATUS_TRACE(status);
  }

  converter->DeleteCxxProperty(value, size, cxx_property);

  return status;
}

/**
 * @brief Sets the stream property.
 * @param[in]  stream  Stream.
 * @param[in]  key     Search key.
 * @param[in]  value   Pointer to the C property.
 * @param[in]  size    Size of the C property.
 * @return Status object.
 */
Status ConverterManager::SetStreamProperty(
    Stream* stream, const std::string& key,
    const void* value, size_t size) {
  util::AutoLock autolock(&mutex_);
  ConverterBase* converter = GetConverter(kConverterTypeProperty, key);
  if (converter == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "unmanaged key=%s", key.c_str());
  }

  Status status;
  void* cxx_property = NULL;
  if (value != NULL) {
    status = converter->CreateCxxProperty(value, size, &cxx_property);
    SENSCORD_STATUS_TRACE(status);
  }

  if (status.ok()) {
    status = stream->SetProperty(key, cxx_property);
    SENSCORD_STATUS_TRACE(status);
  }

  converter->DeleteCxxProperty(value, size, cxx_property);

  return status;
}

/**
 * @brief Gets the channel property.
 * @param[in]   channel  Channel.
 * @param[in]   key      Search key.
 * @param[out]  value    Pointer to the C property.
 * @param[in]   size     Size of the C property.
 * @return Status object.
 */
Status ConverterManager::GetChannelProperty(
    Channel* channel, const std::string& key,
    void* value, size_t size) {
  util::AutoLock autolock(&mutex_);
  ConverterBase* converter = GetConverter(kConverterTypeProperty, key);
  if (converter == NULL) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "unmanaged key=%s", key.c_str());
  }

  void* cxx_property = NULL;
  Status status = converter->CreateCxxProperty(value, size, &cxx_property);
  SENSCORD_STATUS_TRACE(status);

  if (status.ok()) {
    status = channel->GetProperty(key, cxx_property);
    SENSCORD_STATUS_TRACE(status);
  }

  if (status.ok()) {
    status = converter->ConvertProperty(cxx_property, value, size);
    SENSCORD_STATUS_TRACE(status);
  }

  converter->DeleteCxxProperty(value, size, cxx_property);

  return status;
}
#endif  // SENSCORD_SERIALIZE

}  // namespace senscord
