/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_LOGGER_H_
#define SENSCORD_LOGGER_H_

#include <string>

#include "senscord/config.h"
#include "senscord/noncopyable.h"

namespace senscord {

enum LogLevel {
  kLogOff = 0,
  kLogError,
  kLogWarning,
  kLogInfo,
  kLogDebug,
};

namespace util {

/** Logger tag: Default */
const char kLoggerTagDefault[] = "defaults";

/** Logger tag: Core internal */
const char kLoggerTagCore[] = "core";

/**
 * @brief Senscord logger class.
 */
class Logger {
 public:
  typedef LogLevel LogSeverity;
  static const LogSeverity kLogOff = senscord::kLogOff;
  static const LogSeverity kLogError = senscord::kLogError;
  static const LogSeverity kLogWarning = senscord::kLogWarning;
  static const LogSeverity kLogInfo = senscord::kLogInfo;
  static const LogSeverity kLogDebug = senscord::kLogDebug;

 public:
  virtual ~Logger() {}

  /**
   * @brief Output log
   * @param[in] (level) Log level.
   * @param[in] (filename) File name.
   * @param[in] (line_number) Line number.
   * @param[in] (format) Format string.
   * @param[in] ... Variadic argument.
   */
  virtual void PrintLog(
      LogLevel level,
      const char* filename,
      int line_number,
      const char* format,
      ...) = 0;

  /**
   * @brief Flush the log.
   */
  virtual void Flush() = 0;

  /**
   * @brief Set level threshold
   * @param[in] (level) level.
   */
  virtual void SetLevel(LogLevel level) = 0;

  /**
   * @brief Get level threshold
   * @return current level.
   */
  virtual LogLevel GetLevel() const = 0;
};

/**
 * @brief Senscord logger factory.
 */
class LoggerFactory : private util::Noncopyable {
 public:
  /**
   * @brief Get LoggerFactory instance.
   */
  static LoggerFactory* GetInstance();

  /**
   * @brief Get Logger instance.
   * @param[in] (tag) Tag name to identify the logger instance.
   * @return Logger instance.
   */
  Logger* GetLogger(const std::string& tag = kLoggerTagDefault);

  /**
   * @brief Create Logger instance.
   * @param[in] (tag) Tag name to identify the logger instance.
   * @param[in] (level) Output level for logger instance.
   * @return none.
   */
  void CreateLogger(const std::string& tag, LogLevel level = kLogInfo);

  /**
   * @brief Set LogLevel to Logger.
   * @param[in] (tag) Tag name to identify the logger instance.
   * @param[in] (level) Log output level.s
   * @return none
   */
  void SetLevel(
      const std::string& tag, LogLevel level);

 private:
  /**
   * @brief Constructor.
   */
  LoggerFactory();

  /**
   * @brief destructor.
   */
  ~LoggerFactory();

  /**
   * @brief Search Logger instance.
   * @param[in] (tag) Tag name to identify the logger instance.
   * @return Logger instance pointer
   */
  Logger* SearchLogger(const std::string& tag);

 private:
  struct Impl;
  Impl* pimpl_;
};

}  // namespace util
}  // namespace senscord

/** Get Logger instance */
#define SENSCORD_LOG_INSTANCE(tag) \
  senscord::util::LoggerFactory::GetInstance()->GetLogger(tag)

#ifdef SENSCORD_LOG_ENABLED
/** Set Log level */
#define SENSCORD_LOG_SET_LEVEL(tag, level) \
  senscord::util::LoggerFactory::GetInstance()->SetLevel(tag, level)

/** Get Log level */
#define SENSCORD_LOG_GET_LEVEL(tag) SENSCORD_LOG_INSTANCE(tag)->GetLevel()

/** Logger tag */
#ifndef SENSCORD_LOG_TAG

#ifdef SENSCORD_CORE_BUILD
#define SENSCORD_LOG_TAG senscord::util::kLoggerTagCore
#else
#define SENSCORD_LOG_TAG senscord::util::kLoggerTagDefault
#endif  // SENSCORD_CORE_BUILD
#endif  // SENSCORD_LOG_TAG

/*----------------------------------------------*/
/* No tagged macro                              */
/*----------------------------------------------*/
/** Log macro */
#define SENSCORD_LOG(level, ...) \
  SENSCORD_LOG_INSTANCE(SENSCORD_LOG_TAG)->PrintLog( \
      (level), __FILE__, __LINE__, __VA_ARGS__)

/** Error log */
#define SENSCORD_LOG_ERROR(...) \
  SENSCORD_LOG(senscord::kLogError, __VA_ARGS__)

/** Warning log */
#define SENSCORD_LOG_WARNING(...) \
  SENSCORD_LOG(senscord::kLogWarning, __VA_ARGS__)

/** Info log */
#define SENSCORD_LOG_INFO(...) \
  SENSCORD_LOG(senscord::kLogInfo, __VA_ARGS__)

/** Debug log */
#define SENSCORD_LOG_DEBUG(...) \
  SENSCORD_LOG(senscord::kLogDebug, __VA_ARGS__)

/** Flush logs */
#define SENSCORD_LOG_FLUSH() \
  SENSCORD_LOG_INSTANCE(SENSCORD_LOG_TAG)->Flush()

/** Get log level */
#define SENSCORD_LOG_SEVERITY \
  SENSCORD_LOG_INSTANCE(SENSCORD_LOG_TAG)->GetLevel()

/*----------------------------------------------*/
/* On tagged macro                              */
/*----------------------------------------------*/
/** Log macro */
#define SENSCORD_LOG_TAGGED(tag, level, ...) \
  SENSCORD_LOG_INSTANCE(tag)->PrintLog( \
      (level), __FILE__, __LINE__, __VA_ARGS__)

/** Error log */
#define SENSCORD_LOG_ERROR_TAGGED(tag, ...) \
  SENSCORD_LOG_TAGGED(tag, senscord::kLogError, __VA_ARGS__)

/** Warning log */
#define SENSCORD_LOG_WARNING_TAGGED(tag, ...) \
  SENSCORD_LOG_TAGGED(tag, senscord::kLogWarning, __VA_ARGS__)

/** Info log */
#define SENSCORD_LOG_INFO_TAGGED(tag, ...) \
  SENSCORD_LOG_TAGGED(tag, senscord::kLogInfo, __VA_ARGS__)

/** Debug log */
#define SENSCORD_LOG_DEBUG_TAGGED(tag, ...) \
  SENSCORD_LOG_TAGGED(tag, senscord::kLogDebug, __VA_ARGS__)

/** Flush logs */
#define SENSCORD_LOG_FLUSH_TAGGED(tag) \
  SENSCORD_LOG_INSTANCE(tag)->Flush()

#else
#define SENSCORD_LOG_SET_LEVEL(tag, level)
#define SENSCORD_LOG_GET_LEVEL(tag) senscord::kLogOff
#define SENSCORD_LOG(level, ...)
#define SENSCORD_LOG_ERROR(...)
#define SENSCORD_LOG_WARNING(...)
#define SENSCORD_LOG_INFO(...)
#define SENSCORD_LOG_DEBUG(...)
#define SENSCORD_LOG_FLUSH()
#define SENSCORD_LOG_SEVERITY     senscord::kLogOff
#define SENSCORD_LOG_TAGGED(tag, level, ...)
#define SENSCORD_LOG_ERROR_TAGGED(tag, ...)
#define SENSCORD_LOG_WARNING_TAGGED(tag, ...)
#define SENSCORD_LOG_INFO_TAGGED(tag, ...)
#define SENSCORD_LOG_DEBUG_TAGGED(tag, ...)
#define SENSCORD_LOG_FLUSH_TAGGED(tag)

#endif  // SENSCORD_LOG_ENABLED

#endif  // SENSCORD_LOGGER_H_
