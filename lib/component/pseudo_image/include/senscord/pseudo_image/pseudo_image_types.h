/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_PSEUDO_IMAGE_PSEUDO_IMAGE_TYPES_H_
#define SENSCORD_PSEUDO_IMAGE_PSEUDO_IMAGE_TYPES_H_

#include <stdint.h>
#include <string>
#include "senscord/serialize.h"

/**
 * Property key.
 */
const char kPseudoImagePropertyKey[] = "PseudoImageProperty";

/**
 * @brief Pseudo sample property.
 */
struct PseudoImageProperty {
  int x;
  int y;
  std::string z;

  SENSCORD_SERIALIZE_DEFINE(x, y, z)
};

#endif  // SENSCORD_PSEUDO_IMAGE_PSEUDO_IMAGE_TYPES_H_
