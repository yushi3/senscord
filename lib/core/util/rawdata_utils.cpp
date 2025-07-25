/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string>
#include <sstream>

#include "senscord/osal.h"
#include "senscord/senscord.h"

namespace senscord {
namespace internal {

// TemporalContrastData header size.
const uint32_t kTemporalContrastDataHeaderSize = 16;

// TemporalContrastEventsTimeslice header size.
const uint32_t kTemporalContrastEventsTimesliceHeaderSize = 24;

/**
 * @brief Create TemporalContrastEventsTimeslice list.
 * @param[in] (address) Rawdata address
 * @param[in] (size) Rawdata size
 * @param[out] (timeslice_list) TemporalContrastEventsTimeslice list
 * @return Status object.
 */
Status CreateTemporalContrastEventsTimeslice(
    const void* address, size_t size,
    std::vector<TemporalContrastEventsTimeslice>* timeslice_list) {
  Status status;
  if (address == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "address is null");
  }
  if (timeslice_list == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "timeslice_list is null");
  }

  const uint8_t* cursor = reinterpret_cast<const uint8_t*>(address);
  const uint8_t* cursor_end = cursor + size;
  const uint32_t count =
      reinterpret_cast<const TemporalContrastData*>(cursor)->count;

  cursor += kTemporalContrastDataHeaderSize;
  if (cursor > cursor_end) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseOutOfRange,
        "size(%" PRIuS ") is smaller than TemporalContrastDataHeader offset",
        size);
  }

  std::vector<TemporalContrastEventsTimeslice> elements;
  elements.reserve(count);

  for (uint32_t i = 0; i < count; ++i) {
    const TemporalContrastEventsTimeslice* bundle =
        reinterpret_cast<const TemporalContrastEventsTimeslice*>(cursor);
    cursor += kTemporalContrastEventsTimesliceHeaderSize;
    if (cursor > cursor_end) {
      status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
                  Status::kCauseOutOfRange,
                  "Timeslice index(%" PRIu32 ") buffer overrun at "
                  "EventsTimesliceHeader.", i);
      break;
    }
    TemporalContrastEventsTimeslice element = {};
    element.timestamp = bundle->timestamp;
    element.count = bundle->count;
    element.events = reinterpret_cast<TemporalContrastEvent*>(
        reinterpret_cast<uintptr_t>(cursor));
    cursor += sizeof(TemporalContrastEvent) * element.count;
    if (cursor > cursor_end) {
      status = SENSCORD_STATUS_FAIL(kStatusBlockCore,
                  Status::kCauseOutOfRange,
                  "Timeslice index(%" PRIu32 ") buffer overrun in event "
                  "array.", i);
      break;
    }
    elements.push_back(element);
  }

  if (!status.ok()) {
    return status;
  }

  timeslice_list->swap(elements);
  return Status::OK();
}

}  // namespace internal

TemporalContrastDataReader::TemporalContrastDataReader(
    const Channel::RawData& rawdata)
      : timeslice_list_(), status_() {
  if (rawdata.type != kRawDataTypeTemporalContrast) {
    status_ = SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "Invalid RawData type (%s)",
        rawdata.type.c_str());
    return;
  }

  status_ = internal::CreateTemporalContrastEventsTimeslice(
      rawdata.address, rawdata.size, &timeslice_list_);
}

/**
 * @brief Get TemporalContrastEventsTimeslice data count.
 * @return TemporalContrastEventsTimeslice data count.
 */
uint32_t TemporalContrastDataReader::GetCount() const {
  return static_cast<uint32_t>(timeslice_list_.size());
}

/**
 * @brief Get TemporalContrastEventsTimeslice data.
 * @param[in] (index) Target data index.
 * @param[out] (timeslice) TemporalContrastEventsTimeslice data.
 * @return Status object.
 */
Status TemporalContrastDataReader::GetTimeslice(
  uint32_t index,
  TemporalContrastEventsTimeslice* timeslice) const {
  if (!status_.ok()) {
    return status_;
  }

  if (timeslice == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "argument is null");
  }

  if (index >= timeslice_list_.size()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseOutOfRange, "index is out of range");
  }
  *timeslice = timeslice_list_[index];
  return Status::OK();
}

/**
 * @brief Get timeslice data list create status.
 * @return Status object.
 */
Status TemporalContrastDataReader::GetStatus() const {
  return status_;
}

}  // namespace senscord
