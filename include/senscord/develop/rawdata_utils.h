/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_RAWDATA_UTILS_H_
#define SENSCORD_DEVELOP_RAWDATA_UTILS_H_

#include <stdint.h>
#include <vector>

#include "senscord/status.h"

namespace senscord {

struct RawData;
struct TemporalContrastEventsTimeslice;

namespace internal {

/**
 * @brief Create TemporalContrastEventsTimeslice list.
 * @param[in] (address) Rawdata address
 * @param[in] (size) Rawdata size
 * @param[out] (timeslice_list) TemporalContrastEventsTimeslice list
 * @return Status object.
 */
Status CreateTemporalContrastEventsTimeslice(
    const void* address, size_t size,
    std::vector<TemporalContrastEventsTimeslice>* timeslice_list);

}  // namespace internal


/**
 * @brief TemporalContrastData reader class.
 */
class TemporalContrastDataReader {
 public:
  /**
   * @brief Constructor.
   * @param[in] (rawdata) Channel Rawdata.
   */
  explicit TemporalContrastDataReader(const RawData& rawdata);

  /**
   * @brief Get TemporalContrastEventsTimeslice data count.
   * @return TemporalContrastEventsTimeslice data count.
   */
  uint32_t GetCount() const;

  /**
   * @brief Get TemporalContrastEventsTimeslice data.
   * @param[in] (index) Target data index.
   * @param[out] (timeslice) TemporalContrastEventsTimeslice data.
   * @return Status object.
   */
  Status GetTimeslice(
      uint32_t index,
      TemporalContrastEventsTimeslice* timeslice) const;

  /**
   * @brief Get timeslice data list create status.
   * @return Status object.
   */
  Status GetStatus() const;

 private:
  std::vector<TemporalContrastEventsTimeslice> timeslice_list_;
  Status status_;
};

}   // namespace senscord
#endif  // SENSCORD_DEVELOP_RAWDATA_UTILS_H_
