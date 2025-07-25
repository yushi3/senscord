/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <math.h>
#include "senscord/osal.h"

namespace senscord {
namespace osal {

/**
 * @brief Calculates the absolute value of a floating-point number.
 * @param[in] num  Floating-point number.
 * @return The absolute value. There is no error return.
 */
double OSFabs(double num) {
  return fabs(num);
}

}  // namespace osal
}  // namespace senscord
