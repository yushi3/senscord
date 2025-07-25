/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "record/skv_recorder/skv_record_library_manager.h"
#include <string>
#include <vector>
#include <utility>
#include "core/internal_types.h"
#include "util/autolock.h"
#include "util/senscord_utils.h"
#include "util/singleton.h"
#include "logger/logger.h"

namespace senscord {

/**
 * @brief Get the manager instance.
 * @return Manager instance.
 */
SkvRecordLibraryManager* SkvRecordLibraryManager::GetInstance() {
  // for private constructor / destructor
  struct InnerSkvRecordLibraryManager : public SkvRecordLibraryManager {
  };
  return util::Singleton<InnerSkvRecordLibraryManager>::GetInstance();
}

/**
 * @brief Initialize and set library path.
 * @return Status object.
 */
Status SkvRecordLibraryManager::Init() {
  util::AutoLock autolock(&mutex_);
  if (initialized_) {
    return Status::OK();
  }

  initialized_ = true;
  return Status::OK();
}

/**
 * @brief Create the new skv record library.
 * @param[in] (stream) parent stream of library management key.
 * @param[out] (library) skv record library.
 * @return Status object.
 */
Status SkvRecordLibraryManager::CreateSkvRecordLibrary(
    Stream* stream, SkvRecordLibrary** library) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  SkvRecordLibrary* origin = new SkvRecordLibrary();
  // load recorder
  util::AutoLock autolock(&mutex_);
  if (!library_manager_.insert(std::make_pair(stream, origin)).second) {
    // roll-back
    delete origin;
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAlreadyExists,
        "Already registered stream: stream=%p", stream);
  }

  *library = origin;
  return Status::OK();
}

/**
 * @brief Release the skv record library.
 * @param[in] (stream) parent stream of library management key.
 * @return Status object.
 */
Status SkvRecordLibraryManager::ReleaseSkvRecordLibrary(
    SkvRecordLibrary* library) {
  if (library == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  // get stream of managment key from library
  Stream* stream;
  GetStreamFromLibrary(library, &stream);

  // erase library from manager.
  util::AutoLock autolock(&mutex_);
  std::map<Stream*, SkvRecordLibrary*>::iterator found =
      library_manager_.find(stream);
  if (found != library_manager_.end()) {
    delete found->second;
    // pop
    library_manager_.erase(found);
  }

  return Status::OK();
}

/**
 * @brief Get parent stream from skv record library.
 * @param[in] (library) skv record library.
 * @param[out] (stream) parent stream of library management key.
 * @return Status object.
 */
Status SkvRecordLibraryManager::GetStreamFromLibrary(
    SkvRecordLibrary* library, Stream** stream) {
  if (library == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }

  std::map<Stream*, SkvRecordLibrary*>::iterator itr = library_manager_.begin();
  for (; itr != library_manager_.end(); ++itr) {
    if (itr->second == library) {
      *stream = itr->first;
      return Status::OK();
    }
  }
  return SENSCORD_STATUS_FAIL(
      kStatusBlockCore, Status::kCauseNotFound, "Stream is not found ");
}

/**
 * @brief Constructor.
 */
SkvRecordLibraryManager::SkvRecordLibraryManager() :
    initialized_(false), library_manager_() {}

/**
 * @brief Destructor.
 */
SkvRecordLibraryManager::~SkvRecordLibraryManager() {}

}   // namespace senscord
