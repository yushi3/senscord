/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/property_types_audio.h"

namespace {

using AudioPcm = senscord::AudioPcm;

enum FormatType {
  kFormatTypeSigned,
  kFormatTypeUnsigned,
  kFormatTypeFloat,
};

enum ByteOrder {
  kNoEndian,  // 8bit
  kLittleEndian,
  kBigEndian,
};

struct FormatInfo {
  AudioPcm::Format format;
  uint8_t byte_width;
  uint8_t bits_per_sample;
  FormatType format_type;
  ByteOrder byte_order;
};

const FormatInfo kFormatTable[] = {
    // 8bit
    { AudioPcm::kFormatS8, 1, 8, kFormatTypeSigned, kNoEndian },
    { AudioPcm::kFormatU8, 1, 8, kFormatTypeUnsigned, kNoEndian },
    // 16bit
    { AudioPcm::kFormatS16LE, 2, 16, kFormatTypeSigned, kLittleEndian },
    { AudioPcm::kFormatS16BE, 2, 16, kFormatTypeSigned, kBigEndian },
    { AudioPcm::kFormatU16LE, 2, 16, kFormatTypeUnsigned, kLittleEndian },
    { AudioPcm::kFormatU16BE, 2, 16, kFormatTypeUnsigned, kBigEndian },
    // 24bit
    { AudioPcm::kFormatS24LE3, 3, 24, kFormatTypeSigned, kLittleEndian },
    { AudioPcm::kFormatS24BE3, 3, 24, kFormatTypeSigned, kBigEndian },
    { AudioPcm::kFormatU24LE3, 3, 24, kFormatTypeUnsigned, kLittleEndian },
    { AudioPcm::kFormatU24BE3, 3, 24, kFormatTypeUnsigned, kBigEndian },
    // 24bit (using lower 3 bytes out of 4 bytes)
    { AudioPcm::kFormatS24LE, 4, 24, kFormatTypeSigned, kLittleEndian },
    { AudioPcm::kFormatS24BE, 4, 24, kFormatTypeSigned, kBigEndian },
    { AudioPcm::kFormatU24LE, 4, 24, kFormatTypeUnsigned, kLittleEndian },
    { AudioPcm::kFormatU24BE, 4, 24, kFormatTypeUnsigned, kBigEndian },
    // 32bit
    { AudioPcm::kFormatS32LE, 4, 32, kFormatTypeSigned, kLittleEndian },
    { AudioPcm::kFormatS32BE, 4, 32, kFormatTypeSigned, kBigEndian },
    { AudioPcm::kFormatU32LE, 4, 32, kFormatTypeUnsigned, kLittleEndian },
    { AudioPcm::kFormatU32BE, 4, 32, kFormatTypeUnsigned, kBigEndian },
    // 32bit float
    { AudioPcm::kFormatFloat32LE, 4, 32, kFormatTypeFloat, kLittleEndian },
    { AudioPcm::kFormatFloat32BE, 4, 32, kFormatTypeFloat, kBigEndian },
    // 64bit float
    { AudioPcm::kFormatFloat64LE, 8, 64, kFormatTypeFloat, kLittleEndian },
    { AudioPcm::kFormatFloat64BE, 8, 64, kFormatTypeFloat, kBigEndian },
};

const FormatInfo* FindFormat(AudioPcm::Format format) {
  const size_t count = sizeof(kFormatTable) / sizeof(kFormatTable[0]);
  for (size_t i = 0; i < count; ++i) {
    if (kFormatTable[i].format == format) {
      return &kFormatTable[i];
    }
  }
  return NULL;
}

}  // namespace

namespace senscord {

/**
 * @brief Returns the byte width.
 * @param[in] (format) PCM format.
 * @return Byte width.
 */
uint8_t AudioPcm::GetByteWidth(Format format) {
  const FormatInfo* info = FindFormat(format);
  return (info == NULL) ? 0 : info->byte_width;
}

/**
 * @brief Returns the number of bits per sample.
 * @param[in] (format) PCM format.
 * @return Number of bits per sample.
 */
uint8_t AudioPcm::GetBitsPerSample(Format format) {
  const FormatInfo* info = FindFormat(format);
  return (info == NULL) ? 0 : info->bits_per_sample;
}

/**
 * @brief Returns true if signed type.
 * @param[in] (format) PCM format.
 * @return True if signed type.
 */
bool AudioPcm::IsSigned(Format format) {
  const FormatInfo* info = FindFormat(format);
  return (info == NULL) ? false : (info->format_type == kFormatTypeSigned);
}

/**
 * @brief Returns true if unsigned type.
 * @param[in] (format) PCM format.
 * @return True if unsigned type.
 */
bool AudioPcm::IsUnsigned(Format format) {
  const FormatInfo* info = FindFormat(format);
  return (info == NULL) ? false : (info->format_type == kFormatTypeUnsigned);
}

/**
 * @brief Returns true if float type.
 * @param[in] (format) PCM format.
 * @return True if float type.
 */
bool AudioPcm::IsFloat(Format format) {
  const FormatInfo* info = FindFormat(format);
  return (info == NULL) ? false : (info->format_type == kFormatTypeFloat);
}

/**
 * @brief Returns true if little endian.
 * @param[in] (format) PCM format.
 * @return True if little endian.
 */
bool AudioPcm::IsLittleEndian(Format format) {
  const FormatInfo* info = FindFormat(format);
  return (info == NULL) ? false : (info->byte_order != kBigEndian);
}

/**
 * @brief Returns true if big endian.
 * @param[in] (format) PCM format.
 * @return True if big endian.
 */
bool AudioPcm::IsBigEndian(Format format) {
  const FormatInfo* info = FindFormat(format);
  return (info == NULL) ? false : (info->byte_order != kLittleEndian);
}

}  // namespace senscord
