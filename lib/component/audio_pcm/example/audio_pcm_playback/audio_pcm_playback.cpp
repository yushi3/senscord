/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "audio_pcm_playback.h"

#include <string>
#include <sstream>

namespace {

const char kFormatUnknown[] = "unknown";

struct FormatInfo {
  senscord::AudioPcm::Format format;
  const char* name;
};

const FormatInfo kFormatTable[] = {
    // 8bit
    { senscord::AudioPcm::kFormatS8, "S8" },
    { senscord::AudioPcm::kFormatU8, "U8" },
    // 16bit
    { senscord::AudioPcm::kFormatS16LE, "S16LE" },
    { senscord::AudioPcm::kFormatS16BE, "S16BE" },
    { senscord::AudioPcm::kFormatU16LE, "U16LE" },
    { senscord::AudioPcm::kFormatU16BE, "U16BE" },
    // 24bit
    { senscord::AudioPcm::kFormatS24LE3, "S24LE3" },
    { senscord::AudioPcm::kFormatS24BE3, "S24BE3" },
    { senscord::AudioPcm::kFormatU24LE3, "U24LE3" },
    { senscord::AudioPcm::kFormatU24BE3, "U24BE3" },
    // 24bit (using lower 3 bytes out of 4 bytes)
    { senscord::AudioPcm::kFormatS24LE, "S24LE" },
    { senscord::AudioPcm::kFormatS24BE, "S24BE" },
    { senscord::AudioPcm::kFormatU24LE, "U24LE" },
    { senscord::AudioPcm::kFormatU24BE, "U24BE" },
    // 32bit
    { senscord::AudioPcm::kFormatS32LE, "S32LE" },
    { senscord::AudioPcm::kFormatS32BE, "S32BE" },
    { senscord::AudioPcm::kFormatU32LE, "U32LE" },
    { senscord::AudioPcm::kFormatU32BE, "U32BE" },
    // 32bit float
    { senscord::AudioPcm::kFormatFloat32LE, "Float32LE" },
    { senscord::AudioPcm::kFormatFloat32BE, "Float32BE" },
    // 64bit float
    { senscord::AudioPcm::kFormatFloat64LE, "Float64LE" },
    { senscord::AudioPcm::kFormatFloat64BE, "Float64BE" },
};

}  // namespace

const char* GetFormatName(senscord::AudioPcm::Format format) {
  int count = sizeof(kFormatTable) / sizeof(kFormatTable[0]);
  for (int i = 0; i < count; ++i) {
    if (kFormatTable[i].format == format) {
      return kFormatTable[i].name;
    }
  }
  return kFormatUnknown;
}

senscord::AudioPcm::Format GetFormat(const std::string& name) {
  int count = sizeof(kFormatTable) / sizeof(kFormatTable[0]);
  for (int i = 0; i < count; ++i) {
    if (name == kFormatTable[i].name) {
      return kFormatTable[i].format;
    }
  }
  return senscord::AudioPcm::kFormatUnknown;
}

std::string GetFormatNameList() {
  std::ostringstream buf;
  int count = sizeof(kFormatTable) / sizeof(kFormatTable[0]);
  for (int i = 0; i < count; ++i) {
    if (i != 0) {
      buf << ", ";
    }
    buf << kFormatTable[i].name;
  }
  return buf.str();
}
