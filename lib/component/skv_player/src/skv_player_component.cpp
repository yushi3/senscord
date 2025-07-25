/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <vector>
#include <utility>
#include "senscord/logger.h"
#include "senscord/develop/standard_component.h"
#include "src/skv_player_common.h"
#include "src/skv_player_source.h"

/**
 * @brief The factory of stream sources for pseudo image component.
 */
class SkvPlayerSourceFactory : public senscord::StreamSourceFactory {
 public:
  /**
   * @brief Get the List of supported types.
   * @param[in] (args) Arguments written by senscord.xml.
   * @param[out] (list) List of supported types.
   */
  virtual void GetSupportedList(
      const senscord::ComponentArgument& args, SourceTypeList* list) {
    list->push_back(std::make_pair(senscord::kStreamTypeDepth, 0));
  }

  /**
   * @brief Create the stream source on the component.
   * @param[in] (type) Type of creating source.
   * @param[out] (source) Stream source.
   * @return Status object.
   */
  virtual senscord::Status CreateSource(
      const SourceType& type, senscord::StreamSource** source) {
    if (type.first == senscord::kStreamTypeDepth) {
      *source = new SkvPlayerSource();
    } else {
      return SENSCORD_STATUS_FAIL(
        kBlockName,
        senscord::Status::kCauseInvalidArgument,
        "Invalid stream type.");
    }
    return senscord::Status::OK();
  }
};

// register
SENSCORD_REGISTER_COMPONENT(SkvPlayerSourceFactory)
