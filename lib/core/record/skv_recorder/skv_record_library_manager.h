/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_SKV_RECORDER_SKV_RECORD_LIBRARY_MANAGER_H_
#define LIB_CORE_RECORD_SKV_RECORDER_SKV_RECORD_LIBRARY_MANAGER_H_

#include <string>
#include <vector>
#include <map>
#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "record/skv_recorder/skv_record_library.h"
#include "util/mutex.h"
#include "senscord/osal.h"

namespace senscord {

/**
 * @brief Manager of recorders (singleton).
 */
class SkvRecordLibraryManager : private util::Noncopyable {
 public:
  /**
   * @brief Get the manager instance.
   * @return Manager instance.
   */
  static SkvRecordLibraryManager* GetInstance();

  /**
   * @brief Initialize and set library path.
   * @return Status object.
   */
  Status Init();

  /**
   * @brief Create the new skv record library.
   * @param[in] (stream) parent stream of library management key.
   * @param[out] (library) skv record library.
   * @return Status object.
   */
  Status CreateSkvRecordLibrary(Stream* stream, SkvRecordLibrary** library);

  /**
   * @brief Release the skv record library.
   * @param[in] (library) The skv library to release.
   * @return Status object.
   */
  Status ReleaseSkvRecordLibrary(SkvRecordLibrary* library);

  /**
   * @brief Get the recordable format list.
   * @param[out] (formats) List of formats.
   * @return Status object.
   */
  Status GetRecordableFormats(std::vector<std::string>* formats) const;

  /**
   * @brief Get parent stream from skv record library.
   * @param[in] (library) skv record library.
   * @param[out] (stream) parent stream of library management key.
   * @return Status object.
   */
  Status GetStreamFromLibrary(SkvRecordLibrary* library, Stream** stream);

 private:
  /**
   * @brief Constructor.
   */
  SkvRecordLibraryManager();

  /**
   * @brief Destructor.
   */
  ~SkvRecordLibraryManager();

  util::Mutex mutex_;
  bool initialized_;
  std::map<Stream*, SkvRecordLibrary*> library_manager_;
};

}   // namespace senscord
#endif    // LIB_CORE_RECORD_SKV_RECORDER_SKV_RECORD_LIBRARY_MANAGER_H_
