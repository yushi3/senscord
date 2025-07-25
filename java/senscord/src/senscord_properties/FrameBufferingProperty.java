/*
 * SPDX-FileCopyrightText: 2019-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Structure;

/**
 * Frame buffering setting.
 * 
 * @see "senscord::FrameBufferingProperty"
 */
public class FrameBufferingProperty extends Structure {
    public int buffering;
    public int num;
    public int format;

    public static class ByReference extends FrameBufferingProperty
            implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("buffering", "num", "format");
    }
    
    /**
     * Frame buffering number use xml.
     */
    public static final int SENSCORD_BUFFER_NUM_USE_CONFIG = -2;

    /**
     * Frame buffering number default.
     */
    public static final int SENSCORD_BUFFER_NUM_DEFAULT = -1;

    /**
     * Frame buffering number of unlimited.
     */
    public static final int SENSCORD_BUFFER_NUM_UNLIMITED = 0;

    /**
     * Frame buffering.
     * 
     * @see "senscord::Buffering"
     */
    public static interface Buffering {
        /**
         * Use config.
         */
        public static final int SENSCORD_BUFFERING_USE_CONFIG = -2;

        /**
         * Default.
         */
        public static final int SENSCORD_BUFFERING_DEFAULT = -1;

        /**
         * Disable.
         */
        public static final int SENSCORD_BUFFERING_OFF = 0;

        /**
         * Enable.
         */
        public static final int SENSCORD_BUFFERING_ON = 1;
    }

    /**
     * Frame buffering format.
     * 
     * @see "senscord::BufferingFormat"
     */
    public static interface Format {
        /**
         * Use config.
         */
        public static final int SENSCORD_BUFFERING_FORMAT_USE_CONFIG = -2;

        /**
         * Default format.
         */
        public static final int SENSCORD_BUFFERING_FORMAT_DEFAULT = -1;

        /**
         * Discard the latest frame.
         */
        public static final int SENSCORD_BUFFERING_FORMAT_DISCARD = 0;

        /**
         * Overwrite the oldest frame.
         */
        public static final int SENSCORD_BUFFERING_FORMAT_OVERWRITE = 1;

        /**
         * Discard the latest frame.
         * @deprecated "queue" has been replaced by "discard".
         */
        public static final int SENSCORD_BUFFERING_FORMAT_QUEUE = 2;

        /**
         * Overwrite the oldest frame.
         * @deprecated "ring" has been replaced by "overwrite"
         */
        public static final int SENSCORD_BUFFERING_FORMAT_RING = 3;

    }

    /**
     * Get buffer setting.
     * 
     * @return buffer setting
     */
    public int GetBuffering() {
        return buffering;
    }

    /**
     * Set buffer setting.
     * 
     * @param buffering Set from {@link Buffering}.
     */
    public void SetBuffering(int buffering) {
        this.buffering = buffering;
    }

    /**
     * Get buffer setting of String type from buffering value.
     * 
     * @param buffering buffering value
     * @return buffer setting string
     */
    public String GetBufferingString(int buffering) {
        String ret = "";

        switch (buffering) {
            case Buffering.SENSCORD_BUFFERING_USE_CONFIG:
                ret = "USE_CONFIG";
                break;
            case Buffering.SENSCORD_BUFFERING_DEFAULT:
                ret = "DEFAULT";
                break;
            case Buffering.SENSCORD_BUFFERING_OFF:
                ret = "OFF";
                break;
            case Buffering.SENSCORD_BUFFERING_ON:
                ret = "ON";
                break;
            default:
                break;
        }
        return ret;
    }

    /**
     * Get buffer size.
     * 
     * @return buffer size
     */
    public int GetNum() {
        return num;
    }

    /**
     * Set buffer size.
     * 
     * @param num buffer size
     */
    public void SetNum(int num) {
        this.num = num;
    }

    /**
     * Get buffer format.
     * 
     * @return buffer format
     */
    public int GetFormat() {
        return format;
    }

    /**
     * Set buffer format.
     * 
     * @param format buffer format . Set from {@link Format}
     */
    public void SetFormat(int format) {
        this.format = format;
    }

    /**
     * Get buffer format of String type from buffer format value.
     * 
     * @param format buffer format
     * @return buffer format string
     */
    public String GetFormatString(int format) {
        String ret = "";

        switch (format) {
            case Format.SENSCORD_BUFFERING_FORMAT_USE_CONFIG:
                ret = "USE_CONFIG";
                break;
            case Format.SENSCORD_BUFFERING_FORMAT_DEFAULT:
                ret = "DEFAULT";
                break;
            case Format.SENSCORD_BUFFERING_FORMAT_DISCARD:
                ret = "DISCARD";
                break;
            case Format.SENSCORD_BUFFERING_FORMAT_OVERWRITE:
                ret = "OVERWRITE";
                break;
            case Format.SENSCORD_BUFFERING_FORMAT_QUEUE:
                ret = "QUEUE";
                break;
            case Format.SENSCORD_BUFFERING_FORMAT_RING:
                ret = "RING";
                break;
            default:
                break;
        }

        return ret;
    }
}
