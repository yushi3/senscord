/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_C_API_PROPERTY_C_TYPES_AUDIO_H_
#define SENSCORD_C_API_PROPERTY_C_TYPES_AUDIO_H_

#include <stdint.h>

#include "senscord/config.h"

/** Length of the audio format string. */
#define SENSCORD_AUDIO_FORMAT_LENGTH    (64)

/**
 * AudioProperty
 * @see senscord::kAudioPropertyKey
 */
#define SENSCORD_AUDIO_PROPERTY_KEY     "audio_property"

/**
 * @brief Structure containing information about the audio raw data.
 */
struct senscord_audio_property_t {
  char format[SENSCORD_AUDIO_FORMAT_LENGTH];
};

/**
 * @brief Audio format.
 */
#define SENSCORD_AUDIO_FORMAT_LINEAR_PCM    "audio_lpcm"  /* Linear PCM */

/**
 * @brief PCM format.
 */
enum senscord_audio_pcm_format_t {
  SENSCORD_AUDIO_PCM_UNKNOWN = -1,
  /** Signed 8bit */
  SENSCORD_AUDIO_PCM_S8 = 0,
  /** Unsigned 8bit */
  SENSCORD_AUDIO_PCM_U8,
  /** Signed 16bit Little Endian */
  SENSCORD_AUDIO_PCM_S16LE,
  /** Signed 16bit Big Endian */
  SENSCORD_AUDIO_PCM_S16BE,
  /** Unsigned 16bit Little Endian */
  SENSCORD_AUDIO_PCM_U16LE,
  /** Unsigned 16bit Big Endian */
  SENSCORD_AUDIO_PCM_U16BE,
  /** Signed 24bit Little Endian (3 bytes format) */
  SENSCORD_AUDIO_PCM_S24LE3,
  /** Signed 24bit Big Endian (3 bytes format) */
  SENSCORD_AUDIO_PCM_S24BE3,
  /** Unsigned 24bit Little Endian (3 bytes format) */
  SENSCORD_AUDIO_PCM_U24LE3,
  /** Unsigned 24bit Big Endian (3 bytes format) */
  SENSCORD_AUDIO_PCM_U24BE3,
  /** Signed 24bit Little Endian (4 bytes format) */
  SENSCORD_AUDIO_PCM_S24LE,
  /** Signed 24bit Big Endian (4 bytes format) */
  SENSCORD_AUDIO_PCM_S24BE,
  /** Unsigned 24bit Little Endian (4 bytes format) */
  SENSCORD_AUDIO_PCM_U24LE,
  /** Unsigned 24bit Big Endian (4 bytes format) */
  SENSCORD_AUDIO_PCM_U24BE,
  /** Signed 32bit Little Endian */
  SENSCORD_AUDIO_PCM_S32LE,
  /** Signed 32bit Big Endian */
  SENSCORD_AUDIO_PCM_S32BE,
  /** Unsigned 32bit Little Endian */
  SENSCORD_AUDIO_PCM_U32LE,
  /** Unsigned 32bit Big Endian */
  SENSCORD_AUDIO_PCM_U32BE,
  /** Float 32bit Little Endian */
  SENSCORD_AUDIO_PCM_FLOAT32LE,
  /** Float 32bit Big Endian */
  SENSCORD_AUDIO_PCM_FLOAT32BE,
  /** Float 64bit Little Endian */
  SENSCORD_AUDIO_PCM_FLOAT64LE,
  /** Float 64bit Big Endian */
  SENSCORD_AUDIO_PCM_FLOAT64BE,
};

/**
 * AudioPcmProperty
 * @see senscord::kAudioPcmPropertyKey
 */
#define SENSCORD_AUDIO_PCM_PROPERTY_KEY "audio_pcm_property"

/**
 * @brief Structure containing information about the PCM.
 */
struct senscord_audio_pcm_property_t {
  /** Number of channels */
  uint8_t channels;
  /** non-zero: interleaved, zero: non-interleaved */
  uint8_t interleaved;
  /** PCM format */
  enum senscord_audio_pcm_format_t format;
  /** Number of samples per second (e.g. 8000, 44100, 48000, 96000, ...) */
  uint32_t samples_per_second;
  /** Number of samples per frame */
  uint32_t samples_per_frame;
};

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 * @brief Returns the byte width in PCM format.
 * @param[in] (format) PCM format.
 * @return Byte width.
 */
int32_t senscord_audio_pcm_get_byte_width(
    enum senscord_audio_pcm_format_t format);

/**
 * @brief Returns the number of bits per sample in PCM format.
 * @param[in] (format) PCM format.
 * @return Number of bits per sample.
 */
int32_t senscord_audio_pcm_get_bits_per_sample(
    enum senscord_audio_pcm_format_t format);

/**
 * @brief Returns non-zero if PCM format is signed type.
 * @param[in] (format) PCM format.
 * @return Non-zero if signed type.
 */
int32_t senscord_audio_pcm_is_signed(
    enum senscord_audio_pcm_format_t format);

/**
 * @brief Returns non-zero if PCM format is unsigned type.
 * @param[in] (format) PCM format.
 * @return Non-zero if unsigned type.
 */
int32_t senscord_audio_pcm_is_unsigned(
    enum senscord_audio_pcm_format_t format);

/**
 * @brief Returns non-zero if PCM format is float type.
 * @param[in] (format) PCM format.
 * @return Non-zero if float type.
 */
int32_t senscord_audio_pcm_is_float(
    enum senscord_audio_pcm_format_t format);

/**
 * @brief Returns non-zero if PCM format is little endian.
 * @param[in] (format) PCM format.
 * @return Non-zero if little endian.
 */
int32_t senscord_audio_pcm_is_little_endian(
    enum senscord_audio_pcm_format_t format);

/**
 * @brief Returns non-zero if PCM format is big endian.
 * @param[in] (format) PCM format.
 * @return Non-zero if big endian.
 */
int32_t senscord_audio_pcm_is_big_endian(
    enum senscord_audio_pcm_format_t format);

#ifdef __cplusplus
}  /* extern "C" */
#endif  /* __cplusplus */

#endif  /* SENSCORD_C_API_PROPERTY_C_TYPES_AUDIO_H_ */
