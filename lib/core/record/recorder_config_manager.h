/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_RECORD_RECORDER_CONFIG_MANAGER_H_
#define LIB_CORE_RECORD_RECORDER_CONFIG_MANAGER_H_

#include <string>
#include <vector>
#include <map>
#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/osal.h"

namespace senscord {

/**
 * @brief Config manager for recorder (singleton).
 */
class RecorderConfigManager : private util::Noncopyable {
 public:
  /**
   * @brief Read recorders config.
   * @param[in] (filename) Recorder config file path.
   * @return Status object.
   */
  Status ReadConfig(const std::string& filename);

  /**
   * @brief Get the recorder type name by format name.
   * @param[in] (format_name) Format name.
   * @param[out] (type_name) Type name (recorder name).
   * @return Status object.
   */
  Status GetRecorderType(
    const std::string& format_name, std::string* type_name) const;

  /**
   * @brief Get the recordable format list.
   * @param[out] (formats) List of formats.
   * @return Status object.
   */
  Status GetRecordableFormats(std::vector<std::string>* formats) const;

  /**
   * @brief Constructor
   */
  RecorderConfigManager();

  /**
   * @brief Destructor
   */
  ~RecorderConfigManager();

 private:
  /**
   * @brief Parse recorders element of config.
   * @return Status object.
   */
  Status ParseRecorders();

  /**
   * @brief Parse recorder element of config.
   * @return Status object.
   */
  Status ParseRecorder();

  /**
   * @brief Parse attribute of config.
   * @param[in] (attr_name) attribute name.
   * @param[out] (value) type attribute value.
   * @return Status object.
   */
  Status ParseAttribute(const char* attr_name, std::string* value);

  bool is_read_;

  // config informations
  typedef std::map<std::string, std::string> FormatList;
  FormatList format_list_;

  // XML Parser Class.
  osal::OSXmlParser parser_;
};

}    // namespace senscord
#endif    // LIB_CORE_RECORD_RECORDER_CONFIG_MANAGER_H_
