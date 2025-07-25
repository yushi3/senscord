/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sstream>
#include <string>
#include <vector>

#include "senscord/environment.h"
#include "senscord/osal.h"
#include "senscord/logger.h"
#include "core/internal_types.h"
#include "util/autolock.h"
#include "util/mutex.h"

namespace senscord {

std::vector<std::string> Environment::senscord_file_paths_;

namespace {

senscord::util::Mutex* GetMutex() {
  static senscord::util::Mutex mutex;
  return &mutex;
}

}  // namespace

/**
 * @brief Get the path of senscord config file.
 * @param[out] (path) File path.
 * @return Status object.
 */
Status Environment::GetSensCordFilePath(std::vector<std::string>* paths) {
  if (paths == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  util::AutoLock lock(GetMutex());
  *paths = Environment::senscord_file_paths_;

  return Status::OK();
}

/**
 * @brief Set the path of senscord config file.
 * @param[in] (path) File path.
 * @return Status object.
 */
Status Environment::SetSensCordFilePath(
    const std::vector<std::string>& paths) {
  if (paths.empty()) {
    return SENSCORD_STATUS_FAIL(
      kStatusBlockCore, Status::kCauseInvalidArgument,
      "The argument paths are not set.");
  }

  std::vector<std::string>::const_iterator itr = paths.begin();
  std::vector<std::string>::const_iterator end = paths.end();
  for (; itr != end; ++itr) {
    if (itr->empty()) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument,
          "Can not set empty path: \"%s\"", itr->c_str());
    }
  }

  util::AutoLock lock(GetMutex());
  Environment::senscord_file_paths_ = paths;

  return Status::OK();
}

}  // namespace senscord
