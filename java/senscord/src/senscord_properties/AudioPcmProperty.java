/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Structure containing information about the PCM.
 *
 * @see "senscord::AudioPcmProperty"
 */
public class AudioPcmProperty extends Structure {
    public byte channels;
    public byte interleaved;
    public int format;
    public int samples_per_second;
    public int samples_per_frame;

    public static class ByReference extends AudioPcmProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("channels", "interleaved", "format", "samples_per_second", "samples_per_frame");
    }

    /**
     * PCM format.
     */
    public static interface Format {
        public static final int SENSCORD_AUDIO_PCM_UNKNOWN = -1;
        public static final int SENSCORD_AUDIO_PCM_S8 = 0;
        public static final int SENSCORD_AUDIO_PCM_U8 = 1;
        public static final int SENSCORD_AUDIO_PCM_S16LE = 2;
        public static final int SENSCORD_AUDIO_PCM_S16BE = 3;
        public static final int SENSCORD_AUDIO_PCM_U16LE = 4;
        public static final int SENSCORD_AUDIO_PCM_U16BE = 5;
        public static final int SENSCORD_AUDIO_PCM_S24LE3 = 6;
        public static final int SENSCORD_AUDIO_PCM_S24BE3 = 7;
        public static final int SENSCORD_AUDIO_PCM_U24LE3 = 8;
        public static final int SENSCORD_AUDIO_PCM_U24BE3 = 9;
        public static final int SENSCORD_AUDIO_PCM_S24LE = 10;
        public static final int SENSCORD_AUDIO_PCM_S24BE = 11;
        public static final int SENSCORD_AUDIO_PCM_U24LE = 12;
        public static final int SENSCORD_AUDIO_PCM_U24BE = 13;
        public static final int SENSCORD_AUDIO_PCM_S32LE = 14;
        public static final int SENSCORD_AUDIO_PCM_S32BE = 15;
        public static final int SENSCORD_AUDIO_PCM_U32LE = 16;
        public static final int SENSCORD_AUDIO_PCM_U32BE = 17;
        public static final int SENSCORD_AUDIO_PCM_FLOAT32LE = 18;
        public static final int SENSCORD_AUDIO_PCM_FLOAT32BE = 19;
        public static final int SENSCORD_AUDIO_PCM_FLOAT64LE = 20;
        public static final int SENSCORD_AUDIO_PCM_FLOAT64BE = 21;
    }

    public AudioPcmProperty() {
        super();
    }

    /**
     * Get the number of channels.
     *
     * @return number of channels.
     */
    public byte GetChannels() {
        return channels;
    }

    /**
     * Get the interleaved mode.
     *
     * @return zero: non-interleaved, non-zero: interleaved.
     */
    public byte GetInterleaved() {
        return interleaved;
    }

    /**
     * Get the PCM format.
     *
     * @return PCM format.
     */
    public int GetFormat() {
        return format;
    }

    /**
     * Get the number of samples per second.
     *
     * @return number of samples per second.
     */
    public int GetSamplesPerSecond() {
        return samples_per_second;
    }

    /**
     * Get the number of samples per frame.
     *
     * @return number of samples per frame.
     */
    public int GetSamplesPerFrame() {
        return samples_per_frame;
    }

    /**
     * Set the number of channels.
     *
     * @param channels number of channels.
     */
    public void SetChannels(byte channels) {
        this.channels = channels;
    }

    /**
     * Set the interleaved mode.
     *
     * @param interleaved zero: non-interleaved, non-zero: interleaved.
     */
    public void SetInterleaved(byte interleaved) {
        this.interleaved = interleaved;
    }

    /**
     * Set the PCM format.
     *
     * @param format PCM format. {@link Format}
     */
    public void SetFormat(int format) {
        this.format = format;
    }

    /**
     * Set the number of samples per second.
     *
     * @param samples_per_second number of samples per second.
     */
    public void SetSamplesPerSecond(int samples_per_second) {
        this.samples_per_second = samples_per_second;
    }

    /**
     * Set the number of samples per frame.
     *
     * @param samples_per_frame number of samples per frame.
     */
    public void SetSamplesPerFrame(int samples_per_frame) {
        this.samples_per_frame = samples_per_frame;
    }
}
