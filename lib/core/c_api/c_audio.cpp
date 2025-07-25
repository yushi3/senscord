/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/c_api/property_c_types_audio.h"

#include "senscord/property_types_audio.h"

/**
 * @brief Returns the byte width in PCM format.
 * @param[in] (format) PCM format.
 * @return Byte width.
 */
int32_t senscord_audio_pcm_get_byte_width(
    enum senscord_audio_pcm_format_t format) {
  return senscord::AudioPcm::GetByteWidth(
      static_cast<senscord::AudioPcm::Format>(format));
}

/**
 * @brief Returns the number of bits per sample in PCM format.
 * @param[in] (format) PCM format.
 * @return Number of bits per sample.
 */
int32_t senscord_audio_pcm_get_bits_per_sample(
    enum senscord_audio_pcm_format_t format) {
  return senscord::AudioPcm::GetBitsPerSample(
      static_cast<senscord::AudioPcm::Format>(format));
}

/**
 * @brief non-zero if PCM format is signed type.
 * @param[in] (format) PCM format.
 * @return Non-zero if signed type.
 */
int32_t senscord_audio_pcm_is_signed(
    enum senscord_audio_pcm_format_t format) {
  return senscord::AudioPcm::IsSigned(
      static_cast<senscord::AudioPcm::Format>(format));
}

/**
 * @brief Returns non-zero if PCM format is unsigned type.
 * @param[in] (format) PCM format.
 * @return Non-zero if unsigned type.
 */
int32_t senscord_audio_pcm_is_unsigned(
    enum senscord_audio_pcm_format_t format) {
  return senscord::AudioPcm::IsUnsigned(
      static_cast<senscord::AudioPcm::Format>(format));
}

/**
 * @brief Returns non-zero if PCM format is float type.
 * @param[in] (format) PCM format.
 * @return Non-zero if float type.
 */
int32_t senscord_audio_pcm_is_float(
    enum senscord_audio_pcm_format_t format) {
  return senscord::AudioPcm::IsFloat(
      static_cast<senscord::AudioPcm::Format>(format));
}

/**
 * @brief Returns non-zero if PCM format is little endian.
 * @param[in] (format) PCM format.
 * @return Non-zero if little endian.
 */
int32_t senscord_audio_pcm_is_little_endian(
    enum senscord_audio_pcm_format_t format) {
  return senscord::AudioPcm::IsLittleEndian(
      static_cast<senscord::AudioPcm::Format>(format));
}

/**
 * @brief Returns non-zero if PCM format is big endian.
 * @param[in] (format) PCM format.
 * @return Non-zero if big endian.
 */
int32_t senscord_audio_pcm_is_big_endian(
    enum senscord_audio_pcm_format_t format) {
  return senscord::AudioPcm::IsBigEndian(
      static_cast<senscord::AudioPcm::Format>(format));
}
