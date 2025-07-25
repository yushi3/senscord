/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <inttypes.h>

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "senscord/senscord.h"
#include "senscord/logger.h"
#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"

#include "audio_pcm_playback.h"
#include "audio_pcm_playback_alsa.h"

#define LOGD(...) SENSCORD_LOG_DEBUG(__VA_ARGS__)
#define LOGI(...) SENSCORD_LOG_INFO(__VA_ARGS__)

namespace {

namespace osal = senscord::osal;

const char kDefaultCaptureDevice[] = "default";
const char kDefaultStreamKey[] = "audio_stream";

bool StrToUint64(const std::string& source, uint64_t* result) {
  if (result == NULL || source.empty()) {
    return false;
  }
  char* endptr = NULL;
  uint64_t num = 0;
  if (osal::OSStrtoull(
      source.c_str(), &endptr, osal::kOSRadixAuto, &num) != 0) {
    return false;
  }
  if (endptr == NULL || *endptr != '\0') {
    return false;
  }
  *result = num;
  return true;
}

senscord::AudioPcmProperty SetPcmProperty(
    senscord::Stream* stream,
    const senscord::AudioPcmProperty& property) {
  senscord::Status status = stream->SetProperty(
      senscord::kAudioPcmPropertyKey, &property);
  osal::OSPrintf("SetProperty(): status=%s\n", status.ToString().c_str());
  if (!status.ok()) {
    return property;
  }
  senscord::AudioPcmProperty result = property;
  stream->GetProperty(
      senscord::kAudioPcmPropertyKey, &result);
  return result;
}

template<typename T>
void ToHex(std::ostringstream* buf, T value) {
  (*buf) << std::setw(sizeof(T) * 2) << std::setfill('0')
      << std::uppercase << std::hex << static_cast<uint64_t>(value);
}

template<typename Iterator>
std::string ToHex(Iterator first, Iterator last, uint32_t max) {
  std::ostringstream buf;
  uint32_t count = 0;
  if (max == 0) { max = 0xFFFFFFFF; }
  Iterator itr = first;
  for (; (itr != last) && (count < max); ++itr, ++count) {
    if (itr != first) { buf << ' '; }
    ToHex(&buf, *itr);
  }
  if (itr != last) { buf << " ..."; }
  return buf.str();
}

/**
 * @brief Frame receive processing.
 */
void FrameCallback(senscord::Stream* stream, void* private_data) {
  AudioPcmPlayback* playback =
      reinterpret_cast<AudioPcmPlayback*>(private_data);

  senscord::Status status;
  senscord::Frame* frame = NULL;
  status = stream->GetFrame(&frame, 100);
  if (status.ok()) {
    playback->Write(frame);
    uint64_t seq_num = 0;
    frame->GetSequenceNumber(&seq_num);
    senscord::ChannelList channels;
    frame->GetChannelList(&channels);
    LOGI("[seq_num=%" PRIu64 "] channels=%" PRIuS,
         seq_num, channels.size());

    for (senscord::ChannelList::const_iterator itr = channels.begin(),
        end = channels.end(); itr != end; ++itr) {
      senscord::Channel* channel = itr->second;

      senscord::Channel::RawData rawdata = {};
      channel->GetRawData(&rawdata);
      uint8_t* ptr = reinterpret_cast<uint8_t*>(rawdata.address);
      LOGI("  [ch%" PRIu32 "] size=%" PRIuS ", time=%" PRIu64 ".%06u, data=%s",
           itr->first, rawdata.size, rawdata.timestamp / 1000000000,
           (rawdata.timestamp % 1000000000) / 1000,
           ToHex(ptr, ptr + rawdata.size, 16).c_str());
    }
    stream->ReleaseFrame(frame);
  }
}

void MainLoop(senscord::Stream* stream, AudioPcmPlayback* playback) {
  senscord::Status status;
  bool running = false;

  // GetProperty(AudioPcmProperty)
  senscord::AudioPcmProperty pcm_params = {};
  status = stream->GetProperty(
      senscord::kAudioPcmPropertyKey, &pcm_params);
  if (!status.ok()) {
    osal::OSPrintf("GetProperty(AudioPcmProperty): status=%s\n",
                   status.ToString().c_str());
    return;
  }

  while (true) {
    osal::OSPrintf("## Input command\n");
    osal::OSPrintf("##  s: Toggle Start/Stop        (%s)\n",
                   running ? "start" : "stop");
    osal::OSPrintf("##  i: Toggle Interleaved mode  (%s)\n",
                   pcm_params.interleaved ? "interleaved" : "non-interleaved");
    osal::OSPrintf("##  c: Channels                 (%" PRIu8 ")\n",
                   pcm_params.channels);
    osal::OSPrintf("##  f: Sample format            (%s)\n",
                   GetFormatName(pcm_params.format));
    osal::OSPrintf("##  r: Sample rate              (%" PRIu32 " Hz)\n",
                   pcm_params.samples_per_second);
    osal::OSPrintf("##  n: Samples per frame        (%" PRIu32 ")\n",
                   pcm_params.samples_per_frame);
    osal::OSPrintf("##  q: Quit\n");
    osal::OSPrintf("> ");
    std::string input;
    getline(std::cin, input);

    if (input == "q") {
      break;
    }

    if (input == "s" || (input.empty() && running)) {
      // Toggle Start/Stop
      if (!running) {
        status = playback->SetParams(pcm_params);
        osal::OSPrintf("Playback.SetParams(): status=%s\n",
                       status.ToString().c_str());
        status = playback->Start();
        osal::OSPrintf("Playback.Start(): status=%s\n",
                       status.ToString().c_str());
        status = stream->Start();
        osal::OSPrintf("Stream.Start(): status=%s\n",
                       status.ToString().c_str());
        if (status.ok()) {
          running = true;
        }
      } else {
        status = stream->Stop();
        osal::OSPrintf("Stream.Stop(): status=%s\n", status.ToString().c_str());
        if (status.ok()) {
          running = false;
        }
        status = playback->Stop();
        osal::OSPrintf("Playback.Stop(): status=%s\n",
                       status.ToString().c_str());
      }
      continue;
    }

    if (input == "i") {
      // Toggle Interleaved/NonInterleaved
      senscord::AudioPcmProperty params = pcm_params;
      params.interleaved = !params.interleaved;
      pcm_params = SetPcmProperty(stream, params);
      continue;
    }

    if (input == "c") {
      osal::OSPrintf("## Input number of channels (1, 2, 3, ...)\n");
      osal::OSPrintf("> ");
      getline(std::cin, input);
      uint64_t value = 0;
      if (StrToUint64(input, &value)) {
        if (value > UINT8_MAX) {
          value = UINT8_MAX;
        }
        senscord::AudioPcmProperty params = pcm_params;
        params.channels = value;
        pcm_params = SetPcmProperty(stream, params);
      }
      continue;
    }

    if (input == "f") {
      osal::OSPrintf("## Input format (%s)\n", GetFormatNameList().c_str());
      osal::OSPrintf("> ");
      getline(std::cin, input);
      senscord::AudioPcm::Format format = GetFormat(input);
      if (format != senscord::AudioPcm::kFormatUnknown) {
        senscord::AudioPcmProperty params = pcm_params;
        params.format = format;
        pcm_params = SetPcmProperty(stream, params);
      }
      continue;
    }

    if (input == "r") {
      osal::OSPrintf("## Input rate (8000, 16000, 32000, 44100, 48000, "
                     "96000, 19200, ...)\n");
      osal::OSPrintf("> ");
      getline(std::cin, input);
      uint64_t value = 0;
      if (StrToUint64(input, &value)) {
        if (value > UINT32_MAX) {
          value = UINT32_MAX;
        }
        senscord::AudioPcmProperty params = pcm_params;
        params.samples_per_second = value;
        pcm_params = SetPcmProperty(stream, params);
      }
      continue;
    }

    if (input == "n") {
      osal::OSPrintf("## Input samples per frame (0 < X < %" PRIu32 ")\n",
                     pcm_params.samples_per_second);
      osal::OSPrintf("> ");
      getline(std::cin, input);
      uint64_t value = 0;
      if (StrToUint64(input, &value)) {
        if (value == 0) {
          continue;
        }
        if (value > pcm_params.samples_per_second) {
          value = pcm_params.samples_per_second;
        }
        senscord::AudioPcmProperty params = pcm_params;
        params.samples_per_frame = value;
        pcm_params = SetPcmProperty(stream, params);
      }
      continue;
    }
  }
}

senscord::Status Run() {
  senscord::Status status;
  senscord::Core core;

  // Core.Init
  status = core.Init();
  LOGD("Init(): status=%s", status.ToString().c_str());
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  osal::OSPrintf("## Input stream key [audio_stream] > ");
  std::string input;
  getline(std::cin, input);
  if (input.empty()) {
    input = kDefaultStreamKey;
  }

  // Core.OpenStream
  senscord::Stream* stream = NULL;
  status = core.OpenStream(input, &stream);
  LOGD("OpenStream(): status=%s", status.ToString().c_str());
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  osal::OSPrintf("## Input playback device [default] > ");
  getline(std::cin, input);
  if (input.empty()) {
    input = kDefaultCaptureDevice;
  }

  AudioPcmPlayback* playback = new AudioPcmPlaybackAlsa();
  status = playback->Open(input);
  if (!status.ok()) {
    delete playback;
    return SENSCORD_STATUS_TRACE(status);
  }

  status = stream->RegisterFrameCallback(FrameCallback, playback);
  LOGD("RegisterFrameCallback(): status=%s", status.ToString().c_str());
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  MainLoop(stream, playback);

  playback->Close();
  delete playback;

  status = core.CloseStream(stream);
  LOGD("CloseStream(): status=%s", status.ToString().c_str());
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  status = core.Exit();
  LOGD("Exit(): status=%s", status.ToString().c_str());
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  return status;
}

}  // namespace

int32_t main() {
  senscord::Status status = Run();
  if (!status.ok()) {
    osal::OSPrintf("Error: %s\n", status.ToString().c_str());
    return -1;
  }
  return 0;
}
