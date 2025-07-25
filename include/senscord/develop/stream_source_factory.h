/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_STREAM_SOURCE_FACTORY_H_
#define SENSCORD_DEVELOP_STREAM_SOURCE_FACTORY_H_

#include <string>
#include <vector>
#include <utility>    // pair

#include "senscord/config.h"
#include "senscord/noncopyable.h"
#include "senscord/develop/common_types.h"
#include "senscord/develop/stream_source.h"

namespace senscord {

/**
 * @brief Factory of stream sources on the component.
 */
class StreamSourceFactory : private util::Noncopyable {
 public:
  // type of stream source.
  typedef std::pair<std::string, int32_t> SourceType;

  // type of the list of stream source type.
  typedef std::vector<SourceType> SourceTypeList;

  /**
   * @brief Get the List of supported types.
   * @param[in] (args) Arguments written by senscord.xml.
   * @param[out] (list) List of supported types.
   */
  virtual void GetSupportedList(
    const ComponentArgument& args, SourceTypeList* list) = 0;

  /**
   * @brief Create the stream source on the component.
   * @param[in] (type) Type of creating source.
   * @param[out] (source) Stream source.
   * @return Status object.
   */
  virtual Status CreateSource(
    const SourceType& type, StreamSource** source) = 0;

  /**
   * @brief Release the created stream source.
   * @param[in] (source) Stream source created by CreateSource().
   */
  virtual void ReleaseSource(StreamSource* source) {
    delete source;
  }

  /**
   * @brief Destructor
   */
  virtual ~StreamSourceFactory() {}
};

}   // namespace senscord
#endif    // SENSCORD_DEVELOP_STREAM_SOURCE_FACTORY_H_
