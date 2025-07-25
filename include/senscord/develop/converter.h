/*
 * SPDX-FileCopyrightText: 2020-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_CONVERTER_H_
#define SENSCORD_DEVELOP_CONVERTER_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/osal.h"
#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/serialize.h"

/**
 * @brief Macro for registering converter library.
 */
#define SENSCORD_REGISTER_CONVERTER(library_class_name)             \
  extern "C" void* CreateConverter() {                              \
    return new library_class_name();                                \
  }                                                                 \
  extern "C" void DeleteConverter(void* library) {                  \
    delete reinterpret_cast<senscord::ConverterLibrary*>(library);  \
  }

namespace senscord {

/**
 * @brief The base interface of the converter.
 */
class ConverterBase : private util::Noncopyable {
 public:
  virtual ~ConverterBase() {}

#ifdef SENSCORD_SERIALIZE
  /**
   * @brief Serialize the data.
   * @param[in]  input_data   Pointer to the input data.
   * @param[in]  input_size   Size of the input data.
   * @param[out] output_data  Output destination container.
   * @return Status object.
   */
  virtual Status Serialize(
      const void* input_data, size_t input_size,
      std::vector<uint8_t>* output_data) = 0;

  /**
   * @brief Deserialize the data.
   * @param[in]  input_data   Pointer to the input data.
   * @param[in]  input_size   Size of the input data.
   * @param[out] output_data  Pointer to the output data.
   * @param[in]  output_size  Size of the output data.
   * @return Status object.
   */
  virtual Status Deserialize(
      const void* input_data, size_t input_size,
      void* output_data, size_t output_size) = 0;
#else
  /**
   * @brief Creates the C++ property.
   * @param[in]  input_data       Pointer to the C property.
   * @param[in]  input_size       Size of the C property.
   * @param[out] output_property  Output C++ property instance.
   * @return Status object.
   */
  virtual Status CreateCxxProperty(
      const void* input_data, size_t input_size,
      void** output_property) = 0;

  /**
   * @brief Deletes the C++ property.
   * @param[in]  input_data  Pointer to the C property.
   * @param[in]  input_size  Size of the C property.
   * @param[in]  property    C++ property instance to delete.
   * @return Status object.
   */
  virtual void DeleteCxxProperty(
      const void* input_data, size_t input_size,
      void* property) = 0;

  /**
   * @brief Converts the property.
   * @param[in]  input_property  Pointer to the C++ property.
   * @param[out] output_data     Pointer to the C property.
   * @param[in]  output_size     Size of the C property.
   * @return Status object.
   */
  virtual Status ConvertProperty(
      const void* input_property,
      void* output_data, size_t output_size) = 0;
#endif  // SENSCORD_SERIALIZE
};

/**
 * @brief Struct converter for C++.
 */
template<typename CXX>
class StructConverterCxx : public ConverterBase {
 public:
  virtual ~StructConverterCxx() {}

#ifdef SENSCORD_SERIALIZE
  /**
   * @brief Serialize the data.
   * @param[in]  input_data   Pointer to the input data.
   * @param[in]  input_size   Size of the input data.
   * @param[out] output_data  Output destination container.
   * @return Status object.
   */
  virtual Status Serialize(
      const void* input_data, size_t input_size,
      std::vector<uint8_t>* output_data) {
    const CXX* src = reinterpret_cast<const CXX*>(input_data);
    serialize::SerializedBuffer buffer;
    serialize::Encoder encoder(&buffer);
    Status status = encoder.Push(*src);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    status = buffer.swap(output_data);
    return SENSCORD_STATUS_TRACE(status);
  }

  /**
   * @brief Deserialize the data.
   * @param[in]  input_data   Pointer to the input data.
   * @param[in]  input_size   Size of the input data.
   * @param[out] output_data  Pointer to the output data.
   * @param[in]  output_size  Size of the output data.
   * @return Status object.
   */
  virtual Status Deserialize(
      const void* input_data, size_t input_size,
      void* output_data, size_t output_size) {
    CXX* dst = reinterpret_cast<CXX*>(output_data);
    serialize::Decoder decoder(input_data, input_size);
    Status status = decoder.Pop(*dst);
    return SENSCORD_STATUS_TRACE(status);
  }
#endif  // SENSCORD_SERIALIZE
};

/**
 * @brief Struct converter for C.
 */
template<typename C, typename CXX>
class StructConverterC : public StructConverterCxx<CXX> {
 public:
  virtual ~StructConverterC() {}

  /**
   * @brief Convert C to C++ struct.
   * @param[in]  src  C struct of conversion source.
   * @param[out] dst  C++ struct of conversion destination.
   * @return Status object.
   */
  virtual Status c_to_cxx(const C& src, CXX* dst) = 0;

  /**
   * @brief Convert C++ to C struct.
   * @param[in]  src  C++ struct of conversion source.
   * @param[out] dst  C struct of conversion destination.
   * @return Status object.
   */
  virtual Status cxx_to_c(const CXX& src, C* dst) = 0;

#ifdef SENSCORD_SERIALIZE
  /**
   * @brief Serialize the data.
   * @param[in]  input_data   Pointer to the input data.
   * @param[in]  input_size   Size of the input data.
   * @param[out] output_data  Output destination container.
   * @return Status object.
   */
  virtual Status Serialize(
      const void* input_data, size_t input_size,
      std::vector<uint8_t>* output_data) {
    if (input_size != sizeof(C)) {
      return SENSCORD_STATUS_FAIL(
          "", Status::kCauseInvalidArgument,
          "invalid input size.");
    }
    const C* src = reinterpret_cast<const C*>(input_data);
    CXX tmp = {};
    Status status = c_to_cxx(*src, &tmp);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    status = StructConverterCxx<CXX>::Serialize(&tmp, 0, output_data);
    return SENSCORD_STATUS_TRACE(status);
  }

  /**
   * @brief Deserialize the data.
   * @param[in]  input_data   Pointer to the input data.
   * @param[in]  input_size   Size of the input data.
   * @param[out] output_data  Pointer to the output data.
   * @param[in]  output_size  Size of the output data.
   * @return Status object.
   */
  virtual Status Deserialize(
      const void* input_data, size_t input_size,
      void* output_data, size_t output_size) {
    if (output_size != sizeof(C)) {
      return SENSCORD_STATUS_FAIL(
          "", Status::kCauseInvalidArgument,
          "invalid output size.");
    }
    CXX tmp = {};
    Status status = StructConverterCxx<CXX>::Deserialize(
        input_data, input_size, &tmp, 0);
    if (!status.ok()) {
      return SENSCORD_STATUS_TRACE(status);
    }
    C* dst = reinterpret_cast<C*>(output_data);
    status = cxx_to_c(tmp, dst);
    return SENSCORD_STATUS_TRACE(status);
  }
#else
  /**
   * @brief Creates the C++ property.
   * @param[in]  input_data       Pointer to the C property.
   * @param[in]  input_size       Size of the C property.
   * @param[out] output_property  Output C++ property instance.
   * @return Status object.
   */
  virtual Status CreateCxxProperty(
      const void* input_data, size_t input_size,
      void** output_property) {
    if (input_size != sizeof(C)) {
      return SENSCORD_STATUS_FAIL(
          "", Status::kCauseInvalidArgument,
          "invalid input size.");
    }
    const C* src = reinterpret_cast<const C*>(input_data);
    CXX* tmp = new CXX();
    Status status = c_to_cxx(*src, tmp);
    if (!status.ok()) {
      delete tmp;
      return SENSCORD_STATUS_TRACE(status);
    }
    *output_property = tmp;
    return status;
  }

  /**
   * @brief Deletes the C++ property.
   * @param[in]  input_data  Pointer to the C property.
   * @param[in]  input_size  Size of the C property.
   * @param[in]  property    C++ property instance to delete.
   * @return Status object.
   */
  virtual void DeleteCxxProperty(
      const void* input_data, size_t input_size,
      void* property) {
    delete reinterpret_cast<CXX*>(property);
  }

  /**
   * @brief Converts the property.
   * @param[in]  input_property  Pointer to the C++ property.
   * @param[out] output_data     Pointer to the C property.
   * @param[in]  output_size     Size of the C property.
   * @return Status object.
   */
  virtual Status ConvertProperty(
      const void* input_property,
      void* output_data, size_t output_size) {
    if (output_size != sizeof(C)) {
      return SENSCORD_STATUS_FAIL(
          "", Status::kCauseInvalidArgument,
          "invalid output size.");
    }
    const CXX* src = reinterpret_cast<const CXX*>(input_property);
    C* dst = reinterpret_cast<C*>(output_data);
    Status status = cxx_to_c(*src, dst);
    return SENSCORD_STATUS_TRACE(status);
  }
#endif  // SENSCORD_SERIALIZE
};

/**
 * @brief Converter collector.
 */
class ConverterCollector {
 public:
  virtual ~ConverterCollector() {}

  /**
   * @brief Add converter.
   * @param[in] key        Search key.
   * @param[in] converter  Converter.
   */
  virtual void Add(const std::string& key, ConverterBase* converter) = 0;

  /**
   * @brief Add converter.
   * @param[in] key        Search key.
   * @param[in] converter  Converter.
   */
  template<typename C, typename CXX>
  void Add(const std::string& key, StructConverterC<C, CXX>* converter) {
    Add(key, static_cast<ConverterBase*>(converter));
  }
};

/**
 * @brief The base interface of the converter library.
 */
class ConverterLibrary {
 public:
  virtual ~ConverterLibrary() {}

  /**
   * @brief Initialize the converter library.
   * @param[in] collector  Converter collector.
   * @return Status object.
   */
  virtual Status Init(ConverterCollector* collector) = 0;

  /**
   * @brief Helper function that copies a string into a char array.
   * @param[in]  src  String of copy source.
   * @param[out] dst  Char array of destination.
   */
  template<size_t N>
  void StringToCharArray(const std::string& src, char (&dst)[N]) {
    size_t src_size = src.size() + 1;
    if (N > src_size) {
      osal::OSMemcpy(dst, N, src.c_str(), src_size);
    } else {
      osal::OSMemcpy(dst, N, src.c_str(), N - 1);
      dst[N - 1] = '\0';
    }
  }
};

}  // namespace senscord

#endif  // SENSCORD_DEVELOP_CONVERTER_H_
