/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_STATUS_H_
#define SENSCORD_STATUS_H_

#include <stdint.h>
#include <string>

#include "senscord/config.h"
#include "senscord/error_types.h"

#ifdef SENSCORD_STATUS_MESSAGE_ENABLED
#ifdef SENSCORD_STATUS_TRACE_ENABLED

/* Message:ON, Trace:ON */
/** @brief Trace macro. */
#define SENSCORD_STATUS_TRACE(status) (status).AddTrace(__FILE__, __LINE__)

/** @brief Create a fatal error status. */
#define SENSCORD_STATUS_FATAL(block, cause, ...) \
  senscord::Status(senscord::Status::kLevelFatal, (cause), __VA_ARGS__) \
      .SetBlock(block) \
      .AddTrace(__FILE__, __LINE__)

/** @brief Create a failure status. */
#define SENSCORD_STATUS_FAIL(block, cause, ...) \
  senscord::Status(senscord::Status::kLevelFail, (cause), __VA_ARGS__) \
      .SetBlock(block) \
      .AddTrace(__FILE__, __LINE__)

#else

/* Message:ON, Trace:OFF */
/** @brief Trace macro. */
#define SENSCORD_STATUS_TRACE(status) (status).AddTrace(NULL, 0)

/** @brief Create a fatal error status. */
#define SENSCORD_STATUS_FATAL(block, cause, ...) \
  senscord::Status(senscord::Status::kLevelFatal, (cause), __VA_ARGS__) \
      .SetBlock(block)

/** @brief Create a failure status. */
#define SENSCORD_STATUS_FAIL(block, cause, ...) \
  senscord::Status(senscord::Status::kLevelFail, (cause), __VA_ARGS__) \
      .SetBlock(block)

#endif  // SENSCORD_STATUS_TRACE_ENABLED
#else  // SENSCORD_STATUS_MESSAGE_ENABLED

/* Message:OFF, Trace:OFF */
/** @brief Trace macro. */
#define SENSCORD_STATUS_TRACE(status) (status).AddTrace(NULL, 0)

/** @brief Create a fatal error status. */
#define SENSCORD_STATUS_FATAL(block, cause, ...) \
  senscord::Status(senscord::Status::kLevelFatal, (cause), NULL)

/** @brief Create a failure status. */
#define SENSCORD_STATUS_FAIL(block, cause, ...) \
  senscord::Status(senscord::Status::kLevelFail, (cause), NULL)

#endif  // SENSCORD_STATUS_MESSAGE_ENABLED

/**
 * @brief Macro for argument checking.
 * If the judgment expression is true, call return with "Invalid Argument".
 * @param (expr) judgment expression.
 */
#define SENSCORD_STATUS_ARGUMENT_CHECK(expr) \
  do { \
    if (expr) { \
      return SENSCORD_STATUS_FAIL( \
          senscord::kStatusBlockCore, \
          senscord::Status::kCauseInvalidArgument, \
          #expr); \
    } \
  } while (false)

namespace senscord {

/**
 * @brief Block of occurrences on error.
 */
const char kStatusBlockCore[] = "core";

/**
 * @brief Status class indicates the success or failure.
 */
class Status {
 public:
  /**
   * @brief Level of error.
   * (high) kLevelFatal > kLevelFail > kLevelUndefined (low)
   */
  enum Level {
    /**
     * It is not normally used.
     * This value is returned by the level() function when the OK status.
     */
    kLevelUndefined = SENSCORD_LEVEL_UNDEFINED,
    /**
     * It is used in case of normal error.
     */
    kLevelFail = SENSCORD_LEVEL_FAIL,
    /**
     * It is used when system recovery is necessary.
     */
    kLevelFatal = SENSCORD_LEVEL_FATAL,
  };

  /**
   * @brief Cause of error.
   */
  enum Cause {
    kCauseNone = SENSCORD_ERROR_NONE,
    kCauseNotFound = SENSCORD_ERROR_NOT_FOUND,
    kCauseInvalidArgument = SENSCORD_ERROR_INVALID_ARGUMENT,
    kCauseResourceExhausted = SENSCORD_ERROR_RESOURCE_EXHAUSTED,
    kCausePermissionDenied = SENSCORD_ERROR_PERMISSION_DENIED,
    kCauseBusy = SENSCORD_ERROR_BUSY,
    kCauseTimeout = SENSCORD_ERROR_TIMEOUT,
    kCauseCancelled = SENSCORD_ERROR_CANCELLED,
    kCauseAborted = SENSCORD_ERROR_ABORTED,
    kCauseAlreadyExists = SENSCORD_ERROR_ALREADY_EXISTS,
    kCauseInvalidOperation = SENSCORD_ERROR_INVALID_OPERATION,
    kCauseOutOfRange = SENSCORD_ERROR_OUT_OF_RANGE,
    kCauseDataLoss = SENSCORD_ERROR_DATA_LOSS,
    kCauseHardwareError = SENSCORD_ERROR_HARDWARE_ERROR,
    kCauseNotSupported = SENSCORD_ERROR_NOT_SUPPORTED,
    kCauseUnknown = SENSCORD_ERROR_UNKNOWN,
  };

 public:
  /**
   * @brief Create an OK status.
   */
  static Status OK() { return Status(); }

#ifdef SENSCORD_STATUS_MESSAGE_ENABLED
  /**
   * @brief Create an OK status.
   */
  Status() : status_() {}

  /**
   * @brief Create an error status.
   *
   * Status created by this function returns false with the ok() function.
   *
   * @param[in] level   Level of error.
   * @param[in] cause   Cause of error.
   * @param[in] message ... Error message and optional arguments.
   * A format string in printf format can be used.
   */
  explicit Status(Level level, Cause cause, const char* message, ...);

  /**
   * @brief Copy the specified status.
   */
  Status(const Status& rhs);
  Status& operator =(const Status& rhs);

  /**
   * @brief Destructor.
   */
  ~Status();

  /**
   * @brief Returns formatted string.
   * In the case of OK status, it returns the string "OK".
   * In the case of error status, it returns the format string of
   * "level[cause]: message (block)\ntrace".
   */
  std::string ToString() const;

  /**
   * @brief Returns true if OK status.
   *
   * In case of warning or error, it returns false.
   */
  bool ok() const { return (status_ == NULL); }

  /**
   * @brief Returns the level of error.
   * In the case of OK status, it returns the kLevelUndefined.
   */
  Level level() const;

  /**
   * @brief Returns the cause of error.
   * In the case of OK status, it returns the kCauseNone.
   */
  Cause cause() const;

  /**
   * @brief Returns the error message.
   * In the case of OK status, it returns the empty string.
   */
  const std::string& message() const;

  /**
   * @brief Returns the occurrence block of error.
   * To set it, call SetBlock() function.
   * In the case of OK status, it returns the empty string.
   */
  const std::string& block() const;

  /**
   * @brief Returns the trace information of error.
   * To set it, call AddTrace() function.
   * In the case of OK status, it returns the empty string.
   */
#ifdef SENSCORD_STATUS_TRACE_ENABLED
  const std::string& trace() const;
#else
  const std::string& trace() const { return EmptyString(); }
#endif  // SENSCORD_STATUS_TRACE_ENABLED

  /**
   * @brief Set the occurrence block of error.
   * @param[in] block Occurrence block of error.
   * In the case of OK status, it is ignored.
   */
  Status& SetBlock(const std::string& block);

  /**
   * @brief Add a trace information of error.
   * @param[in] file  File name.
   * @param[in] line  Number of lines.
   * @see SENSCORD_STATUS_TRACE macro.
   * In the case of OK status, it is ignored.
   **/
#ifdef SENSCORD_STATUS_TRACE_ENABLED
  Status& AddTrace(const char* file, int32_t line);
#else
  Status& AddTrace(const char*, int32_t) { return *this; }
#endif  // SENSCORD_STATUS_TRACE_ENABLED

  /**
   * @brief Convert Level to string.
   * @param[in] level  Level of error.
   */
  static std::string GetLevelString(Level level);

  /**
   * @brief Convert Cause to string.
   * @param[in] cause  Cause of error.
   */
  static std::string GetCauseString(Cause cause);
#else  // SENSCORD_STATUS_MESSAGE_ENABLED
  Status() : level_(kLevelUndefined), cause_(kCauseNone) {}
  explicit Status(Level level, Cause cause, const char*, ...) :
      level_(static_cast<uint8_t>(level)),
      cause_(static_cast<uint8_t>(cause)) {}
  std::string ToString() const { return ok() ? "OK" : "NG"; }
  bool ok() const { return (cause_ == kCauseNone); }
  Level level() const { return static_cast<Level>(level_); }
  Cause cause() const { return static_cast<Cause>(cause_); }
  const std::string& message() const { return EmptyString(); }
  const std::string& block() const { return EmptyString(); }
  const std::string& trace() const { return EmptyString(); }
  Status& SetBlock(const std::string&) { return *this; }
  Status& AddTrace(const char*, int32_t) { return *this; }
  static std::string GetLevelString(Level) { return ""; }
  static std::string GetCauseString(Cause) { return ""; }
#endif  // SENSCORD_STATUS_MESSAGE_ENABLED

  /**
   * @brief Compare the level of error.
   * @param[in] level  Level of error.
   * (high) kLevelFatal > kLevelFail > kLevelUndefined (low)
   */
  bool operator ==(Level level) const { return (this->level() == level); }
  bool operator !=(Level level) const { return (this->level() != level); }
  bool operator > (Level level) const { return (this->level() > level);  }
  bool operator >=(Level level) const { return (this->level() >= level); }
  bool operator < (Level level) const { return (this->level() < level);  }
  bool operator <=(Level level) const { return (this->level() <= level); }

  /**
   * @brief Compare whether the cause of error matches.
   * @param[in] cause  Cause of error.
   */
  bool operator ==(Cause cause) const { return (this->cause() == cause); }
  bool operator !=(Cause cause) const { return (this->cause() != cause); }

 private:
  /**
   * @brief Returns the empty string.
   */
  static const std::string& EmptyString();

 private:
#ifdef SENSCORD_STATUS_MESSAGE_ENABLED
  /**
   * @brief Inner status.
   */
  void* status_;
#else
  uint8_t level_;
  uint8_t cause_;
#endif  // SENSCORD_STATUS_MESSAGE_ENABLED
};

}  // namespace senscord

#endif  // SENSCORD_STATUS_H_
