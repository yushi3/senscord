/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_SYNCHRONIZER_FPS_METER_H_
#define SENSCORD_SYNCHRONIZER_FPS_METER_H_

#include "senscord/osal.h"

class FpsMeter {
 private:
  double fps_now_;
  uint64_t last_tick_;
  uint64_t tick_sum_;
  uint64_t tick_count_;
  uint64_t total_tick_count_;

 public:
  FpsMeter()
      : fps_now_(0.0l),
        last_tick_(0ULL),
        tick_sum_(0ULL),
        tick_count_(0ULL),
        total_tick_count_(0ULL) {}
  double GetFrameRate() { return fps_now_; }

  void TickFrame() {
    uint64_t now_tick = GetTick();
    tick_sum_ += (now_tick - last_tick_);
    tick_count_++;

    if (tick_sum_ > 1000) {
      fps_now_ = 1000.0f / (static_cast<double>(tick_sum_) /
                               static_cast<double>(tick_count_));
      total_tick_count_ += tick_count_;
      tick_count_ = 0;
      tick_sum_ = 0;
    }

    last_tick_ = now_tick;
  }

  // return milisec
  uint64_t GetTick() {
    uint64_t nanosec = 0;
    senscord::osal::OSGetTime(&nanosec);
    return nanosec / 1000000ULL;
  }

  uint64_t GetFrameCount() { return total_tick_count_; }
};

#endif  // SENSCORD_SYNCHRONIZER_FPS_METER_H_
