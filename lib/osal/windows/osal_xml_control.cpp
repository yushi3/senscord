/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>

#include "senscord/osal.h"
#include "common/osal_logger.h"
#include "windows/osal_xml_control.h"

namespace senscord {
namespace osal {

// first get singleton instance for thread safe.
static XmlControl* dummy = XmlControl::GetInstance();

/**
 * @brief XmlControl constructor.
 */
XmlControl::XmlControl() {
  OSCreateMutex(&mutex_);
  OSCreateCond(&cond_);
  use_flag_ = false;
}

/**
 * @brief XmlControl destructor.
 */
XmlControl::~XmlControl() {
  OSDestroyMutex(mutex_);
  OSDestroyCond(cond_);
}

/**
 * @brief Get singleton instance.
 */
XmlControl* XmlControl::GetInstance() {
  static XmlControl instance;
  return &instance;
}

/**
 * @brief Get control over the opening of the XML file.
 */
void XmlControl::GetControl() {
  int32_t mutex_result = OSLockMutex(mutex_);
  if (mutex_result != 0) {
    SENSCORD_OSAL_LOG_WARNING("OSLockMutex failed(0x%" PRIx32 ")",
        mutex_result);
  }

  while (use_flag_ != false) {
    int cond_result = OSWaitCond(cond_, mutex_);
    if (cond_result != 0) {
      SENSCORD_OSAL_LOG_WARNING("OSWaitCond failed(0x%" PRIx32 ")",
          cond_result);
    }
  }
  use_flag_ = true;

  mutex_result = OSUnlockMutex(mutex_);
  if (mutex_result != 0) {
    SENSCORD_OSAL_LOG_WARNING("OSUnlockMutex failed(0x%" PRIx32 ")",
        mutex_result);
  }
}

/**
 * @brief Release control over the opening of the XML file.
 */
void XmlControl::ReleaseControl() {
  int32_t mutex_result = OSLockMutex(mutex_);
  if (mutex_result != 0) {
    SENSCORD_OSAL_LOG_WARNING("OSLockMutex failed(0x%" PRIx32 ")",
        mutex_result);
  }

  use_flag_ = false;
  int cond_result = OSSignalCond(cond_);
  if (cond_result != 0) {
    SENSCORD_OSAL_LOG_WARNING("OSSignalCond failed(0x%" PRIx32 ")",
        cond_result);
  }

  mutex_result = OSUnlockMutex(mutex_);
  if (mutex_result != 0) {
    SENSCORD_OSAL_LOG_WARNING("OSUnlockMutex failed(0x%" PRIx32 ")",
        mutex_result);
  }
}

}  //  namespace osal
}  //  namespace senscord
