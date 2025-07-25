/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string>
#include <sstream>

#include "senscord/status.h"
#include "senscord/osal.h"
#ifdef SENSCORD_STATUS_MESSAGE_ENABLED
#include "util/shared_pointer.h"
#endif  // SENSCORD_STATUS_MESSAGE_ENABLED

namespace senscord {

#ifdef SENSCORD_STATUS_MESSAGE_ENABLED

/**
 * @brief Maximum length of message.
 */
static const uint32_t kMessageMaxLength = 512;

/**
 * @brief Inner status.
 */
struct InnerStatus {
  Status::Level level;  ///< Level of error.
  Status::Cause cause;  ///< Cause of error.
  std::string message;  ///< Error message.
  std::string block;    ///< Occurrence block of error.
#ifdef SENSCORD_STATUS_TRACE_ENABLED
  std::string trace;    ///< Trace information of error.
  std::ostringstream trace_buffer;  ///< Buffer for tracing.
#endif  // SENSCORD_STATUS_TRACE_ENABLED
};

typedef SharedPointer<InnerStatus> Pointer;

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
Status::Status(Level level, Cause cause, const char* message, ...)
    : status_(new Pointer(new InnerStatus)) {
  Pointer& this_status = *reinterpret_cast<Pointer*>(status_);
  this_status->level = level;
  this_status->cause = cause;
  if (message != NULL) {
    va_list args;
    va_start(args, message);
    char buffer[kMessageMaxLength];
    int32_t ret = osal::OSVsnprintf(buffer, sizeof(buffer), message, args);
    if (ret < 0) {
      // convert error.
      this_status->message = message;
    } else {
      this_status->message = buffer;
    }
    va_end(args);
  }
}

/**
 * @brief Copy the specified status.
 */
Status::Status(const Status& rhs) : status_() {
  if (rhs.status_ == NULL) {
    // OK status.
    return;
  }
  const Pointer& rhs_status = *reinterpret_cast<const Pointer*>(rhs.status_);
  status_ = new Pointer(rhs_status);
}

/**
 * @brief Copy the specified status.
 */
Status& Status::operator =(const Status& rhs) {
  if (rhs.status_ == NULL) {
    // OK status.
    delete reinterpret_cast<Pointer*>(status_);
    status_ = NULL;
    return *this;
  }
  const Pointer& rhs_status = *reinterpret_cast<const Pointer*>(rhs.status_);
  if (status_ == NULL) {
    status_ = new Pointer(rhs_status);
  } else {
    Pointer& this_status = *reinterpret_cast<Pointer*>(status_);
    if (this_status != rhs_status) {
      this_status = rhs_status;
    }
  }
  return *this;
}

/**
 * @brief Destructor.
 */
Status::~Status() {
  delete reinterpret_cast<Pointer*>(status_);
}

/**
 * @brief Returns formatted string.
 * In the case of OK status, it returns the string "OK".
 * In the case of error status, it returns the format string of
 * "level[cause] message (block)\ntrace".
 */
std::string Status::ToString() const {
  if (ok()) {
    return "OK";
  }
  const Pointer& this_status = *reinterpret_cast<const Pointer*>(status_);
  std::ostringstream oss;
  oss << GetLevelString(this_status->level);
  oss << "[" << GetCauseString(this_status->cause) << "]: ";
  if (!this_status->message.empty()) {
    oss << this_status->message;
  } else {
    oss << "(empty message)";
  }
  if (!this_status->block.empty()) {
    oss << " (" << this_status->block << ")";
  }
  const std::string& file_trace = trace();
  if (!file_trace.empty()) {
    oss << '\n' << file_trace;
  }
  return oss.str();
}

/**
 * @brief Returns the level of error.
 * In the case of OK status, it returns the kLevelUndefined.
 */
Status::Level Status::level() const {
  if (ok()) {
    return kLevelUndefined;
  }
  const Pointer& this_status = *reinterpret_cast<const Pointer*>(status_);
  return this_status->level;
}

/**
 * @brief Returns the cause of error.
 * In the case of OK status, it returns the kCauseNone.
 */
Status::Cause Status::cause() const {
  if (ok()) {
    return kCauseNone;
  }
  const Pointer& this_status = *reinterpret_cast<const Pointer*>(status_);
  return this_status->cause;
}

/**
 * @brief Returns the error message.
 * In the case of OK status, it returns the empty string.
 */
const std::string& Status::message() const {
  if (ok()) {
    return EmptyString();
  }
  const Pointer& this_status = *reinterpret_cast<const Pointer*>(status_);
  return this_status->message;
}

/**
 * @brief Returns the occurrence block of error.
 * To set it, call SetBlock() function.
 * In the case of OK status, it returns the empty string.
 */
const std::string& Status::block() const {
  if (ok()) {
    return EmptyString();
  }
  const Pointer& this_status = *reinterpret_cast<const Pointer*>(status_);
  return this_status->block;
}

/**
 * @brief Set the occurrence block of error.
 * @param[in] block Occurrence block of error.
 * In the case of OK status, it is ignored.
 */
Status& Status::SetBlock(const std::string& block) {
  if (ok()) {
    return *this;
  }
  Pointer& this_status = *reinterpret_cast<Pointer*>(status_);
  this_status->block = block;
  return *this;
}

#ifdef SENSCORD_STATUS_TRACE_ENABLED
/**
 * @brief Returns the trace information of error.
 * To set it, call AddTrace() function.
 * In the case of OK status, it returns the empty string.
 */
const std::string& Status::trace() const {
  if (ok()) {
    return EmptyString();
  }
  Pointer& this_status = *reinterpret_cast<Pointer*>(status_);
  this_status->trace = this_status->trace_buffer.str();
  return this_status->trace;
}

/**
 * @brief Add a trace information of error.
 * @param[in] file  File name.
 * @param[in] line  Number of lines.
 * @see SENSCORD_STATUS_TRACE macro.
 * In the case of OK status, it is ignored.
 **/
Status& Status::AddTrace(const char* file, int32_t line) {
  if (ok()) {
    return *this;
  }
  if (file == NULL) {
    return *this;
  }

  // full path -> file name.
  const char* filename = osal::OSBasename(file);
  if (filename == NULL) {
    filename = file;
  }

  Pointer& this_status = *reinterpret_cast<Pointer*>(status_);
  std::ostringstream::pos_type pos = this_status->trace_buffer.tellp();
  if (pos > 0) {
    this_status->trace_buffer << '\n';
  }
  this_status->trace_buffer << filename << ':' << line;
  return *this;
}
#endif  // SENSCORD_STATUS_TRACE_ENABLED

/**
 * @brief Convert Level to string.
 * @param[in] level  Level of error.
 */
std::string Status::GetLevelString(Level level) {
  switch (level) {
    case kLevelFatal:
      return "Fatal";
    case kLevelFail:
      return "Fail";
    default:
      return "Undefined";
  }
}

/**
 * @brief Convert Cause to string.
 * @param[in] cause  Cause of error.
 */
std::string Status::GetCauseString(Cause cause) {
  switch (cause) {
    case kCauseNone:
      return "None";
    case kCauseNotFound:
      return "NotFound";
    case kCauseInvalidArgument:
      return "InvalidArgument";
    case kCauseResourceExhausted:
      return "ResourceExhausted";
    case kCausePermissionDenied:
      return "PermissionDenied";
    case kCauseBusy:
      return "Busy";
    case kCauseTimeout:
      return "Timeout";
    case kCauseCancelled:
      return "Cancelled";
    case kCauseAborted:
      return "Aborted";
    case kCauseAlreadyExists:
      return "AlreadyExists";
    case kCauseInvalidOperation:
      return "InvalidOperation";
    case kCauseOutOfRange:
      return "OutOfRange";
    case kCauseDataLoss:
      return "DataLoss";
    case kCauseHardwareError:
      return "HardwareError";
    case kCauseNotSupported:
      return "NotSupported";
    default:
      return "Unknown";
  }
}

#endif  // SENSCORD_STATUS_MESSAGE_ENABLED

/**
 * @brief Returns the empty string.
 */
const std::string& Status::EmptyString() {
  static std::string empty;
  return empty;
}

}  // namespace senscord
