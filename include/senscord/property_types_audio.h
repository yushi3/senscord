/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_PROPERTY_TYPES_AUDIO_H_
#define SENSCORD_PROPERTY_TYPES_AUDIO_H_

#include <stdint.h>

#include <string>

#include "senscord/config.h"
#include "senscord/serialize_define.h"

namespace senscord {

/**
 * @brief PCM audio.
 */
class AudioPcm {
 public:
  /**
   * @brief PCM format.
   */
  enum Format {
    kFormatUnknown = -1,
    // 8bit
    kFormatS8 = 0,  /**< Signed 8bit */
    kFormatU8,      /**< Unsigned 8bit */
    // 16bit
    kFormatS16LE,   /**< Signed 16bit Little Endian */
    kFormatS16BE,   /**< Signed 16bit Big Endian */
    kFormatU16LE,   /**< Unsigned 16bit Little Endian */
    kFormatU16BE,   /**< Unsigned 16bit Big Endian */
    // 24bit
    kFormatS24LE3,  /**< Signed 24bit Little Endian (3 bytes format) */
    kFormatS24BE3,  /**< Signed 24bit Big Endian (3 bytes format) */
    kFormatU24LE3,  /**< Unsigned 24bit Little Endian (3 bytes format) */
    kFormatU24BE3,  /**< Unsigned 24bit Big Endian (3 bytes format) */
    // 24bit (using lower 3 bytes out of 4 bytes)
    kFormatS24LE,   /**< Signed 24bit Little Endian (4 bytes format) */
    kFormatS24BE,   /**< Signed 24bit Big Endian (4 bytes format) */
    kFormatU24LE,   /**< Unsigned 24bit Little Endian (4 bytes format) */
    kFormatU24BE,   /**< Unsigned 24bit Big Endian (4 bytes format) */
    // 32bit
    kFormatS32LE,   /**< Signed 32bit Little Endian */
    kFormatS32BE,   /**< Signed 32bit Big Endian */
    kFormatU32LE,   /**< Unsigned 32bit Little Endian */
    kFormatU32BE,   /**< Unsigned 32bit Big Endian */
    // 32bit float
    kFormatFloat32LE,  /**< Float 32bit Little Endian */
    kFormatFloat32BE,  /**< Float 32bit Big Endian */
    // 64bit float
    kFormatFloat64LE,  /**< Float 64bit Little Endian */
    kFormatFloat64BE,  /**< Float 64bit Big Endian */
  };

 public:
  /**
   * @brief Returns the byte width.
   * @param[in] (format) PCM format.
   * @return Byte width.
   */
  static uint8_t GetByteWidth(Format format);

  /**
   * @brief Returns the number of bits per sample.
   * @param[in] (format) PCM format.
   * @return Number of bits per sample.
   */
  static uint8_t GetBitsPerSample(Format format);

  /**
   * @brief Returns true if signed type.
   * @param[in] (format) PCM format.
   * @return True if signed type.
   */
  static bool IsSigned(Format format);

  /**
   * @brief Returns true if unsigned type.
   * @param[in] (format) PCM format.
   * @return True if unsigned type.
   */
  static bool IsUnsigned(Format format);

  /**
   * @brief Returns true if float type.
   * @param[in] (format) PCM format.
   * @return True if float type.
   */
  static bool IsFloat(Format format);

  /**
   * @brief Returns true if little endian.
   * @param[in] (format) PCM format.
   * @return True if little endian.
   */
  static bool IsLittleEndian(Format format);

  /**
   * @brief Returns true if big endian.
   * @param[in] (format) PCM format.
   * @return True if big endian.
   */
  static bool IsBigEndian(Format format);

 private:
  AudioPcm();
  ~AudioPcm();
};

}  // namespace senscord

SENSCORD_SERIALIZE_ADD_ENUM(senscord::AudioPcm::Format)

namespace senscord {

/**
 * AudioProperty
 */
const char kAudioPropertyKey[] = "audio_property";

/**
 * @brief Structure containing information about the audio raw data.
 */
struct AudioProperty {
  std::string format;  /**< Audio format */

  SENSCORD_SERIALIZE_DEFINE(format)
};

/**
 * @brief Audio format
 */
const char kAudioFormatLinearPcm[] = "audio_lpcm";  // Linear PCM

/**
 * AudioPcmProperty
 */
const char kAudioPcmPropertyKey[] = "audio_pcm_property";

/**
 * @brief Structure containing information about the PCM.
 */
struct AudioPcmProperty {
  /** Number of channels */
  uint8_t channels;
  /** true: interleaved, false: non-interleaved */
  bool interleaved;
  /** PCM format */
  AudioPcm::Format format;
  /** Number of samples per second (e.g. 8000, 44100, 48000, 96000, ...) */
  uint32_t samples_per_second;
  /** Number of samples per frame */
  uint32_t samples_per_frame;

  SENSCORD_SERIALIZE_DEFINE(
      channels, interleaved, format, samples_per_second, samples_per_frame)
};

}  // namespace senscord

#endif  // SENSCORD_PROPERTY_TYPES_AUDIO_H_
