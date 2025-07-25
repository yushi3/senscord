/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_SERIALIZE_DEFINE_H_
#define SENSCORD_SERIALIZE_DEFINE_H_

#include "senscord/config.h"

#ifdef SENSCORD_SERIALIZE

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif  // __GNUC__
#include <msgpack.hpp>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif  // __GNUC__

// Serialization format: default (msgpack-map)
#define SENSCORD_SERIALIZE_DEFINE(...) \
  MSGPACK_DEFINE_MAP(__VA_ARGS__)

// Serialization format: msgpack-array
#define SENSCORD_SERIALIZE_DEFINE_ARRAY(...) \
  MSGPACK_DEFINE_ARRAY(__VA_ARGS__)

// You need to use SENSCORD_SERIALIZE_ADD_ENUM in the global namespace.
#define SENSCORD_SERIALIZE_ADD_ENUM(Enum) \
  MSGPACK_ADD_ENUM(Enum)

#else

#define SENSCORD_SERIALIZE_DEFINE(...)
#define SENSCORD_SERIALIZE_DEFINE_ARRAY(...)
#define SENSCORD_SERIALIZE_ADD_ENUM(Enum)

#endif  // SENSCORD_SERIALIZE
#endif  // SENSCORD_SERIALIZE_DEFINE_H_
