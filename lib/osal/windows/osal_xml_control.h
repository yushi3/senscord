/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_OSAL_WINDOWS_OSAL_XML_CONTROL_H_
#define LIB_OSAL_WINDOWS_OSAL_XML_CONTROL_H_

#include <stdint.h>

#include "senscord/osal.h"

namespace senscord {
namespace osal {

/**
 * @brief Class that controls the opening of the XML file.
 */
class XmlControl {
 public:
  /**
   * @brief Get singleton instance.
   */
  static XmlControl* GetInstance();

  /**
   * @brief Get control over the opening of the XML file.
   */
  void GetControl();

  /**
   * @brief Release control over the opening of the XML file.
   */
  void ReleaseControl();

 private:
  /**
   * @brief XmlControl constructor.
   */
  XmlControl();

  /**
   * @brief XmlControl destructor.
   */
  ~XmlControl();

  XmlControl(const XmlControl&);  // = delete;
  XmlControl& operator=(const XmlControl&);  // = delete;

  OSMutex* mutex_;
  OSCond*  cond_;
  bool     use_flag_;
};

}  //  namespace osal
}  //  namespace senscord

#endif  // LIB_OSAL_WINDOWS_OSAL_XML_CONTROL_H_
