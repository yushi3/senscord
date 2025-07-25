/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_NONCOPYABLE_H_
#define SENSCORD_NONCOPYABLE_H_

#include "senscord/config.h"

namespace senscord {
namespace util {

/**
 * @brief Non-copyable basic class.
 */
class Noncopyable {
 public:
  Noncopyable(const Noncopyable&);
  Noncopyable& operator=(const Noncopyable&);

 protected:
  Noncopyable() {}
  ~Noncopyable() {}
};

}   // namespace util
}   // namespace senscord
#endif  // SENSCORD_NONCOPYABLE_H_
