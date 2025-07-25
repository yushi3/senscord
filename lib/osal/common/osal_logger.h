/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_OSAL_COMMON_OSAL_LOGGER_H_
#define LIB_OSAL_COMMON_OSAL_LOGGER_H_

#include <stdio.h>
#include <stdarg.h>

#include <sstream>
#include <iomanip>

#include "senscord/logger_config.h"
#include "senscord/osal.h"

/**
 * @brief Log level.
 *
 * This definition value can be specified from CMake.
 */
#ifndef SENSCORD_OSAL_LOG_LEVEL
#define SENSCORD_OSAL_LOG_LEVEL senscord::osal::LOG_OFF
#endif  // SENSCORD_OSAL_LOG_LEVEL

/**
 * @brief Macro that checks log level.
 */
#define SENSCORD_OSAL_LOG_CHECK(severity) \
  (SENSCORD_OSAL_LOG_LEVEL >= (severity))

/**
 * @brief Log macro.
 *
 * If this macro is expanded and the left side is false,
 * the right side will not be evaluated.
 * It will be optimized by many compilers.
 *
 * @code
 * // SENSCORD_OSAL_LOG_LEVEL senscord::osal::LOG_WARNING
 *
 * // SENSCORD_OSAL_LOG(senscord::osal::LOG_WARNING, "logging is enabled");
 * true  && (bool)senscord::osal::PrintLog(...);
 * ^^^^^^^^
 *
 * // SENSCORD_OSAL_LOG(senscord::osal::LOG_INFO, "logging is disabled");
 * false && (bool)senscord::osal::PrintLog(...);
 * ^^^^^^^^
 * @endcode
 */
#define SENSCORD_OSAL_LOG(severity, ...) \
  SENSCORD_OSAL_LOG_CHECK(severity) && senscord::osal::PrintLog( \
      (severity), __FILE__, __LINE__, __VA_ARGS__)

#define SENSCORD_OSAL_LOG_ERROR(...) \
  SENSCORD_OSAL_LOG(senscord::osal::LOG_ERROR, __VA_ARGS__)

#define SENSCORD_OSAL_LOG_WARNING(...) \
  SENSCORD_OSAL_LOG(senscord::osal::LOG_WARNING, __VA_ARGS__)

#define SENSCORD_OSAL_LOG_INFO(...) \
  SENSCORD_OSAL_LOG(senscord::osal::LOG_INFO, __VA_ARGS__)

#define SENSCORD_OSAL_LOG_DEBUG(...) \
  SENSCORD_OSAL_LOG(senscord::osal::LOG_DEBUG, __VA_ARGS__)

namespace senscord {
namespace osal {

enum OSLogSeverity {
  LOG_OFF = 0,
  LOG_ERROR,
  LOG_WARNING,
  LOG_INFO,
  LOG_DEBUG,
};

/**
 * @brief Print log.
 * @param[in] (severity) Log severity.
 * @param[in] (filename) File name.
 * @param[in] (line_number) Line number.
 * @param[in] (format) format string.
 * @param[in] ... Variadic argument.
 * @return Always true (For short-circuit evaluation optimization)
 */
inline bool PrintLog(
    OSLogSeverity severity,
    const char* filename,
    int line_number,
    const char* format,
    ...) {
  std::ostringstream buffer;
#ifdef SENSCORD_LOG_TIME_ENABLED
  {
    OSSystemTime time = {};
    OSGetLocalTime(&time);
    buffer << std::setfill('0');
    buffer << std::setw(4) << time.year << '/';
    buffer << std::setw(2) << static_cast<uint16_t>(time.month) << '/';
    buffer << std::setw(2) << static_cast<uint16_t>(time.day) << ' ';
    buffer << std::setw(2) << static_cast<uint16_t>(time.hour) << ':';
    buffer << std::setw(2) << static_cast<uint16_t>(time.minute) << ':';
    buffer << std::setw(2) << static_cast<uint16_t>(time.second) << '.';
    buffer << std::setw(3) << time.milli_second << ' ';
  }
#endif  // SENSCORD_LOG_TIME_ENABLED
  if (filename != NULL) {
    const char* basename = OSBasename(filename);
    basename = (basename == NULL) ? filename : basename;
    buffer << '[' << basename << ':' << line_number << "] ";
  }
  if (severity != LOG_OFF) {
    const char* kLogSeverityString[] = {
        "",
        "Error",
        "Warning",
        "Info",
        "Debug",
    };
    buffer << kLogSeverityString[severity] << ": ";
  }
  buffer << "[osal] " << format << '\n';
  va_list args;
  va_start(args, format);
  OSVprintf(buffer.str().c_str(), args);
  va_end(args);
  return true;
}

}  //  namespace osal
}  //  namespace senscord

#endif  // LIB_OSAL_COMMON_OSAL_LOGGER_H_
