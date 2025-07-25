/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <string>

#include "senscord/senscord.h"
#include "senscord/status.h"
#include "senscord/osal.h"
#include "c_api/c_common.h"
#include "senscord/c_api/senscord_c_api.h"
#include "senscord/develop/rawdata_utils.h"

namespace c_api = senscord::c_api;

/**
 * @brief Create Temporal Contrast Data reader handle.
 * @param[in] rawdata  Channel RawData.
 * @param[out] reader  Reader handle.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_temporal_contrast_reader_create(
    const struct senscord_raw_data_t* rawdata,
    senscord_temporal_contrast_data_reader_t* reader) {
  SENSCORD_C_API_ARGUMENT_CHECK(rawdata == NULL);
  SENSCORD_C_API_ARGUMENT_CHECK(reader == 0);

  senscord::Channel::RawData raw_data = {};
  raw_data.address = rawdata->address;
  raw_data.size = rawdata->size;
  raw_data.type = rawdata->type;
  raw_data.timestamp = rawdata->timestamp;

  senscord::TemporalContrastDataReader* create_list =
      new senscord::TemporalContrastDataReader(raw_data);
  senscord::Status status = create_list->GetStatus();
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    delete create_list;
    return -1;
  }

  *reader = c_api::ToHandle(create_list);
  return 0;
}

/**
 * @brief Destroy Temporal Contrast Data reader handle.
 * @param[in] reader  Reader handle.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_temporal_contrast_reader_destroy(
    senscord_temporal_contrast_data_reader_t reader) {
  SENSCORD_C_API_ARGUMENT_CHECK(reader == 0);
  senscord::TemporalContrastDataReader* reader_ptr =
      c_api::ToPointer<senscord::TemporalContrastDataReader*>(reader);
  delete reader_ptr;
  return 0;
}

/**
 * @brief Get TemporalContrastEventsTimeslice data count.
 * @param[in] reader  Reader handle.
 * @param[out] count  TemporalContrastEventsTimeslice data count.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_temporal_contrast_reader_get_count(
    senscord_temporal_contrast_data_reader_t reader,
    uint32_t* count) {
  SENSCORD_C_API_ARGUMENT_CHECK(reader == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(count == NULL);
  senscord::TemporalContrastDataReader* reader_ptr =
      c_api::ToPointer<senscord::TemporalContrastDataReader*>(reader);
  *count = reader_ptr->GetCount();
  return 0;
}

/**
 * @brief Get TemporalContrastEventsTimeslice data.
 * @param[in] reader  Reader handle.
 * @param[in] index  Index of the data to retrieve.
 * @param[out] timeslice  TemporalContrastEventsTimeslice data.
 * @return 0 is success or minus is failed.
 */
int32_t senscord_temporal_contrast_reader_get_timeslice(
    senscord_temporal_contrast_data_reader_t reader,
    uint32_t index,
    struct senscord_temporal_contrast_events_timeslice_t* timeslice) {
  SENSCORD_C_API_ARGUMENT_CHECK(reader == 0);
  SENSCORD_C_API_ARGUMENT_CHECK(timeslice == NULL);
  senscord::TemporalContrastDataReader* reader_ptr =
      c_api::ToPointer<senscord::TemporalContrastDataReader*>(reader);

  senscord::TemporalContrastEventsTimeslice tmp = {};
  senscord::Status status = reader_ptr->GetTimeslice(index, &tmp);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }

  timeslice->timestamp = tmp.timestamp;
  timeslice->count = tmp.count;
  timeslice->events =
      reinterpret_cast<senscord_temporal_contrast_event_t*>(tmp.events);
  return 0;
}
