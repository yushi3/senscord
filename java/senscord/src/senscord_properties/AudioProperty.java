/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Native;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Structure containing information about the audio raw data.
 *
 * @see "senscord::AudioProperty"
 */
public class AudioProperty extends Structure {
    public byte[] format;

    public static class ByReference extends AudioProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("format");
    }

    /**
     * Audio format.
     */
    public static interface Format {
        /**
         * Linear PCM.
         */
        public static final String SENSCORD_AUDIO_FORMAT_LINEAR_PCM = "audio_lpcm";
    }

    public AudioProperty() {
        super();
        format = new byte[64];
    }

    /**
     * Get audio format.
     *
     * @return Audio format
     */
    public String GetFormat() {
        return Native.toString(format);
    }

    /**
     * Set audio format.
     *
     * @param format Audio format. Maximum length is 64. {@link Format}
     */
    public void SetFormat(String format) {
        Arrays.fill(this.format, (byte)0);
        System.arraycopy(format.getBytes(), 0, this.format, 0, format.length());
    }
}
