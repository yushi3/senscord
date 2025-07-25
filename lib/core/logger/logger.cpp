/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "logger/logger.h"

#include <stdio.h>
#include <stdarg.h>

#include <iomanip>
#include <map>
#include <string>
#include <sstream>
#include <utility>

#include "senscord/logger_config.h"
#include "senscord/osal.h"
#include "util/singleton.h"

#ifdef SENSCORD_LOG_TYPE_SYSLOG
#include <syslog.h>
#endif  // SENSCORD_LOG_TYPE_SYSLOG

namespace {

#ifdef SENSCORD_LOG_ENABLED

#ifdef SENSCORD_LOG_TYPE_FILE
const char kLogFileName[] = "senscord.log";
#endif  // SENSCORD_LOG_TYPE_FILE

#ifndef SENSCORD_LOG_TYPE_SYSLOG
/**
 * @brief Adds a time string to the log buffer.
 * @param[out] (buffer) Log buffer.
 */
void AddLogTime(std::ostringstream* buffer) {
#ifdef SENSCORD_LOG_TIME_ENABLED
  senscord::osal::OSSystemTime time = {};
  senscord::osal::OSGetLocalTime(&time);
  *buffer << std::setfill('0');
  *buffer << std::setw(4) << time.year << '/';
  *buffer << std::setw(2) << static_cast<uint16_t>(time.month) << '/';
  *buffer << std::setw(2) << static_cast<uint16_t>(time.day) << ' ';
  *buffer << std::setw(2) << static_cast<uint16_t>(time.hour) << ':';
  *buffer << std::setw(2) << static_cast<uint16_t>(time.minute) << ':';
  *buffer << std::setw(2) << static_cast<uint16_t>(time.second) << '.';
  *buffer << std::setw(3) << time.milli_second << ' ';
#endif  // SENSCORD_LOG_TIME_ENABLED
}

/**
 * @brief Adds a level string to the log buffer.
 * @param[out] (buffer) Log buffer.
 * @param[in] (level) Log level.
 */
void AddLogLevel(
    std::ostringstream* buffer, senscord::LogLevel level) {
  if (level != senscord::kLogOff) {
    const char* kLogLevelString[] = {
        "",
        "Error",
        "Warning",
        "Info",
        "Debug",
    };
    *buffer << kLogLevelString[level] << ": ";
  }
}
#endif  // SENSCORD_LOG_TYPE_SYSLOG

/**
 * @brief Adds a tag to the log buffer.
 * @param[out] (buffer) Log buffer.
 * @param[in] (tag) Logger tag.
 */
void AddLogTag(std::ostringstream* buffer, const std::string& tag) {
#if 0
  if (tag.empty()) {
    *buffer << "[senscord] ";
  } else {
    *buffer << "[" << tag << "] ";
  }
#endif
}

/**
 * @brief Adds a filename to the log buffer.
 * @param[out] (buffer) Log buffer.
 * @param[in] (filename) File name.
 * @param[in] (line_number) Line number.
 */
void AddLogFilename(
    std::ostringstream* buffer, const char* filename, int32_t line_number) {
  if (filename != NULL) {
    const char* basename = senscord::osal::OSBasename(filename);
    basename = (basename == NULL) ? filename : basename;
    *buffer << '[' << basename << ':' << line_number << "] ";
  }
}
#endif  // SENSCORD_LOG_ENABLED

}  // namespace

namespace senscord {
namespace util {

#ifdef SENSCORD_LOG_ENABLED

/**
 * @brief Logger base.
 */
class LoggerBase : public Logger {
 public:
  /**
   * @brief Constructor.
   */
  explicit LoggerBase(const std::string& tag, LogLevel level)
      : tag_(tag), level_(level) {}

  /**
   * @brief Get logger tag
   */
  const std::string& GetTag() const {
    return tag_;
  }

  /**
   * @brief Set level threshold
   * @param[in] (level) level.
   */
  virtual void SetLevel(LogLevel level) {
    level_ = (kLogOff > level) ? kLogOff :
        ((kLogDebug < level) ? kLogDebug : level);
  }

  /**
   * @brief Get level threshold
   * @return current level.
   */
  virtual LogLevel GetLevel() const {
    return level_;
  }

  /**
   * @brief Check level threshold
   * @return true if the log output condition.
   */
  bool CheckLevel(LogLevel level) const {
    return (level_ != kLogOff) && (level_ >= level);
  }

 private:
  std::string tag_;
  LogLevel level_;
};

#ifdef SENSCORD_LOG_TYPE_CONSOLE
/**
 * @brief Console Logger.
 */
class LoggerConsole : public LoggerBase {
 public:
  /**
   * @brief Constructor.
   */
  explicit LoggerConsole(const std::string& tag, LogLevel level) :
      LoggerBase(tag, level) {}

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
      ...) {
    if (!CheckLevel(level)) {
      return;
    }
    std::ostringstream buffer;
    AddLogTime(&buffer);
    AddLogTag(&buffer, GetTag());
    AddLogFilename(&buffer, filename, line_number);
    AddLogLevel(&buffer, level);
    buffer << format << '\n';
    va_list args;
    va_start(args, format);
    osal::OSVprintf(buffer.str().c_str(), args);
    va_end(args);
  }

  /**
   * @brief Flush the log.
   */
  virtual void Flush() {
    // stdout FILE* can be converted to OSFile*.
    osal::OSFflush(reinterpret_cast<osal::OSFile*>(stdout));
  }
};

typedef LoggerConsole LoggerCore;

#endif  // SENSCORD_LOG_TYPE_CONSOLE

#ifdef SENSCORD_LOG_TYPE_FILE
/**
 * @brief File Logger.
 */
class LoggerFile : public LoggerBase {
 private:
  class File : public Singleton<File> {
   public:
    File() : fd_() {
      int32_t ret = osal::OSFopen(kLogFileName, "a", &fd_);
      if (ret != 0) {
        osal::OSPrintf("[senscord] Error: create log file(%s)\n", kLogFileName);
      }
    }

    ~File() {
      if (fd_ != NULL) {
        osal::OSFclose(fd_);
        fd_ = NULL;
      }
    }

    osal::OSFile* GetFile() const { return fd_; }

   private:
    osal::OSFile* fd_;
  };

 public:
  /**
   * @brief Constructor.
   */
  explicit LoggerFile(const std::string& tag, LogLevel level) :
      LoggerBase(tag, level) {}

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
      ...) {
    osal::OSFile* file = File::GetInstance()->GetFile();
    if (file == NULL) {
      return;
    }
    if (!CheckLevel(level)) {
      return;
    }
    std::ostringstream buffer;
    AddLogTime(&buffer);
    AddLogTag(&buffer, GetTag());
    AddLogFilename(&buffer, filename, line_number);
    AddLogLevel(&buffer, level);
    buffer << format << '\n';
    va_list args;
    va_start(args, format);
#ifdef _WIN32
    vfprintf_s(reinterpret_cast<FILE*>(file), buffer.str().c_str(), args);
#else
    vfprintf(reinterpret_cast<FILE*>(file), buffer.str().c_str(), args);
#endif  // _WIN32
    va_end(args);
  }

  /**
   * @brief Flush the log.
   */
  virtual void Flush() {
    osal::OSFflush(File::GetInstance()->GetFile());
  }
};

typedef LoggerFile LoggerCore;

#endif  // SENSCORD_LOG_TYPE_FILE

#ifdef SENSCORD_LOG_TYPE_SYSLOG
/**
 * @brief Syslog Logger.
 */
class LoggerSyslog : public LoggerBase {
 public:
  /**
   * @brief Constructor.
   */
  explicit LoggerSyslog(const std::string& tag, LogLevel level) :
      LoggerBase(tag, level) {}

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
      ...) {
    if (!CheckLevel(level)) {
      return;
    }
    static const int SyslogLevel[] = {
        0,
        LOG_ERR,
        LOG_WARNING,
        LOG_INFO,
        LOG_DEBUG,
    };
    std::ostringstream buffer;
    AddLogTag(&buffer, GetTag());
    AddLogFilename(&buffer, filename, line_number);
    buffer << format;
    va_list args;
    va_start(args, format);
    vsyslog(SyslogLevel[level], buffer.str().c_str(), args);
    va_end(args);
  }

  /**
   * @brief Flush the log.
   */
  virtual void Flush() {}
};

typedef LoggerSyslog LoggerCore;

#endif  // SENSCORD_LOG_TYPE_SYSLOG

#else

class LoggerCore : public Logger {
 public:
  explicit LoggerCore(const std::string&, LogLevel level) {}
  virtual void PrintLog(LogLevel, const char*, int, const char*, ...) {}
  virtual void Flush() {}
  virtual void SetLevel(LogLevel) {}
  virtual LogLevel GetLevel() const { return kLogOff; }
};

#endif  // SENSCORD_LOG_ENABLED

struct LoggerFactory::Impl {
  Mutex mutex;
  LoggerCore defaults_logger;
  std::map<std::string, LoggerCore> loggers;

  Impl() : defaults_logger(kLoggerTagDefault, senscord::kLogInfo) {}
};

/**
 * @brief Get LoggerFactory instance.
 */
LoggerFactory* LoggerFactory::GetInstance() {
  // for private constructor / destructor
  struct InnerLoggerFactory : public LoggerFactory {
  };
  return Singleton<InnerLoggerFactory, true>::GetInstance();
}

/**
 * @brief Create Logger instance.
 * @param[in] (tag) Tag name to identify the logger instance.
 * @param[in] (level) Output level for logger instance.
 * @return none.
 */
void LoggerFactory::CreateLogger(
    const std::string& tag, LogLevel level) {
  util::AutoLock lock(&pimpl_->mutex);
  if (tag == kLoggerTagDefault) {
    pimpl_->defaults_logger.SetLevel(level);
  } else {
    std::map<std::string, LoggerCore>::iterator itr = pimpl_->loggers.find(tag);
    if (itr == pimpl_->loggers.end()) {
      pimpl_->loggers.insert(std::make_pair(tag, LoggerCore(tag, level)));
    } else {
     itr->second.SetLevel(level);
    }
  }
}

/**
 * @brief Get Logger instance.
 * @param[in] (tag) Tag name to identify the logger instance.
 * @return Logger instance.
 */
Logger* LoggerFactory::GetLogger(const std::string& tag) {
  util::AutoLock lock(&pimpl_->mutex);
  std::map<std::string, LoggerCore>::iterator itr = pimpl_->loggers.find(tag);
  if (itr != pimpl_->loggers.end()) {
    return &itr->second;
  }

  return &pimpl_->defaults_logger;
}

/**
 * @brief Search Logger instance.
 * @param[in] (tag) Tag name to identify the logger instance.
 * @return Logger instance.
 */
Logger* LoggerFactory::SearchLogger(const std::string& tag) {
  if (tag == kLoggerTagDefault) {
    return &pimpl_->defaults_logger;
  } else {
    std::map<std::string, LoggerCore>::iterator itr = pimpl_->loggers.find(tag);
    if (itr != pimpl_->loggers.end()) {
      return &itr->second;
    }
  }
  return NULL;
}

/**
 * @brief Set LogLevel to Logger.
 * @param[in] (tag) Tag name to identify the logger instance.
 * @param[in] (level) Log output level.
 * @return none
 */
void LoggerFactory::SetLevel(const std::string& tag, LogLevel level) {
  util::AutoLock lock(&pimpl_->mutex);
  Logger* logger = SearchLogger(tag);
  if (logger != NULL) {
    logger->SetLevel(level);
  }
}

LoggerFactory::LoggerFactory()
    : pimpl_(new Impl) {
  CreateLogger(kLoggerTagCore);
}

LoggerFactory::~LoggerFactory() {
  delete pimpl_;
}

}  // namespace util
}  // namespace senscord
